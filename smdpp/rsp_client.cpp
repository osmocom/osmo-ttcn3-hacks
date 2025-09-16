/* RSP Cryptographic Operations Implementation
 *
 * Author: Eric Wild <ewild@sysmocom.de> / sysmocom - s.f.m.c. GmbH
 *
 * Released under the terms of GNU General Public License, Version 2 or
 * (at your option) any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/types.h>

#include <openssl/cmac.h>
#include <openssl/ec.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>

#include <openssl/safestack.h>
#include <openssl/stack.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>

#include <curl/curl.h>
}

#include "helpers.h"
#include "logger.h"
namespace RspCrypto {

class CertificateUtil {
    private:
	static std::unique_ptr<BIO, BIODeleter> readFileToBIO(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file) {
			throw std::runtime_error("Failed to open file: " + filePath);
		}

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		std::unique_ptr<BIO, BIODeleter> bio(BIO_new_mem_buf(content.c_str(), -1));
		if (!bio) {
			throw OpenSSLError("Failed to create BIO from file content");
		}

		return bio;
	}

	static std::vector<uint8_t> extractPublicKeyData(EVP_PKEY* key) {
		size_t pub_len = 0;
		if (EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &pub_len) != 1) {
			throw OpenSSLError("Failed to get public key size");
		}

		std::vector<uint8_t> publicKeyData(pub_len);
		if (EVP_PKEY_get_octet_string_param(key, OSSL_PKEY_PARAM_PUB_KEY, publicKeyData.data(), pub_len, &pub_len) != 1) {
			throw OpenSSLError("Failed to extract public key");
		}

		return publicKeyData;
	}

	static std::vector<uint8_t> getCertificateExtension(X509* cert, int nid) {
		int loc = X509_get_ext_by_NID(cert, nid, -1);
		if (loc < 0) {
			return std::vector<uint8_t>();
		}

		X509_EXTENSION* ext = X509_get_ext(cert, loc);
		if (!ext) {
			return std::vector<uint8_t>();
		}

		ASN1_OCTET_STRING* ext_data = X509_EXTENSION_get_data(ext);
		if (!ext_data) {
			return std::vector<uint8_t>();
		}

		return std::vector<uint8_t>(ASN1_STRING_get0_data(ext_data), ASN1_STRING_get0_data(ext_data) + ASN1_STRING_length(ext_data));
	}

    public:
	static std::string getEID(X509* cert) {
		X509_NAME* subject = X509_get_subject_name(cert);
		if (!subject) {
			return "";
		}

		// EID is stored in serialNumber field
		int index = X509_NAME_get_index_by_NID(subject, NID_serialNumber, -1);
		if (index < 0) {
			return "";
		}

		X509_NAME_ENTRY* entry = X509_NAME_get_entry(subject, index);
		if (!entry) {
			return "";
		}

		ASN1_STRING* asn1_str = X509_NAME_ENTRY_get_data(entry);
		if (!asn1_str) {
			return "";
		}

		const unsigned char* str_data = ASN1_STRING_get0_data(asn1_str);
		int str_len = ASN1_STRING_length(asn1_str);

		return std::string(reinterpret_cast<const char*>(str_data), str_len);
	}

	static std::string getSubjectName(X509* cert) {
		char subjectName[256];
		X509_NAME_oneline(X509_get_subject_name(cert), subjectName, sizeof(subjectName));
		return std::string(subjectName);
	}

	static std::vector<uint8_t> getSubjectKeyIdentifier(X509* cert) {
		auto ext_data = getCertificateExtension(cert, NID_subject_key_identifier);
		if (ext_data.empty()) {
			return std::vector<uint8_t>();
		}

		const unsigned char* p = ext_data.data();
		long xlen = ext_data.size();
		int tag, xclass;

		ASN1_get_object(&p, &xlen, &tag, &xclass, ext_data.size());

		return std::vector<uint8_t>(p, p + xlen);
	}

	static std::vector<uint8_t> getAuthorityKeyIdentifier(X509* cert) {
		auto ext_data = getCertificateExtension(cert, NID_authority_key_identifier);
		if (ext_data.empty()) {
			return std::vector<uint8_t>();
		}

		// AKI is more complex, need to extract keyid
		const unsigned char* p = ext_data.data();
		AUTHORITY_KEYID* akid = d2i_AUTHORITY_KEYID(NULL, &p, ext_data.size());

		std::vector<uint8_t> aki;
		if (akid && akid->keyid) {
			aki.assign(akid->keyid->data, akid->keyid->data + akid->keyid->length);
			AUTHORITY_KEYID_free(akid);
		}

		return aki;
	}

	static std::unique_ptr<X509, X509Deleter> loadCertFromDER(const std::vector<uint8_t>& derData) {
		const unsigned char* p = derData.data();
		X509* cert = d2i_X509(nullptr, &p, derData.size());

		if (!cert) {
			throw OpenSSLError("Failed to parse DER certificate");
		}

		return std::unique_ptr<X509, X509Deleter>(cert);
	}

	static std::vector<std::unique_ptr<X509, X509Deleter>> loadCertificateChain(const std::string& filename) {
		std::vector<std::unique_ptr<X509, X509Deleter>> chain;

		if (std::filesystem::path(filename).extension() == ".der") {
			FILE* file = fopen(filename.c_str(), "rb");
			if (!file) {
				throw std::runtime_error("No certificates found in file: " + filename);
			}

			X509* cert = d2i_X509_fp(file, nullptr);
			chain.push_back(std::unique_ptr<X509, X509Deleter>(cert));
			fclose(file);

		} else {
			BIO* bio = BIO_new_file(filename.c_str(), "r");
			if (!bio) {
				throw OpenSSLError("Failed to open certificate file: " + filename);
			}
			X509* cert = nullptr;
			while ((cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr)) != nullptr) {
				chain.push_back(std::unique_ptr<X509, X509Deleter>(cert));
			}

			BIO_free(bio);
		}

		if (chain.empty()) {
			throw std::runtime_error("No certificates found in file: " + filename);
		}

		return chain;
	}

	static std::vector<uint8_t> certToDER(X509* cert) {
		BIO* bio = BIO_new(BIO_s_mem());
		if (!bio) {
			throw OpenSSLError("Failed to create BIO for certificate conversion");
		}

		i2d_X509_bio(bio, cert);
		BUF_MEM* bufferPtr;
		BIO_get_mem_ptr(bio, &bufferPtr);

		std::vector<uint8_t> der;
		der.assign(bufferPtr->data, bufferPtr->data + bufferPtr->length);
		BIO_free(bio);

		return der;
	}

	static bool verifyCertificateChainDynamic(X509* cert, const std::vector<X509*>& certPool, X509* rootCA = nullptr, bool verbose = false) {
		LOG_INFO("Verifying certificate chain...");
		LOG_INFO("Target certificate: " + getSubjectName(cert));

		// certificate store for trusted roots only
		std::unique_ptr<X509_STORE, X509_STORE_Deleter> store(X509_STORE_new());
		if (!store) {
			throw OpenSSLError("Failed to create X509_STORE");
		}

		// Only add the explicitly provided root CA to the trust store
		if (rootCA) {
			if (verbose) {
				LOG_INFO("Adding trusted root CA: " + getSubjectName(rootCA));
			}
			if (X509_STORE_add_cert(store.get(), rootCA) != 1) {
				throw OpenSSLError("Failed to add root CA to store");
			}
		} else {
			// If no root CA provided, find self-signed certificates in the pool
			LOG_WARNING("No explicit root CA provided - searching for self-signed "
				    "certificates");
			for (auto candidate : certPool) {
				if (X509_check_issued(candidate, candidate) == X509_V_OK) {
					if (verbose) {
						LOG_INFO("Adding self-signed certificate as trusted root: " + getSubjectName(candidate));
					}
					if (X509_STORE_add_cert(store.get(), candidate) != 1) {
						unsigned long err = ERR_peek_last_error();
						// Ignore duplicate certificate errors
						if (ERR_GET_REASON(err) != X509_R_CERT_ALREADY_IN_HASH_TABLE) {
							throw OpenSSLError("Failed to add root CA to store");
						}
						ERR_clear_error();
					}
				}
			}
		}

		// Build untrusted chain from cert pool (excluding the root if it's in
		// there) IMPORTANT: custom deleter that only frees the stack, not the
		// certificates
		auto stack_only_deleter = [](STACK_OF(X509) * stack) {
			if (stack) {
				sk_X509_free(stack); // This only frees the stack structure, not the contents
			}
		};
		std::unique_ptr<STACK_OF(X509), decltype(stack_only_deleter)> untrusted_chain(sk_X509_new_null(), stack_only_deleter);
		if (!untrusted_chain) {
			throw OpenSSLError("Failed to create untrusted chain");
		}

		// Add all certificates from the pool as untrusted intermediates
		for (auto poolCert : certPool) {
			// Skip if this is the target certificate
			if (X509_cmp(poolCert, cert) == 0) {
				continue;
			}
			// Skip if this is the root CA (already in trust store)
			if (rootCA && X509_cmp(poolCert, rootCA) == 0) {
				continue;
			}
			// Add to untrusted chain
			if (sk_X509_push(untrusted_chain.get(), poolCert) == 0) {
				throw OpenSSLError("Failed to add certificate to untrusted chain");
			}
		}

		if (verbose) {
			LOG_INFO("Untrusted chain contains " + std::to_string(sk_X509_num(untrusted_chain.get())) + " certificates");
		}

		std::unique_ptr<X509_STORE_CTX, X509_STORE_CTX_Deleter> ctx(X509_STORE_CTX_new());
		if (!ctx) {
			throw OpenSSLError("Failed to create X509_STORE_CTX");
		}

		if (X509_STORE_CTX_init(ctx.get(), store.get(), cert, untrusted_chain.get()) != 1) {
			throw OpenSSLError("Failed to initialize X509_STORE_CTX");
		}

		unsigned long flags = X509_V_FLAG_CHECK_SS_SIGNATURE;
		X509_STORE_CTX_set_flags(ctx.get(), flags);

		X509_STORE_CTX_set_verify_cb(ctx.get(), [](int ok, X509_STORE_CTX* ctx) -> int {
			if (!ok) {
				int error = X509_STORE_CTX_get_error(ctx);
				// SGP.22 EUM certificates use name constraints in a specific way
				if (error == X509_V_ERR_PERMITTED_VIOLATION || error == X509_V_ERR_EXCLUDED_VIOLATION || error == X509_V_ERR_SUBTREE_MINMAX) {
					// Get the certificate that triggered this
					X509* cert = X509_STORE_CTX_get_current_cert(ctx);
					if (cert) {
						char subject_buf[256];
						X509_NAME_oneline(X509_get_subject_name(cert), subject_buf, sizeof(subject_buf));
						LOG_INFO("Processing certificate with name constraint abuse: " + std::string(subject_buf));
					}

					return 1;
				}
			}
			return ok; // Use default behavior for other errors
		});

		int result = X509_verify_cert(ctx.get());

		if (result != 1) {
			int error = X509_STORE_CTX_get_error(ctx.get());
			int depth = X509_STORE_CTX_get_error_depth(ctx.get());
			X509* errorCert = X509_STORE_CTX_get_current_cert(ctx.get());

			LOG_ERROR("Certificate verification failed:");
			LOG_ERROR("  Error:   " + std::string(X509_verify_cert_error_string(error)));
			LOG_ERROR("  Depth:   " + std::to_string(depth));

			if (errorCert) {
				LOG_ERROR("  Cert:    " + getSubjectName(errorCert));
			}

			return false;
		}

		LOG_INFO("Certificate verification successful");

		if (verbose) {
			STACK_OF(X509)* verified_chain = X509_STORE_CTX_get1_chain(ctx.get());
			if (verified_chain) {
				LOG_INFO("Verified certificate chain:");
				for (int i = 0; i < sk_X509_num(verified_chain); i++) {
					X509* chainCert = sk_X509_value(verified_chain, i);
					LOG_INFO("  " + std::to_string(i + 1) + ". " + getSubjectName(chainCert));
				}
				sk_X509_pop_free(verified_chain, X509_free);
			}
		}

		return true;
	}

	static std::vector<std::filesystem::path> find_cert_files(const std::filesystem::path& root_path, const std::vector<std::string>& name_filters = {}) {
		std::vector<std::filesystem::path> result;

		if (!std::filesystem::exists(root_path)) {
			std::cerr << "Path does not exist: " << root_path << std::endl;
			return result;
		}

		auto file_matches = [&name_filters](const std::filesystem::path& path) {
			std::string ext = path.extension().string();
			if (!(ext == ".pem" || ext == ".der" || ext == ".crt")) {
				return false;
			}

			// If no filters specified, accept all matching extensions
			if (name_filters.empty()) {
				return true;
			}

			// Check if filename contains any of the filters
			std::string filename = path.filename().string();
			return std::any_of(name_filters.begin(), name_filters.end(),
					   [&filename](const std::string& filter) { return filename.find(filter) != std::string::npos; });
		};

		if (std::filesystem::is_regular_file(root_path)) {
			if (file_matches(root_path)) {
				result.push_back(root_path);
			}
		} else if (std::filesystem::is_directory(root_path)) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(root_path)) {
				if (entry.is_regular_file() && file_matches(entry.path())) {
					result.push_back(entry.path());
				}
			}
		}

		return result;
	}

	static std::vector<std::unique_ptr<X509, X509Deleter>> loadCertificatesFromDirectory(const std::string& directory,
											     const std::vector<std::string>& name_filters) {
		std::vector<std::unique_ptr<X509, X509Deleter>> certificates;

		auto files_in_dir = find_cert_files(directory, name_filters);

		for (const auto& entry : files_in_dir) {
			std::string ext = entry.extension().string();
			auto fpath = entry.string();

			try {
				if (ext == ".pem" || ext == ".crt") {
					auto fileCerts = loadCertificateChain(entry);
					for (auto& cert : fileCerts) {
						certificates.push_back(std::move(cert));
					}
				} else if (ext == ".der") {
					FILE* file = fopen(fpath.c_str(), "rb");
					if (!file) {
						LOG_WARNING("Failed to open certificate file: " + fpath);
						continue;
					}

					fseek(file, 0, SEEK_END);
					long file_size = ftell(file);
					rewind(file);

					std::vector<unsigned char> data(file_size);
					if (fread(data.data(), 1, file_size, file) != static_cast<size_t>(file_size)) {
						fclose(file);
						LOG_WARNING("Failed to read certificate file: " + fpath);
						continue;
					}

					fclose(file);

					const unsigned char* p = data.data();
					X509* cert = d2i_X509(nullptr, &p, file_size);

					if (cert) {
						certificates.push_back(std::unique_ptr<X509, X509Deleter>(cert));
						LOG_INFO("Loaded certificate: " + getSubjectName(cert) + " from " + fpath);
					}
				}
			} catch (const std::exception& e) {
				// already logged in called functions
			}
		}

		return certificates;
	}

	static void xx_loadCertificate(const std::string& certPath, const std::string& typeName, std::unique_ptr<X509, X509Deleter>& certStorage) {
		try {
			auto certs = loadCertificateChain(certPath);
			if (certs.empty()) {
				throw std::runtime_error("No " + typeName + " certificate found in file: " + certPath);
			}

			certStorage = std::move(certs[0]);
			LOG_DEBUG("Loaded " + typeName + " certificate: " + getSubjectName(certStorage.get()));

		} catch (const std::exception& e) {
			LOG_ERROR("Failed to load " + typeName + " certificate: " + std::string(e.what()));
			throw;
		}
	}

	static void xx_loadPrivateKey(const std::string& keyPath, const std::string& typeName, std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter>& keyStorage) {
		try {
			if (!std::filesystem::exists(keyPath)) {
				throw std::runtime_error(typeName + " private key file not found: " + keyPath);
			}

			auto keyBio = readFileToBIO(keyPath);

			keyStorage.reset(PEM_read_bio_PrivateKey(keyBio.get(), nullptr, nullptr, nullptr));
			if (!keyStorage) {
				throw OpenSSLError("Failed to read " + typeName + " private key");
			}

			LOG_DEBUG("Loaded " + typeName + " private key from: " + keyPath);

		} catch (const std::exception& e) {
			LOG_ERROR("Failed to load " + typeName + " private key: " + std::string(e.what()));
			throw;
		}
	}

	static bool xx_loadPublicKey(const std::string& pubKeyPath, const std::string& typeName, std::vector<uint8_t>& publicKeyStorage) {
		try {
			if (!std::filesystem::exists(pubKeyPath)) {
				LOG_INFO(typeName + " public key file not found: " + pubKeyPath + " (will generate from private key)");
				return false;
			}

			auto keyBio = readFileToBIO(pubKeyPath);

			std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pubKey(PEM_read_bio_PUBKEY(keyBio.get(), nullptr, nullptr, nullptr));
			if (!pubKey) {
				LOG_WARNING("Failed to read " + typeName + " public key from file (will generate from private key)");
				return false;
			}

			publicKeyStorage = extractPublicKeyData(pubKey.get());

			LOG_INFO("Loaded " + typeName + " public key from file (" + std::to_string(publicKeyStorage.size()) + " bytes)");
			return true;

		} catch (const std::exception& e) {
			LOG_WARNING("Failed to load " + typeName + " public key from file: " + std::string(e.what()) + " (will generate from private key)");
			return false;
		}
	}

	static void xx_generatePublicKeyFromPrivate(const std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter>& privateKey, const std::string& typeName,
						    std::vector<uint8_t>& publicKeyStorage) {
		if (!privateKey) {
			throw std::runtime_error("No " + typeName + " private key available for public key generation");
		}

		publicKeyStorage = extractPublicKeyData(privateKey.get());

		LOG_DEBUG("Generated " + typeName + " public key from private key (" + std::to_string(publicKeyStorage.size()) + " bytes)");
	}

	static std::vector<std::string> parse_permitted_eins_from_cert(X509* cert) {
		std::vector<std::string> permitted_iins;

		bool is_old_variant = false;
		int pos = X509_get_ext_by_NID(cert, NID_certificate_policies, -1);
		if (pos >= 0) {
			X509_EXTENSION* ext = X509_get_ext(cert, pos);
			CERTIFICATEPOLICIES* policies = (CERTIFICATEPOLICIES*)X509V3_EXT_d2i(ext);
			if (policies) {
				for (int i = 0; i < sk_POLICYINFO_num(policies); i++) {
					POLICYINFO* policy = sk_POLICYINFO_value(policies, i);
					char oid_str[128];
					OBJ_obj2txt(oid_str, sizeof(oid_str), policy->policyid, 1);
					if (std::string(oid_str) == "2.23.146.1.2.1.2") {
						is_old_variant = true;
						break;
					}
				}
				CERTIFICATEPOLICIES_free(policies);
			}
		}

		if (is_old_variant) {
			// Parse from nameConstraints
			pos = X509_get_ext_by_NID(cert, NID_name_constraints, -1);
			if (pos >= 0) {
				X509_EXTENSION* ext = X509_get_ext(cert, pos);
				NAME_CONSTRAINTS* nc = (NAME_CONSTRAINTS*)X509V3_EXT_d2i(ext);
				if (nc && nc->permittedSubtrees) {
					for (int i = 0; i < sk_GENERAL_SUBTREE_num(nc->permittedSubtrees); i++) {
						GENERAL_SUBTREE* subtree = sk_GENERAL_SUBTREE_value(nc->permittedSubtrees, i);
						if (subtree->base->type == GEN_DIRNAME) {
							X509_NAME* name = subtree->base->d.directoryName;
							int idx = X509_NAME_get_index_by_NID(name, NID_serialNumber, -1);
							if (idx >= 0) {
								X509_NAME_ENTRY* entry = X509_NAME_get_entry(name, idx);
								ASN1_STRING* asn1_str = X509_NAME_ENTRY_get_data(entry);
								std::string iin(reinterpret_cast<const char*>(ASN1_STRING_get0_data(asn1_str)),
										ASN1_STRING_length(asn1_str));
								permitted_iins.push_back(iin);
							}
						}
					}
				}
				if (nc)
					NAME_CONSTRAINTS_free(nc);
			}
		} else {
			// Parse from GSMA permittedEins extension
			ASN1_OBJECT* obj = OBJ_txt2obj("2.23.146.1.2.2.0", 1);
			pos = X509_get_ext_by_OBJ(cert, obj, -1);
			ASN1_OBJECT_free(obj);

			if (pos >= 0) {
				X509_EXTENSION* ext = X509_get_ext(cert, pos);
				ASN1_OCTET_STRING* ext_data = X509_EXTENSION_get_data(ext);
				const unsigned char* p = ext_data->data;
				long remaining = ext_data->length;

				// SEQUENCE OF PrintableString
				int tag, xclass;
				long xlen;

				// SEQUENCE tag
				ASN1_get_object(&p, &xlen, &tag, &xclass, remaining);
				if (tag == V_ASN1_SEQUENCE) {
					remaining = xlen;

					while (remaining > 0) {
						const unsigned char* elem_start = p;
						ASN1_get_object(&p, &xlen, &tag, &xclass, remaining);

						if (tag == V_ASN1_PRINTABLESTRING) {
							std::string iin(reinterpret_cast<const char*>(p), xlen);
							permitted_iins.push_back(iin);
						}

						p += xlen;
						remaining -= (p - elem_start);
					}
				}
			}
		}

		return permitted_iins;
	}

	static std::string getPermittedEINs(const std::vector<uint8_t>& eumCertData) {
		try {
			auto cert = loadCertFromDER(eumCertData);
			auto eins = parse_permitted_eins_from_cert(cert.get());

			// Join EINs with comma
			std::string result;
			for (size_t i = 0; i < eins.size(); i++) {
				if (i > 0)
					result += ",";
				result += eins[i];
			}

			return result;
		} catch (const std::exception& e) {
			LOG_ERROR("getPermittedEINs failed: " + std::string(e.what()));
			return "";
		}
	}

	static bool validateEIDRange(const std::string& eid, const std::vector<uint8_t>& eumCertData) {
		try {
			auto eumCert = loadCertFromDER(eumCertData);
			std::vector<std::string> permittedEins = parse_permitted_eins_from_cert(eumCert.get());

			if (permittedEins.empty()) {
				LOG_WARNING("No permitted EINs found in EUM certificate");
				return false;
			}

			// Check if EID starts with any permitted EIN
			std::string eidNormalized = eid;
			std::transform(eidNormalized.begin(), eidNormalized.end(), eidNormalized.begin(), ::toupper);

			for (const auto& ein : permittedEins) {
				if (eidNormalized.find(ein) == 0) {
					LOG_INFO("EID " + eidNormalized + " matches permitted EIN " + ein);
					return true;
				}
			}

			LOG_ERROR("EID " + eidNormalized + " is not in any permitted EIN list");
			return false;
		} catch (const std::exception& e) {
			LOG_ERROR("validateEIDRange failed: " + std::string(e.what()));
			return false;
		}
	}

	static bool hasRSPRole(const std::vector<uint8_t>& certData, const std::string& roleOid) {
		try {
			auto cert = loadCertFromDER(certData);

			int pos = X509_get_ext_by_NID(cert.get(), NID_certificate_policies, -1);
			if (pos < 0)
				return false;

			X509_EXTENSION* ext = X509_get_ext(cert.get(), pos);
			CERTIFICATEPOLICIES* policies = (CERTIFICATEPOLICIES*)X509V3_EXT_d2i(ext);

			if (!policies)
				return false;

			for (int i = 0; i < sk_POLICYINFO_num(policies); i++) {
				POLICYINFO* policy = sk_POLICYINFO_value(policies, i);
				char oid_str[128];
				OBJ_obj2txt(oid_str, sizeof(oid_str), policy->policyid, 1);

				if (std::string(oid_str) == roleOid) {
					CERTIFICATEPOLICIES_free(policies);
					return true;
				}
			}

			CERTIFICATEPOLICIES_free(policies);
			return false;
		} catch (const std::exception& e) {
			LOG_ERROR("hasRSPRole failed: " + std::string(e.what()));
			return false;
		}
	}

	static std::string getCurveOID(const std::vector<uint8_t>& certData) {
		try {
			auto cert = loadCertFromDER(certData);

			EVP_PKEY* pkey = X509_get0_pubkey(cert.get());
			if (!pkey) {
				LOG_ERROR("Failed to get public key from certificate");
				return "";
			}

			if (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC) {
				LOG_ERROR("Certificate does not contain EC key");
				return "";
			}

			char curve_name[256] = { 0 };
			size_t curve_name_len = sizeof(curve_name);

			if (EVP_PKEY_get_utf8_string_param(pkey, "group", curve_name, sizeof(curve_name), &curve_name_len) != 1) {
				LOG_ERROR("Failed to get curve name from EC key");
				return "";
			}

			std::string curve_str(curve_name);

			if (curve_str == "prime256v1") {
				return "prime256v1";
			} else if (curve_str == "secp384r1") {
				return "secp384r1";
			} else if (curve_str == "brainpoolP256r1") {
				return "brainpoolP256r1";
			} else if (curve_str == "brainpoolP384r1") {
				return "brainpoolP384r1";
			}

			return "unknown";
		} catch (const std::exception& e) {
			LOG_ERROR("getCurveOID failed: " + std::string(e.what()));
			return "";
		}
	}

	// ECDH compatible (same curve)
	static bool verifyECDHCompatible(const std::vector<uint8_t>& pubKey1, const std::vector<uint8_t>& pubKey2) {
		try {
			// Both should be uncompressed EC points starting with 0x04
			if (pubKey1.empty() || pubKey2.empty() || pubKey1[0] != 0x04 || pubKey2[0] != 0x04) {
				LOG_ERROR("Invalid EC public key format");
				return false;
			}

			// For P-256, size should be 65 bytes (1 + 32 + 32)
			if (pubKey1.size() == 65 && pubKey2.size() == 65) {
				LOG_INFO("Both keys are P-256 format, ECDH compatible");
				return true;
			}

			// For P-384, size should be 97 bytes (1 + 48 + 48)
			if (pubKey1.size() == 97 && pubKey2.size() == 97) {
				LOG_INFO("Both keys are P-384 format, ECDH compatible");
				return true;
			}

			LOG_ERROR("Key sizes don't match or unsupported curve");
			return false;
		} catch (const std::exception& e) {
			LOG_ERROR("verifyECDHCompatible failed: " + std::string(e.what()));
			return false;
		}
	}

	static std::vector<uint8_t> convertBSI_TR03111_to_DER(const std::vector<uint8_t>& bsi_signature) {
		if (bsi_signature.size() != 64) {
			std::cerr << "Invalid BSI TR-03111 signature size: " << bsi_signature.size() << std::endl;
			return {};
		}

		BIGNUM* r = BN_new();
		BIGNUM* s = BN_new();

		if (!r || !s) {
			if (r)
				BN_free(r);
			if (s)
				BN_free(s);
			return {};
		}

		BN_bin2bn(bsi_signature.data(), 32, r);
		BN_bin2bn(bsi_signature.data() + 32, 32, s);

		ECDSA_SIG* ecdsa_sig = ECDSA_SIG_new();
		if (!ecdsa_sig) {
			BN_free(r);
			BN_free(s);
			return {};
		}

		ECDSA_SIG_set0(ecdsa_sig, r, s);

		int der_len = i2d_ECDSA_SIG(ecdsa_sig, nullptr);
		if (der_len <= 0) {
			ECDSA_SIG_free(ecdsa_sig);
			return {};
		}

		std::vector<uint8_t> der_signature(der_len);
		unsigned char* der_ptr = der_signature.data();
		i2d_ECDSA_SIG(ecdsa_sig, &der_ptr);

		ECDSA_SIG_free(ecdsa_sig);

		return der_signature;
	}

	static std::vector<uint8_t> convertDER_to_BSI_TR03111(const std::vector<uint8_t>& der_signature) {
		if (der_signature.empty()) {
			std::cerr << "Empty DER signature" << std::endl;
			return {};
		}

		const unsigned char* der_ptr = der_signature.data();
		ECDSA_SIG* ecdsa_sig = d2i_ECDSA_SIG(nullptr, &der_ptr, der_signature.size());

		if (!ecdsa_sig) {
			std::cerr << "Failed to parse DER signature" << std::endl;
			return {};
		}

		const BIGNUM *r, *s;
		ECDSA_SIG_get0(ecdsa_sig, &r, &s);

		if (!r || !s) {
			ECDSA_SIG_free(ecdsa_sig);
			return {};
		}

		std::vector<uint8_t> bsi_signature(64, 0);

		int r_len = BN_num_bytes(r);
		int s_len = BN_num_bytes(s);

		if (r_len > 32 || s_len > 32) {
			std::cerr << "BIGNUM values too large for BSI TR-03111 format" << std::endl;
			ECDSA_SIG_free(ecdsa_sig);
			return {};
		}

		// Convert r to bytes (with leading zeros padding)
		int r_offset = 32 - r_len;
		BN_bn2bin(r, bsi_signature.data() + r_offset);

		// Convert s to bytes (with leading zeros padding)
		int s_offset = 64 - s_len;
		BN_bn2bin(s, bsi_signature.data() + s_offset);

		ECDSA_SIG_free(ecdsa_sig);

		return bsi_signature;
	}

	static bool verify_TR031111(const std::vector<uint8_t>& message, const std::vector<uint8_t>& bsi_signature, EVP_PKEY* publicKey) {
		std::vector<uint8_t> der_signature = convertBSI_TR03111_to_DER(bsi_signature);
		if (der_signature.empty()) {
			std::cerr << "Failed to convert signature to DER format" << std::endl;
			return false;
		}

		std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdctx(EVP_MD_CTX_new());
		if (!mdctx.get()) {
			std::cerr << "Failed to create MD context" << std::endl;
			return false;
		}

		if (EVP_DigestVerifyInit(mdctx.get(), nullptr, EVP_sha256(), nullptr, publicKey) != 1) {
			std::cerr << "Failed to initialize digest verify" << std::endl;
			return false;
		}

		if (EVP_DigestVerifyUpdate(mdctx.get(), message.data(), message.size()) != 1) {
			std::cerr << "Failed to update digest verify" << std::endl;
			return false;
		}

		int result = EVP_DigestVerifyFinal(mdctx.get(), der_signature.data(), der_signature.size());

		return result == 1;
	}
};

class HttpClient {
    public:
	HttpClient() {
		// should be called once per application
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}

	~HttpClient() {
		curl_global_cleanup();
	}

	struct ResponseData {
		std::string body;
		long statusCode;
		std::string headers;
	};

	struct PostConfig {
		bool useMutualTLS = false;
		std::string clientCertPath;
		std::string clientKeyPath;
		bool includeAdminProtocolHeader = false;
		bool verboseOutput = false;
		std::string contentType = "application/json";
	};

	ResponseData postJson(const std::string& url, unsigned int port, const std::string& jsonData, X509_STORE* store, std::vector<X509*>& certPool,
			      const PostConfig& config);

    private:
	static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
		size_t realSize = size * nmemb;
		auto* response = static_cast<std::string*>(userp);
		response->append(static_cast<char*>(contents), realSize);
		return realSize;
	}

	static size_t headerCallback(void* contents, size_t size, size_t nmemb, void* userp) {
		size_t realSize = size * nmemb;
		auto* headers = static_cast<std::string*>(userp);
		headers->append(static_cast<char*>(contents), realSize);
		return realSize;
	}

	struct SslCtxData {
		X509_STORE* store;
		std::vector<X509*>* certPool;
		bool verifyResult;
		std::string errorMessage;
	};

	static std::string get_cn_name(X509_NAME* const name) {
		int idx = -1;
		unsigned char* utf8 = NULL;
		X509_NAME_ENTRY* entry;
		ASN1_STRING* data;

		if (!name)
			return {};

		idx = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
		if (idx < 0)
			return {};

		entry = X509_NAME_get_entry(name, idx);
		if (!entry)
			return {};

		data = X509_NAME_ENTRY_get_data(entry);
		if (!data)
			return {};

		if (!ASN1_STRING_to_UTF8(&utf8, data) || !utf8)
			return {};

		std::string retstr((char*)utf8);
		OPENSSL_free(utf8);
		return retstr;
	}

	static CURLcode sslCtxFunction(CURL* curl, SSL_CTX* sslCtx, void* arg) {
		SslCtxData* ctxData = static_cast<SslCtxData*>(arg);
		SSL_CTX* ctx = (SSL_CTX*)sslCtx;
		X509_STORE* store = SSL_CTX_get_cert_store(ctx);

		for (auto cert : *ctxData->certPool) {
			LOG_DEBUG("ADDED Issuer: " + get_cn_name(X509_get_issuer_name(cert)) + " Subject: " + get_cn_name(X509_get_subject_name(cert)) +
				  " SKI: " + HexUtil::bytesToHex(CertificateUtil::getSubjectKeyIdentifier(cert)) +
				  " AKI: " + HexUtil::bytesToHex(CertificateUtil::getAuthorityKeyIdentifier(cert)));

			auto result = X509_STORE_add_cert(store, cert);
			if (result != 1) {
				printf("Warning: Could not add certificate\n");

				unsigned long err = ERR_get_error();
				if (ERR_GET_REASON(err) != X509_R_CERT_ALREADY_IN_HASH_TABLE) {
					ERR_print_errors_fp(stderr);
				}
			}
		}

		SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER, nullptr);

		return CURLE_OK;
	}
};

inline HttpClient::ResponseData HttpClient::postJson(const std::string& url, unsigned int port, const std::string& jsonData, X509_STORE* store,
						     std::vector<X509*>& certPool, const PostConfig& config) {
	ResponseData response;
	CURL* curl = curl_easy_init();

	if (!curl) {
		throw std::runtime_error("Failed to initialize CURL");
	}

	struct curl_slist* headers = nullptr;
	std::string contentTypeHeader = "Content-Type: " + config.contentType;
	if (config.useMutualTLS) {
		if (config.contentType == "application/json") {
			contentTypeHeader += ";charset=UTF-8";
		}
		headers = curl_slist_append(headers, contentTypeHeader.c_str());
		headers = curl_slist_append(headers, "Accept: application/json");
		if (config.includeAdminProtocolHeader) {
			headers = curl_slist_append(headers, "X-Admin-Protocol: gsma/rsp/v2.5.0");
		}
	} else {
		headers = curl_slist_append(headers, contentTypeHeader.c_str());
		// For ASN.1, accept the same content type
		std::string acceptHeader = "Accept: " + (config.contentType == "application/x-gsma-rsp-asn1" ? config.contentType : "application/json");
		headers = curl_slist_append(headers, acceptHeader.c_str());
		// Always add X-Admin-Protocol for ES9+ regardless of content type
		headers = curl_slist_append(headers, "X-Admin-Protocol: gsma/rsp/v2.5.0");
	}

	SslCtxData ctxData = { .store = store, .certPool = &certPool, .verifyResult = false, .errorMessage = "" };

	try {
		// Basic CURL setup
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_PORT, port);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonData.size()); // Important for binary data
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);

		// Enable SSL verification
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		// Set reasonable timeouts
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

		if (config.useMutualTLS) {
			if (!config.clientCertPath.empty()) {
				curl_easy_setopt(curl, CURLOPT_SSLCERT, config.clientCertPath.c_str());

				std::string certType = "PEM";
				if (config.clientCertPath.find(".der") != std::string::npos || config.clientCertPath.find(".DER") != std::string::npos) {
					certType = "DER";
				}
				curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, certType.c_str());
				LOG_DEBUG("Set client certificate: " + config.clientCertPath + " (type: " + certType + ")");
			}

			if (!config.clientKeyPath.empty()) {
				curl_easy_setopt(curl, CURLOPT_SSLKEY, config.clientKeyPath.c_str());
				curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
				LOG_DEBUG("Set client private key: " + config.clientKeyPath);
			}
		}

		// Use our custom SSL context function for server cert verification
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslCtxFunction);
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, &ctxData);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, config.verboseOutput ? 1L : 0L);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			throw std::runtime_error(std::string("CURL request failed: ") + curl_easy_strerror(res));
		}

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		return response;
	} catch (...) {
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		throw;
	}
}

// Hashed Confirmation Code = SHA256(SHA256(Confirmation Code) | TransactionID)
static std::vector<uint8_t> computeConfirmationCodeHash(const std::string& confirmationCode, const std::vector<uint8_t>& transactionId) {
	// Convert confirmation code from hex string to bytes (like EID handling)
	std::vector<uint8_t> ccBytes;
	for (size_t i = 0; i < confirmationCode.length(); i += 2) {
		if (i + 1 < confirmationCode.length()) {
			std::string byteString = confirmationCode.substr(i, 2);
			uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
			ccBytes.push_back(byte);
		}
	}

	unsigned char firstHash[SHA256_DIGEST_LENGTH];
	SHA256(ccBytes.data(), ccBytes.size(), firstHash);

	std::vector<uint8_t> concatenated;
	concatenated.insert(concatenated.end(), firstHash, firstHash + SHA256_DIGEST_LENGTH);
	concatenated.insert(concatenated.end(), transactionId.begin(), transactionId.end());

	unsigned char finalHash[SHA256_DIGEST_LENGTH];
	SHA256(concatenated.data(), concatenated.size(), finalHash);

	return std::vector<uint8_t>(finalHash, finalHash + SHA256_DIGEST_LENGTH);
}

class RSPClient {
    public:
	RSPClient(const std::string& serverUrl, const unsigned int serverPort, const std::vector<std::string>& certPath,
		  const std::vector<std::string>& name_filters = {})
		: m_serverUrl(serverUrl) {
		// OpenSSL 3.0+ initializes automatically

		for (auto cpath : certPath) {
			bool isDirectory = std::filesystem::is_directory(cpath);
			try {
				if (isDirectory) {
					LOG_DEBUG("Loading certificates from directory: " + cpath);

					auto certs = CertificateUtil::loadCertificatesFromDirectory(cpath, name_filters);
					LOG_DEBUG("Loaded " + std::to_string(certs.size()) + " certificates");

					// Separate root CAs and intermediates
					for (auto& cert : certs) {
						// Identify root certificates (self-signed)
						if (X509_check_issued(cert.get(), cert.get()) == X509_V_OK) {
							LOG_DEBUG("Found root CA: " + CertificateUtil::getSubjectName(cert.get()));

							// Store the first root CA as our primary root
							if (!m_rootCA) {
								m_rootCA = std::move(cert);
							} else {
								// Store additional roots for chain verification
								m_certPool.push_back(std::move(cert));
							}
						} else {
							// Store intermediate certificates
							m_certPool.push_back(std::move(cert));
						}
					}

					if (!m_rootCA) {
						LOG_WARNING("No root CA found in directory");
					}
				} else {
					// load certificate chain from single file
					LOG_INFO("Loading certificates from file: " + cpath);
					auto caChain = CertificateUtil::loadCertificateChain(cpath);

					if (!caChain.empty()) {
						m_rootCA = std::move(caChain[0]);

						for (size_t i = 1; i < caChain.size(); ++i) {
							m_intermediateCA.push_back(std::move(caChain[i]));
						}

						LOG_INFO("Loaded root CA: " + CertificateUtil::getSubjectName(m_rootCA.get()));

						for (const auto& cert : m_intermediateCA) {
							LOG_INFO("Loaded intermediate CA: " + CertificateUtil::getSubjectName(cert.get()));

							// add intermediates to certificate pool for compatibility
							m_certPool.push_back(std::unique_ptr<X509, X509Deleter>(X509_dup(cert.get())));
						}
					}
				}
			} catch (const std::exception& e) {
				LOG_ERROR("Failed to load certificates: " + std::string(e.what()));
				throw;
			}
		}
	}

	~RSPClient() {
		// OpenSSL 3.0+ handles cleanup automatically
	}

	void setCACertPath(const std::string& path) {
		m_caCertPath = path;
	}

	void loadEUICCCertificate(const std::string& euiccCertPath) {
		loadCertificate(euiccCertPath, "eUICC", m_euiccCert);

		try {
			m_EID = CertificateUtil::getEID(m_euiccCert.get());
			LOG_DEBUG("Using EID from certificate: " + m_EID);

			auto ski = CertificateUtil::getSubjectKeyIdentifier(m_euiccCert.get());
			m_euiccSKI = ski;
			LOG_DEBUG("Using eUICC SKI as PKID: " + HexUtil::bytesToHex(m_euiccSKI));
		} catch (const std::exception& e) {
			LOG_ERROR("Failed to extract EUICC-specific data: " + std::string(e.what()));
			throw;
		}
	}

	void loadEUICCKeyPair(const std::string& euiccPrivateKeyPath, const std::string& euiccPublicKeyPath = "") {
		loadKeyPair(euiccPrivateKeyPath, euiccPublicKeyPath, "eUICC", m_euiccPrivateKey, m_euiccPublicKeyData);
	}

	void loadEUMCertificate(const std::string& eumCertPath) {
		loadCertificate(eumCertPath, "EUM", m_eumCert);
	}

	void loadEUMKeyPair(const std::string& eumPrivateKeyPath, const std::string& eumPublicKeyPath = "") {
		loadKeyPair(eumPrivateKeyPath, eumPublicKeyPath, "EUM", m_eumPrivateKey, m_eumPublicKeyData);
	}

	std::vector<uint8_t> generateChallenge() {
		std::vector<uint8_t> challenge(16);
		if (RAND_bytes(challenge.data(), challenge.size()) != 1) {
			throw OpenSSLError("Failed to generate random challenge");
		}
		return challenge;
	}

	int getCertificateCurveNID(X509* cert) {
		if (!cert) {
			throw std::runtime_error("Certificate is null");
		}

		std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pubkey(X509_get_pubkey(cert));
		if (!pubkey) {
			throw OpenSSLError("Failed to extract public key from certificate");
		}

		char curve_name[80];
		size_t curve_name_len = sizeof(curve_name);
		if (EVP_PKEY_get_utf8_string_param(pubkey.get(), OSSL_PKEY_PARAM_GROUP_NAME, curve_name, sizeof(curve_name), &curve_name_len) != 1) {
			throw OpenSSLError("Failed to get curve name from certificate");
		}

		std::string curve_str(curve_name);
		int curve_nid;

		if (curve_str == "prime256v1" || curve_str == "P-256") {
			curve_nid = NID_X9_62_prime256v1;
		} else if (curve_str == "brainpoolP256r1") {
			curve_nid = NID_brainpoolP256r1;
		} else {
			throw std::runtime_error("Unsupported curve in certificate: " + curve_str);
		}

		return curve_nid;
	}

	int getEUICCCurveNID() {
		if (!m_euiccCert) {
			throw std::runtime_error("eUICC certificate not loaded");
		}

		int curve_nid = getCertificateCurveNID(m_euiccCert.get());

		if (curve_nid == NID_X9_62_prime256v1) {
			LOG_INFO("eUICC certificate uses P-256 curve");
		} else if (curve_nid == NID_brainpoolP256r1) {
			LOG_INFO("eUICC certificate uses brainpoolP256r1 curve");
		}

		return curve_nid;
	}

	void generateEUICCOtpk() {
		int curve_nid = getEUICCCurveNID();
		std::string curve_name = (curve_nid == NID_X9_62_prime256v1) ? "P-256" : "brainpoolP256r1";

		std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> pctx(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr), EVP_PKEY_CTX_free);
		if (!pctx) {
			throw OpenSSLError("Failed to create EVP_PKEY_CTX");
		}

		if (EVP_PKEY_keygen_init(pctx.get()) <= 0) {
			throw OpenSSLError("Failed to initialize key generation");
		}

		if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx.get(), curve_nid) <= 0) {
			throw OpenSSLError("Failed to set " + curve_name + " curve");
		}

		EVP_PKEY* pkey_raw = nullptr;
		if (EVP_PKEY_keygen(pctx.get(), &pkey_raw) <= 0) {
			throw OpenSSLError("Failed to generate EC key pair");
		}

		m_euicc_ot_PrivateKey.reset(pkey_raw);

		size_t pub_len = 0;

		if (EVP_PKEY_get_octet_string_param(m_euicc_ot_PrivateKey.get(), OSSL_PKEY_PARAM_PUB_KEY, nullptr, 0, &pub_len) != 1) {
			throw OpenSSLError("Failed to get public key size");
		}

		// Both P-256 and brainpoolP256r1 have 256-bit (32 byte) coordinates
		// Uncompressed format: 0x04 + 32 bytes X + 32 bytes Y = 65 bytes
		if (pub_len != 65) {
			throw OpenSSLError("Unexpected public key size for " + curve_name);
		}

		m_euiccOtpk.resize(pub_len);
		if (EVP_PKEY_get_octet_string_param(m_euicc_ot_PrivateKey.get(), OSSL_PKEY_PARAM_PUB_KEY, m_euiccOtpk.data(), pub_len, &pub_len) != 1) {
			throw OpenSSLError("Failed to extract public key");
		}

		LOG_INFO("Generated eUICC OtPK (" + curve_name + "): " + HexUtil::bytesToHex(m_euiccOtpk));

		if (m_euiccOtpk[0] != 0x04) {
			throw std::runtime_error("Invalid public key format - expected uncompressed point");
		}

		LOG_DEBUG("Public key format verified - uncompressed EC point");
	}

	std::vector<uint8_t> computeECDHSharedSecret(const std::vector<uint8_t>& otherPublicKey) {
		if (!m_euicc_ot_PrivateKey) {
			throw std::runtime_error("eUICC ephemeral private key not available");
		}

		int curve_nid = getEUICCCurveNID();
		const char* curve_param_name = (curve_nid == NID_X9_62_prime256v1) ? "P-256" : "brainpoolP256r1";

		std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> pctx(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr), EVP_PKEY_CTX_free);
		if (!pctx) {
			throw OpenSSLError("Failed to create EVP_PKEY_CTX");
		}

		if (EVP_PKEY_fromdata_init(pctx.get()) <= 0) {
			throw OpenSSLError("Failed to initialize fromdata");
		}

		OSSL_PARAM_BLD* param_bld = OSSL_PARAM_BLD_new();
		if (!param_bld) {
			throw OpenSSLError("Failed to create OSSL_PARAM_BLD");
		}

		if (!OSSL_PARAM_BLD_push_utf8_string(param_bld, OSSL_PKEY_PARAM_GROUP_NAME, curve_param_name, 0)) {
			OSSL_PARAM_BLD_free(param_bld);
			throw OpenSSLError("Failed to set curve name");
		}

		if (!OSSL_PARAM_BLD_push_octet_string(param_bld, OSSL_PKEY_PARAM_PUB_KEY, otherPublicKey.data(), otherPublicKey.size())) {
			OSSL_PARAM_BLD_free(param_bld);
			throw OpenSSLError("Failed to set public key");
		}

		OSSL_PARAM* params = OSSL_PARAM_BLD_to_param(param_bld);
		if (!params) {
			OSSL_PARAM_BLD_free(param_bld);
			throw OpenSSLError("Failed to build params");
		}

		EVP_PKEY* other_pkey_raw = nullptr;
		if (EVP_PKEY_fromdata(pctx.get(), &other_pkey_raw, EVP_PKEY_PUBLIC_KEY, params) <= 0) {
			OSSL_PARAM_free(params);
			OSSL_PARAM_BLD_free(param_bld);
			throw OpenSSLError("Failed to create public key from data");
		}

		OSSL_PARAM_free(params);
		OSSL_PARAM_BLD_free(param_bld);

		std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> other_pkey(other_pkey_raw);

		std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> derive_ctx(EVP_PKEY_CTX_new(m_euicc_ot_PrivateKey.get(), nullptr),
										       EVP_PKEY_CTX_free);
		if (!derive_ctx) {
			throw OpenSSLError("Failed to create derive context");
		}

		if (EVP_PKEY_derive_init(derive_ctx.get()) <= 0) {
			throw OpenSSLError("Failed to initialize key derivation");
		}

		if (EVP_PKEY_derive_set_peer(derive_ctx.get(), other_pkey.get()) <= 0) {
			throw OpenSSLError("Failed to set peer key");
		}

		size_t secret_len = 0;
		if (EVP_PKEY_derive(derive_ctx.get(), nullptr, &secret_len) <= 0) {
			throw OpenSSLError("Failed to get shared secret length");
		}

		std::vector<uint8_t> shared_secret(secret_len);
		if (EVP_PKEY_derive(derive_ctx.get(), shared_secret.data(), &secret_len) <= 0) {
			throw OpenSSLError("ECDH computation failed");
		}

		shared_secret.resize(secret_len);

		LOG_INFO("Computed ECDH shared secret: " + HexUtil::bytesToHex(shared_secret));
		return shared_secret;
	}

	std::vector<uint8_t> signDataWithEUICC(const std::vector<uint8_t>& dataToSign) {
		if (!m_euiccPrivateKey) {
			throw std::runtime_error("eUICC private key not available for signing");
		}
		auto rv = signDataWithKey(dataToSign, m_euiccPrivateKey.get());
		return CertificateUtil::convertDER_to_BSI_TR03111(rv);
	}

	bool verifyServerSignature(const std::vector<uint8_t>& serverSigned1, const std::vector<uint8_t>& serverSignature1,
				   const std::vector<uint8_t>& derDataServerCert) {
		auto serverCert = CertificateUtil::loadCertFromDER(derDataServerCert);
		return verifyServerSignature(serverSigned1, serverSignature1, serverCert.get());
	}

	void setConfirmationCode(const std::string& confirmationCode) {
		m_confirmationCode = confirmationCode;
		if (!m_transactionId.empty()) {
			m_confirmationCodeHash = computeConfirmationCodeHash(m_confirmationCode, m_transactionId);
			LOG_INFO("Set confirmation code, computed hash: " + HexUtil::bytesToHex(m_confirmationCodeHash));
		} else {
			LOG_ERROR("Cannot compute confirmation code hash - transaction ID not set");
		}
	}

	void setTransactionId(const std::vector<uint8_t>& transactionId) {
		m_transactionId = transactionId;
		LOG_INFO("Set transaction ID: " + HexUtil::bytesToHex(transactionId));
		// If confirmation code was already set, recalculate hash
		if (!m_confirmationCode.empty()) {
			m_confirmationCodeHash = computeConfirmationCodeHash(m_confirmationCode, m_transactionId);
			LOG_INFO("Recalculated confirmation code hash: " + HexUtil::bytesToHex(m_confirmationCodeHash));
		}
	}

	std::vector<uint8_t> getConfirmationCodeHash() const {
		return m_confirmationCodeHash;
	}

	void setConfirmationCodeHash(const std::vector<uint8_t>& hash) {
		m_confirmationCodeHash = hash;
		LOG_INFO("Set confirmation code hash directly: " + HexUtil::bytesToHex(hash));
	}

	std::vector<uint8_t> getEUMCertificate() {
		return CertificateUtil::certToDER(m_eumCert.get());
	}

	std::vector<uint8_t> getEUICCCertificate() {
		return CertificateUtil::certToDER(m_euiccCert.get());
	}

	std::vector<uint8_t> getCICertificate() {
		// If no eUICC certificate is loaded, return the primary root CA
		if (!m_euiccCert) {
			if (m_rootCA) {
				return CertificateUtil::certToDER(m_rootCA.get());
			}
			return std::vector<uint8_t>();
		}

		int euicc_curve_nid = getEUICCCurveNID();

		if (m_rootCA) {
			try {
				int root_curve_nid = getCertificateCurveNID(m_rootCA.get());
				if (root_curve_nid == euicc_curve_nid) {
					LOG_DEBUG("Primary root CA matches eUICC curve type");
					return CertificateUtil::certToDER(m_rootCA.get());
				}
			} catch (const std::exception& e) {
				LOG_WARNING("Failed to get curve type from primary root CA: " + std::string(e.what()));
			}
		}

		// Search for a matching root CA in the certificate pool
		for (const auto& cert : m_certPool) {
			// Check if this is a self-signed certificate (root CA)
			if (X509_check_issued(cert.get(), cert.get()) == X509_V_OK) {
				try {
					int cert_curve_nid = getCertificateCurveNID(cert.get());
					if (cert_curve_nid == euicc_curve_nid) {
						LOG_DEBUG("Found matching root CA in certificate pool for curve type");
						return CertificateUtil::certToDER(cert.get());
					}
				} catch (const std::exception& e) {
					LOG_WARNING("Failed to get curve type from certificate: " + std::string(e.what()));
				}
			}
		}

		// If no matching root CA found, return the primary root CA as fallback
		LOG_WARNING("No root CA found matching eUICC curve type, returning primary "
			    "root CA");
		if (m_rootCA) {
			return CertificateUtil::certToDER(m_rootCA.get());
		}
		return std::vector<uint8_t>();
	}

	std::vector<uint8_t> getEUICCOtpk() {
		return m_euiccOtpk;
	}

    private:
	void loadCertificate(const std::string& certPath, const std::string& certType, std::unique_ptr<X509, X509Deleter>& certStorage) {
		CertificateUtil::xx_loadCertificate(certPath, certType, certStorage);
	}

	void loadKeyPair(const std::string& privateKeyPath, const std::string& publicKeyPath, const std::string& keyType,
			 std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter>& privateKeyStorage, std::vector<uint8_t>& publicKeyStorage) {
		CertificateUtil::xx_loadPrivateKey(privateKeyPath, keyType, privateKeyStorage);

		if (!publicKeyPath.empty() && CertificateUtil::xx_loadPublicKey(publicKeyPath, keyType, publicKeyStorage)) {
			return;
		}

		CertificateUtil::xx_generatePublicKeyFromPrivate(privateKeyStorage, keyType, publicKeyStorage);
	}

	std::vector<uint8_t> signDataWithKey(const std::vector<uint8_t>& dataToSign, EVP_PKEY* pkey) {
		if (dataToSign.empty()) {
			throw std::runtime_error("Data to sign is empty");
		}

		std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdctx(EVP_MD_CTX_new());
		if (!mdctx) {
			throw OpenSSLError("Failed to create signature context");
		}

		if (EVP_DigestSignInit(mdctx.get(), nullptr, EVP_sha256(), nullptr, pkey) != 1) {
			throw OpenSSLError("Failed to initialize signature operation");
		}

		if (EVP_DigestSignUpdate(mdctx.get(), dataToSign.data(), dataToSign.size()) != 1) {
			throw OpenSSLError("Failed to update signature");
		}

		size_t sigLen;
		if (EVP_DigestSignFinal(mdctx.get(), nullptr, &sigLen) != 1) {
			throw OpenSSLError("Failed to determine signature length");
		}

		if (sigLen == 0) {
			throw std::runtime_error("signature length is zero");
		}

		std::vector<uint8_t> signature(sigLen);
		if (EVP_DigestSignFinal(mdctx.get(), signature.data(), &sigLen) != 1) {
			throw OpenSSLError("Failed to create signature");
		}

		signature.resize(sigLen);
		LOG_DEBUG("Created signature (" + std::to_string(sigLen) + " bytes) over " + std::to_string(dataToSign.size()) + " bytes of data");

		return signature;
	}

	bool verifyServerSignature(const std::vector<uint8_t>& signedData, const std::vector<uint8_t>& signature, X509* scertdata) {
		try {
			// Handle prefix in serverSignature1 based on SGP.22 spec
			const unsigned char* signatureData = signature.data();
			size_t signatureLen = signature.size();

			if (signatureLen >= 3 && signatureData[0] == 0x5f && signatureData[1] == 0x37 && signatureData[2] == 0x40) {
				LOG_DEBUG("Detected 5F3740 prefix in serverSignature1, skipping for "
					  "verification");
				signatureData += 3;
				signatureLen -= 3;
			}

			std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pubkey(X509_get_pubkey(scertdata));
			if (!pubkey) {
				throw OpenSSLError("Failed to extract public key from certificate");
			}

			auto ski = CertificateUtil::getSubjectKeyIdentifier(scertdata);
			LOG_DEBUG(" using cert with this SKI to verify sig: " + HexUtil::bytesToHex(ski));

			auto verifyResult = CertificateUtil::verify_TR031111(
				signedData, std::vector<unsigned char>(signatureData, signatureData + signatureLen), pubkey.get());

			if (verifyResult) {
				LOG_INFO("Signature verification successful!");
				return true;
			} else {
				LOG_ERROR("Signature verification failed - invalid signature");
				return false;
			}
		} catch (const std::exception& e) {
			LOG_ERROR("Error verifying signature: " + std::string(e.what()));
			return false;
		}
	}

	bool verifyServerSignature(const std::vector<uint8_t>& serverSigned1, const std::vector<uint8_t>& serverSignature1) {
		return verifyServerSignature(serverSigned1, serverSignature1, m_serverCert.get());
	}

    public:
	void configureHttpClient(bool useCustomTlsCert, const std::string& customTlsCertPath = "") {
		m_useCustomTlsCert = useCustomTlsCert;
		m_customTlsCertPath = customTlsCertPath;
	}

	// Set authentication parameters for mutual TLS auth
	void setAuthParams(bool useMutualTLS, const std::string& clientCertPath, const std::string& clientKeyPath) {
		m_useMutualTLS = useMutualTLS;
		m_clientCertPath = clientCertPath;
		m_clientKeyPath = clientKeyPath;
		LOG_DEBUG("Auth params set: mutualTLS=" + std::to_string(useMutualTLS) + ", cert=" + clientCertPath + ", key=" + clientKeyPath);
	}

	// Send HTTPS POST without client authentication (for ES9+)
	std::string sendHttpsPost(const std::string& endpoint, const std::string& body, int& httpStatusCode, unsigned int portOverride) {
		return sendHttpsPostUnified(endpoint, body, httpStatusCode, portOverride);
	}

	// Send HTTPS POST with pre-configured client authentication (for ES2+)
	std::string sendHttpsPostWithAuth(const std::string& endpoint, const std::string& body, int& httpStatusCode, unsigned int portOverride) {
		if (!m_useMutualTLS) {
			LOG_WARNING("sendHttpsPostWithAuth called but mutual TLS not configured");
		}
		return sendHttpsPostUnified(endpoint, body, httpStatusCode, portOverride, m_useMutualTLS, m_clientCertPath, m_clientKeyPath);
	}

	std::string sendHttpsPostWithContentType(const std::string& endpoint, const std::string& body, const std::string& contentType, int& httpStatusCode,
						 unsigned int portOverride) {
		LOG_DEBUG("sendHttpsPostWithContentType: endpoint=" + endpoint + ", contentType=" + contentType + ", bodySize=" + std::to_string(body.size()) +
			  ", port=" + std::to_string(portOverride));
		return sendHttpsPostUnified(endpoint, body, httpStatusCode, portOverride, false, "", "", contentType);
	}

	std::string sendHttpsPostUnified(const std::string& endpoint, const std::string& body, int& httpStatusCode, unsigned int portOverride,
					 bool useMutualTLS = false, const std::string& clientCertPath = "", const std::string& clientKeyPath = "",
					 const std::string& contentType = "application/json") {
		if (!m_httpClient) {
			m_httpClient = std::make_unique<HttpClient>();
		}

		// full URL without port (port set via CURLOPT_PORT)
		std::string url = "https://" + m_serverUrl + endpoint;

		// Build certificate pool for server verification
		std::vector<X509*> rawCertPool;
		for (const auto& cert : m_certPool) {
			rawCertPool.push_back(cert.get());
		}

		// Add custom TLS server certificate if configured globally
		if (m_useCustomTlsCert && !m_customTlsCertPath.empty()) {
			try {
				auto tlsCerts = CertificateUtil::loadCertificateChain(m_customTlsCertPath);
				for (auto& cert : tlsCerts) {
					rawCertPool.push_back(cert.get());
				}
				LOG_DEBUG("Added custom TLS server certificate from: " + m_customTlsCertPath);
			} catch (const std::exception& e) {
				LOG_WARNING("Failed to load custom TLS server certificate: " + std::string(e.what()));
			}
		}

		HttpClient::PostConfig config;
		config.useMutualTLS = useMutualTLS;
		config.clientCertPath = clientCertPath;
		config.clientKeyPath = clientKeyPath;
		config.includeAdminProtocolHeader = useMutualTLS; // ES2+ requires this header
		config.verboseOutput = false;
		config.contentType = contentType;

		LOG_DEBUG("Sending " + std::string(useMutualTLS ? "ES2+ request with mutual TLS" : "HTTPS request") + " to: " + endpoint +
			  " (port: " + std::to_string(portOverride) + ")");

		HttpClient::ResponseData response = m_httpClient->postJson(url, portOverride, body, nullptr, rawCertPool, config);

		httpStatusCode = response.statusCode;

		LOG_DEBUG("HTTP " + std::to_string(httpStatusCode) + " response, body length: " + std::to_string(response.body.length()));

		return response.body;
	}

	std::string m_serverUrl;

	std::unique_ptr<X509, X509Deleter> m_rootCA;
	std::vector<std::unique_ptr<X509, X509Deleter>> m_intermediateCA;
	std::unique_ptr<X509, X509Deleter> m_serverCert;
	std::vector<std::unique_ptr<X509, X509Deleter>> m_certPool;

	std::vector<uint8_t> m_euiccSKI; // eUICC SKI for PKIDs
	std::string m_EID; // eUICC ID
	std::unique_ptr<X509, X509Deleter> m_euiccCert; // eUICC certificate
	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_euiccPrivateKey; // eUICC private key for signing
	std::vector<uint8_t> m_euiccPublicKeyData; // eUICC public key data
	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_euicc_ot_PrivateKey; // One-time private key
	std::vector<uint8_t> m_euiccOtpk; // One-time public key from eUICC

	std::unique_ptr<X509, X509Deleter> m_eumCert; // EUM certificate
	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_eumPrivateKey; // EUM private key for signing
	std::vector<uint8_t> m_eumPublicKeyData; // EUM public key data

	std::string m_confirmationCode; // Confirmation code (PIN)
	std::vector<uint8_t> m_confirmationCodeHash; // Computed confirmation code hash
	std::vector<uint8_t> m_transactionId; // Transaction ID for hash computation
	std::string m_caCertPath = ""; // Path to system CA certificates

	std::unique_ptr<HttpClient> m_httpClient;
	bool m_useCustomTlsCert = true; // Flag for custom vs public CA
	std::string m_customTlsCertPath; // Path to custom TLS certificate

	// Authentication parameters for mutual TLS auth
	bool m_useMutualTLS = false;
	std::string m_clientCertPath = "";
	std::string m_clientKeyPath = "";
};

} // namespace RspCrypto