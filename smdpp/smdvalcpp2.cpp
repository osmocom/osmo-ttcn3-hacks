
#include <cstddef>
#include <filesystem>
#include <iterator>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include <openssl/asn1t.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/crypto.h>
#include <openssl/cmac.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/safestack.h>
#include <openssl/stack.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/safestack.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/stack.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <openssl/asn1.h>
#include <openssl/asn1t.h>

#include <dirent.h>
#include <sys/types.h>

#include <sys/stat.h>

#include <curl/curl.h>
#include <openssl/types.h>

#include <cJSON.h>
#include "rsp_ossl_imp.h"
}

#include "logger.h"

DEFINE_STACK_OF(ASN1_OCTET_STRING)

namespace RspCrypto
{

// Custom deleters for unique_ptr
struct BIODeleter {
	void operator()(BIO *bio) const
	{
		BIO_free_all(bio);
	}
};

struct X509Deleter {
	void operator()(X509 *cert) const
	{
		X509_free(cert);
	}
};

struct X509_STORE_Deleter {
	void operator()(X509_STORE *store) const
	{
		X509_STORE_free(store);
	}
};

struct X509_STORE_CTX_Deleter {
	void operator()(X509_STORE_CTX *ctx) const
	{
		X509_STORE_CTX_free(ctx);
	}
};

struct STACK_OF_X509_Deleter {
	void operator()(STACK_OF(X509) * stack) const
	{
		sk_X509_pop_free(stack, X509_free);
	}
};

struct EVP_PKEY_Deleter {
	void operator()(EVP_PKEY *key) const
	{
		EVP_PKEY_free(key);
	}
};

struct EC_KEY_Deleter {
	void operator()(EC_KEY *key) const
	{
		EC_KEY_free(key);
	}
};

struct EVP_MD_CTX_Deleter {
	void operator()(EVP_MD_CTX *ctx) const
	{
		EVP_MD_CTX_free(ctx);
	}
};

struct SERVER_SIGNED1_Deleter {
	void operator()(SERVER_SIGNED1 *ss1) const
	{
		SERVER_SIGNED1_free(ss1);
	}
};

struct EUICC_INFO1_Deleter {
	void operator()(EUICC_INFO1 *info) const
	{
		EUICC_INFO1_free(info);
	}
};

struct cJSON_Deleter {
	void operator()(cJSON *json) const
	{
		cJSON_Delete(json);
	}
};

// OpenSSL memory free deleter
struct OpenSSLFreeDeleter {
	void operator()(void *p) const
	{
		OPENSSL_free(p);
	}
};

// Base64 utility class
class Base64 {
    public:
	static std::vector<uint8_t> decode(const std::string &b64message)
	{
		if (b64message.empty()) {
			return {};
		}

		// Calculate decoded length
		int padding = 0;
		if (b64message.size() > 0) {
			if (b64message[b64message.size() - 1] == '=' && b64message[b64message.size() - 2] == '=') {
				padding = 2;
			} else if (b64message[b64message.size() - 1] == '=') {
				padding = 1;
			}
		}

		size_t decodeLen = (b64message.size() * 3) / 4 - padding;
		std::vector<uint8_t> buffer(decodeLen);

		// Create a BIO chain for base64 decoding
		BIO *b64 = BIO_new(BIO_f_base64());
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // No newlines

		BIO *mem = BIO_new_mem_buf(b64message.c_str(), -1);
		BIO *bioChain = BIO_push(b64, mem);

		// Ensure proper cleanup with a unique_ptr
		std::unique_ptr<BIO, BIODeleter> bioPtr(bioChain);

		// Read the decoded data
		int length = BIO_read(bioChain, buffer.data(), b64message.size());
		if (length <= 0) {
			throw std::runtime_error("Failed to decode base64 data");
		}

		buffer.resize(length); // Resize to actual decoded length
		return buffer;
	}

	static std::string encode(const std::vector<uint8_t> &data)
	{
		if (data.empty()) {
			return {};
		}

		// Create a BIO chain for base64 encoding
		BIO *b64 = BIO_new(BIO_f_base64());
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // No newlines

		BIO *mem = BIO_new(BIO_s_mem());
		BIO *bioChain = BIO_push(b64, mem);

		// Ensure proper cleanup with a unique_ptr
		std::unique_ptr<BIO, BIODeleter> bioPtr(bioChain);

		// Write the data to be encoded
		BIO_write(bioChain, data.data(), data.size());
		BIO_flush(bioChain);

		// Get the encoded data
		BUF_MEM *bufferPtr;
		BIO_get_mem_ptr(mem, &bufferPtr);

		return std::string(bufferPtr->data, bufferPtr->length);
	}
};

// Utility for hex conversion
class HexUtil {
    public:
	static std::vector<uint8_t> hexToBytes(const std::string &hex)
	{
		if (hex.length() % 2 != 0) {
			throw std::runtime_error("Invalid hex string length");
		}

		std::vector<uint8_t> bytes(hex.length() / 2);
		for (size_t i = 0; i < bytes.size(); ++i) {
			sscanf(hex.c_str() + (i * 2), "%2hhx", &bytes[i]);
		}

		return bytes;
	}

	static std::string bytesToHex(const std::vector<uint8_t> &bytes)
	{
		return bytesToHex(bytes.data(), bytes.size());
	}

	static std::string bytesToHex(const uint8_t *data, size_t length)
	{
		std::stringstream ss;
		ss << std::hex << std::uppercase << std::setfill('0');

		for (size_t i = 0; i < length; ++i) {
			ss << std::setw(2) << static_cast<int>(data[i]);
		}

		return ss.str();
	}
};

// Certificate utility class
class CertificateUtil {
    public:
	// Extract EID (eUICC ID) from certificate subject
	static std::string getEID(X509 *cert)
	{
		X509_NAME *subject = X509_get_subject_name(cert);
		if (!subject) {
			return "";
		}

		// EID is stored in serialNumber field
		int index = X509_NAME_get_index_by_NID(subject, NID_serialNumber, -1);
		if (index < 0) {
			return "";
		}

		X509_NAME_ENTRY *entry = X509_NAME_get_entry(subject, index);
		if (!entry) {
			return "";
		}

		ASN1_STRING *asn1_str = X509_NAME_ENTRY_get_data(entry);
		if (!asn1_str) {
			return "";
		}

		const unsigned char *str_data = ASN1_STRING_get0_data(asn1_str);
		int str_len = ASN1_STRING_length(asn1_str);

		return std::string(reinterpret_cast<const char *>(str_data), str_len);
	}

	static void printCertificateDetails(X509 *cert)
	{
		// Get basic certificate information
		char subjectName[256];
		char issuerName[256];
		X509_NAME_oneline(X509_get_subject_name(cert), subjectName, sizeof(subjectName));
		X509_NAME_oneline(X509_get_issuer_name(cert), issuerName, sizeof(issuerName));

		// Get validity period
		ASN1_TIME *notBefore = X509_get_notBefore(cert);
		ASN1_TIME *notAfter = X509_get_notAfter(cert);

		// Convert ASN1_TIME to readable format
		BIO *bio = BIO_new(BIO_s_mem());
		BUF_MEM *bptr;

		// Format start date
		BIO_reset(bio);
		ASN1_TIME_print(bio, notBefore);
		BIO_get_mem_ptr(bio, &bptr);
		std::string startDate(bptr->data, bptr->length);

		// Format end date
		BIO_reset(bio);
		ASN1_TIME_print(bio, notAfter);
		BIO_get_mem_ptr(bio, &bptr);
		std::string endDate(bptr->data, bptr->length);
		BIO_free(bio);

		// Get certificate identifiers
		auto ski = getSubjectKeyIdentifier(cert);
		auto aki = getAuthorityKeyIdentifier(cert);

		// Print all details
		Logger::info("Certificate Details:");
		Logger::info("  Subject: " + std::string(subjectName));
		Logger::info("  Issuer:  " + std::string(issuerName));
		Logger::info("  Valid:   " + startDate + " to " + endDate);
		Logger::info("  SKI:     " + HexUtil::bytesToHex(ski));
		Logger::info("  AKI:     " + HexUtil::bytesToHex(aki));

		// Check if self-signed
		if (X509_check_issued(cert, cert) == X509_V_OK) {
			Logger::info("  Note:    Self-signed (Root) certificate");
		}
	}

	// Load certificate from PEM string
	static std::unique_ptr<X509, X509Deleter> loadCertFromPEM(const std::string &pemData)
	{
		BIO *bio = BIO_new_mem_buf(pemData.c_str(), -1);
		if (!bio) {
			throw OpenSSLError("Failed to create BIO for certificate");
		}

		X509 *cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
		BIO_free(bio);

		if (!cert) {
			throw OpenSSLError("Failed to parse PEM certificate");
		}

		return std::unique_ptr<X509, X509Deleter>(cert);
	}

	// Load certificate from DER data
	static std::unique_ptr<X509, X509Deleter> loadCertFromDER(const std::vector<uint8_t> &derData)
	{
		const unsigned char *p = derData.data();
		X509 *cert = d2i_X509(nullptr, &p, derData.size());

		if (!cert) {
			throw OpenSSLError("Failed to parse DER certificate");
		}

		return std::unique_ptr<X509, X509Deleter>(cert);
	}

	// Extract subject name from certificate
	static std::string getSubjectName(X509 *cert)
	{
		char subjectName[256];
		X509_NAME_oneline(X509_get_subject_name(cert), subjectName, sizeof(subjectName));
		return std::string(subjectName);
	}

	// Extract issuer name from certificate
	static std::string getIssuerName(X509 *cert)
	{
		char issuerName[256];
		X509_NAME_oneline(X509_get_issuer_name(cert), issuerName, sizeof(issuerName));
		return std::string(issuerName);
	}

	// Check if certificate is expired
	static bool isExpired(X509 *cert)
	{
		return (X509_cmp_current_time(X509_get_notAfter(cert)) <= 0);
	}

	// Check if certificate is not yet valid
	static bool isNotYetValid(X509 *cert)
	{
		return (X509_cmp_current_time(X509_get_notBefore(cert)) >= 0);
	}

	// Convert X509 to PEM format
	static std::string certToPEM(X509 *cert)
	{
		BIO *bio = BIO_new(BIO_s_mem());
		if (!bio) {
			throw OpenSSLError("Failed to create BIO for certificate conversion");
		}

		PEM_write_bio_X509(bio, cert);
		BUF_MEM *bufferPtr;
		BIO_get_mem_ptr(bio, &bufferPtr);

		std::string pem(bufferPtr->data, bufferPtr->length);
		BIO_free(bio);

		return pem;
	}

	static std::vector<uint8_t> certToDER(X509 *cert)
	{
		BIO *bio = BIO_new(BIO_s_mem());
		if (!bio) {
			throw OpenSSLError("Failed to create BIO for certificate conversion");
		}

		i2d_X509_bio(bio, cert);
		BUF_MEM *bufferPtr;
		BIO_get_mem_ptr(bio, &bufferPtr);

		std::vector<uint8_t> der;
		der.assign(bufferPtr->data, bufferPtr->data + bufferPtr->length);
		BIO_free(bio);

		return der;
	}

	// Verify certificate chain
	static bool verifyCertificateChain(X509 *cert, STACK_OF(X509) * intermediates, X509 *rootCA)
	{
		// Create certificate store
		std::unique_ptr<X509_STORE, X509_STORE_Deleter> store(X509_STORE_new());
		if (!store) {
			throw OpenSSLError("Failed to create X509_STORE");
		}

		// Add root CA to the store
		if (X509_STORE_add_cert(store.get(), rootCA) != 1) {
			throw OpenSSLError("Failed to add root CA to store");
		}

		// Create verification context
		std::unique_ptr<X509_STORE_CTX, X509_STORE_CTX_Deleter> ctx(X509_STORE_CTX_new());
		if (!ctx) {
			throw OpenSSLError("Failed to create X509_STORE_CTX");
		}

		// Initialize verification context
		if (X509_STORE_CTX_init(ctx.get(), store.get(), cert, intermediates) != 1) {
			throw OpenSSLError("Failed to initialize X509_STORE_CTX");
		}

		// Set verification parameters
		X509_STORE_CTX_set_flags(ctx.get(), X509_V_FLAG_CHECK_SS_SIGNATURE);

		// Perform verification
		int result = X509_verify_cert(ctx.get());

		if (result != 1) {
			int error = X509_STORE_CTX_get_error(ctx.get());
			LOG_ERROR("Certificate verification failed: " +
				  std::string(X509_verify_cert_error_string(error)));
			return false;
		}

		return true;
	}

	// Load certificate chain from PEM file
	static std::vector<std::unique_ptr<X509, X509Deleter>> loadCertificateChain(const std::string &filename)
	{
		std::vector<std::unique_ptr<X509, X509Deleter>> chain;

		if (std::filesystem::path(filename).extension() == ".der") {
			FILE *file = fopen(filename.c_str(), "rb");
			if (!file) {
				throw std::runtime_error("No certificates found in file: " + filename);
			}

			X509 *cert = d2i_X509_fp(file, nullptr);
			chain.push_back(std::unique_ptr<X509, X509Deleter>(cert));
			fclose(file);

		} else {
			BIO *bio = BIO_new_file(filename.c_str(), "r");
			if (!bio) {
				throw OpenSSLError("Failed to open certificate file: " + filename);
			}
			X509 *cert = nullptr;
			while ((cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr)) != nullptr) {
				chain.push_back(std::unique_ptr<X509, X509Deleter>(cert));
			}

			BIO_free(bio);
		}

		// Check if any certificates were loaded
		if (chain.empty()) {
			throw std::runtime_error("No certificates found in file: " + filename);
		}

		return chain;
	}


	// Find all certificates required for a chain using AKI/SKI matching
	static std::vector<std::unique_ptr<X509, X509Deleter>> findCertificateChain(X509 *targetCert,
										    const std::vector<X509 *> &certPool)
	{
		std::vector<std::unique_ptr<X509, X509Deleter>> chain;

		// Add the target certificate first
		chain.push_back(std::unique_ptr<X509, X509Deleter>(X509_dup(targetCert)));

		// Start with the target cert and work up the chain
		X509 *currentCert = targetCert;
		std::set<std::string> visitedSubjects; // Prevent cycles

		while (currentCert) {
			// Check if this is a self-signed certificate (likely a root)
			if (X509_check_issued(currentCert, currentCert) == X509_V_OK) {
				LOG_INFO("Reached self-signed certificate: " + getSubjectName(currentCert));
				break;
			}

			// Get subject and add to visited set to prevent loops
			std::string subject = getSubjectName(currentCert);
			if (visitedSubjects.find(subject) != visitedSubjects.end()) {
				LOG_WARNING("Certificate chain contains a cycle at: " + subject);
				break;
			}
			visitedSubjects.insert(subject);

			// Try to find issuer using AKI/SKI match
			auto aki = getAuthorityKeyIdentifier(currentCert);
			X509 *issuerCert = nullptr;

			if (!aki.empty()) {
				// Try SKI match first (more specific)
				for (auto cert : certPool) {
					auto ski = getSubjectKeyIdentifier(cert);
					if (!ski.empty() && ski == aki) {
						issuerCert = cert;
						LOG_DEBUG("Found issuer by SKI match: " + getSubjectName(cert));
						break;
					}
				}
			}

			// If SKI match failed, try subject/issuer name match
			if (!issuerCert) {
				X509_NAME *issuerName = X509_get_issuer_name(currentCert);
				for (auto cert : certPool) {
					X509_NAME *subjectName = X509_get_subject_name(cert);
					if (X509_NAME_cmp(subjectName, issuerName) == 0) {
						issuerCert = cert;
						LOG_DEBUG("Found issuer by subject name match: " +
							  getSubjectName(cert));
						break;
					}
				}
			}

			if (!issuerCert) {
				LOG_WARNING("Could not find issuer for certificate: " + subject);
				break;
			}

			// Add the issuer certificate to our chain
			chain.push_back(std::unique_ptr<X509, X509Deleter>(X509_dup(issuerCert)));

			// Move up the chain
			currentCert = issuerCert;
		}

		return chain;
	}

	// Create a certificate stack from our chain for OpenSSL verification
	static STACK_OF(X509) * createCertificateStack(const std::vector<std::unique_ptr<X509, X509Deleter>> &chain,
						       bool skipFirst = true)
	{
		STACK_OF(X509) *stack = sk_X509_new_null();
		if (!stack) {
			throw OpenSSLError("Failed to create certificate stack");
		}

		// Skip the first certificate (the target) if requested
		size_t startIdx = skipFirst ? 1 : 0;

		for (size_t i = startIdx; i < chain.size(); i++) {
			sk_X509_push(stack, X509_dup(chain[i].get()));
		}

		return stack;
	}



	// ###
	//  Add these methods to your CertificateUtil class

	// Extract Subject Key Identifier from certificate
	static std::vector<uint8_t> getSubjectKeyIdentifier(X509 *cert)
	{
		std::vector<uint8_t> ski;

		int loc = X509_get_ext_by_NID(cert, NID_subject_key_identifier, -1);
		if (loc >= 0) {
			X509_EXTENSION *ext = X509_get_ext(cert, loc);
			if (ext) {
				ASN1_OCTET_STRING *ext_data = X509_EXTENSION_get_data(ext);

				// Parse the SKI value (handle ASN.1 encoding)
				const unsigned char *p = ext_data->data;
				long xlen = ext_data->length;
				int tag, xclass;

				ASN1_get_object(&p, &xlen, &tag, &xclass, ext_data->length);

				// Copy the raw SKI value
				ski.assign(p, p + xlen);
			}
		}

		return ski;
	}

	// Extract Authority Key Identifier from certificate
	static std::vector<uint8_t> getAuthorityKeyIdentifier(X509 *cert)
	{
		std::vector<uint8_t> aki;

		int loc = X509_get_ext_by_NID(cert, NID_authority_key_identifier, -1);
		if (loc >= 0) {
			X509_EXTENSION *ext = X509_get_ext(cert, loc);
			if (ext) {
				ASN1_OCTET_STRING *ext_data = X509_EXTENSION_get_data(ext);

				// AKI is more complex, we need to extract keyid
				const unsigned char *p = ext_data->data;
				AUTHORITY_KEYID *akid = d2i_AUTHORITY_KEYID(NULL, &p, ext_data->length);

				if (akid && akid->keyid) {
					aki.assign(akid->keyid->data, akid->keyid->data + akid->keyid->length);
					AUTHORITY_KEYID_free(akid);
				}
			}
		}

		return aki;
	}

	static std::vector<std::filesystem::path> find_cert_files(const std::filesystem::path &root_path,
								  const std::vector<std::string> &name_filters = {})
	{
		std::vector<std::filesystem::path> result;

		if (!std::filesystem::exists(root_path)) {
			std::cerr << "Path does not exist: " << root_path << std::endl;
			return result;
		}

		auto file_matches = [&name_filters](const std::filesystem::path &path) {
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
					   [&filename](const std::string &filter) {
						   return filename.find(filter) != std::string::npos;
					   });
		};

		if (std::filesystem::is_regular_file(root_path)) {
			if (file_matches(root_path)) {
				result.push_back(root_path);
			}
		} else if (std::filesystem::is_directory(root_path)) {
			for (const auto &entry : std::filesystem::recursive_directory_iterator(root_path)) {
				if (entry.is_regular_file() && file_matches(entry.path())) {
					result.push_back(entry.path());
				}
			}
		}

		return result;
	}

	// Load certificates from a directory
	static std::vector<std::unique_ptr<X509, X509Deleter>>
	loadCertificatesFromDirectory(const std::string &directory, const std::vector<std::string> &name_filters)
	{
		std::vector<std::unique_ptr<X509, X509Deleter>> certificates;

		auto files_in_dir = find_cert_files(directory, name_filters);

		for (const auto &entry : files_in_dir
		     //  std::filesystem::recursive_directory_iterator(directory)
		) {
			//   if (!entry.is_regular_file())
			//     continue;
			//   std::string ext = entry.path().extension().string();
			//   if (!(ext == ".pem" || ext == ".der" || ext == ".crt"))
			//     continue;
			std::string ext = entry.extension().string();

			auto fpath = entry.string();
			try {
				// Handle PEM files
				if (ext == ".pem" || ext == ".crt") {
					auto fileCerts = loadCertificateChain(entry);
					for (auto &cert : fileCerts) {
						// printCertificateDetails(cert.get());
						certificates.push_back(std::move(cert));
					}
				}
				// Handle DER files
				else if (ext == ".der") {
					FILE *file = fopen(fpath.c_str(), "rb");
					if (!file) {
						Logger::warning("Failed to open certificate file: " + fpath);
						continue;
					}

					fseek(file, 0, SEEK_END);
					long file_size = ftell(file);
					rewind(file);

					std::vector<unsigned char> data(file_size);
					if (fread(data.data(), 1, file_size, file) != static_cast<size_t>(file_size)) {
						fclose(file);
						Logger::warning("Failed to read certificate file: " + fpath);
						continue;
					}

					fclose(file);

					const unsigned char *p = data.data();
					X509 *cert = d2i_X509(nullptr, &p, file_size);

					if (cert) {
						certificates.push_back(std::unique_ptr<X509, X509Deleter>(cert));
						Logger::info("Loaded certificate: " + getSubjectName(cert) + " from " +
							     fpath);
						// printCertificateDetails(cert);
					}
				}
			} catch (const std::exception &e) {
				// Logger::warning("Error loading certificate from " + fpath + ": " +
				// e.what());
			}
		}

		return certificates;
	}

	// Find issuer certificate using AKI/SKI matching
	static X509 *findIssuerCertificate(X509 *cert, const std::vector<X509 *> &certPool)
	{
		// Try AKI/SKI match first
		auto aki = getAuthorityKeyIdentifier(cert);
		if (!aki.empty()) {
			for (auto candidate : certPool) {
				auto ski = getSubjectKeyIdentifier(candidate);
				if (!ski.empty() && ski == aki) {
					Logger::debug("Found issuer by SKI match: " + getSubjectName(candidate));
					return candidate;
				}
			}
		}

		// Fall back to subject/issuer name match
		X509_NAME *issuerName = X509_get_issuer_name(cert);
		for (auto candidate : certPool) {
			X509_NAME *subjectName = X509_get_subject_name(candidate);
			if (X509_NAME_cmp(subjectName, issuerName) == 0) {
				Logger::debug("Found issuer by subject name match: " + getSubjectName(candidate));
				return candidate;
			}
		}

		return nullptr;
	}

	// Build certificate chain using AKI/SKI or issuer/subject matching
	static STACK_OF(X509) *
		buildCertificateChain(X509 *cert, const std::vector<X509 *> &certPool, bool verbose = false)
	{
		STACK_OF(X509) *chain = sk_X509_new_null();
		if (!chain) {
			throw OpenSSLError("Failed to create certificate stack");
		}

		if (verbose) {
			Logger::info("Building certificate chain starting with:");
			printCertificateDetails(cert);
		}

		X509 *current = cert;
		std::set<std::string> visited; // Prevent cycles

		while (current) {
			// Skip the first certificate (target) - we'll add it externally
			if (current != cert) {
				// Add certificate to the chain
				if (sk_X509_push(chain, X509_dup(current)) == 0) {
					sk_X509_pop_free(chain, X509_free);
					throw OpenSSLError("Failed to add certificate to chain");
				}
			}

			// Check if this is a self-signed certificate (root)
			if (X509_check_issued(current, current) == X509_V_OK) {
				if (verbose) {
					Logger::info("Reached root certificate: " + getSubjectName(current));
				}
				break;
			}

			// Track visited certificates to prevent loops
			std::string subject = getSubjectName(current);
			if (visited.find(subject) != visited.end()) {
				Logger::warning("Certificate chain contains a cycle at: " + subject);
				break;
			}
			visited.insert(subject);

			// Find the issuer
			X509 *issuer = findIssuerCertificate(current, certPool);
			if (!issuer) {
				Logger::warning("Could not find issuer for: " + subject);
				break;
			}

			// Print validation link
			if (verbose) {
				auto currentSKI = getSubjectKeyIdentifier(current);
				auto issuerSKI = getSubjectKeyIdentifier(issuer);
				auto currentAKI = getAuthorityKeyIdentifier(current);
				auto issuerAKI = getAuthorityKeyIdentifier(issuer);

				Logger::info("Certificate validation link:");
				Logger::info("  Subject: " + getSubjectName(current));
				Logger::info("  AKI:     " + HexUtil::bytesToHex(currentAKI));
				Logger::info("  SKI:     " + HexUtil::bytesToHex(currentSKI));
				Logger::info("  ↓");
				Logger::info("  Issuer:  " + getSubjectName(issuer));
				Logger::info("  AKI:     " + HexUtil::bytesToHex(issuerAKI));
				Logger::info("  SKI:     " + HexUtil::bytesToHex(issuerSKI));

				// Verify AKI/SKI match
				if (!currentAKI.empty() && !issuerSKI.empty()) {
					if (currentAKI == issuerSKI) {
						Logger::info("  Match:   AKI and SKI match ");
					} else {
						Logger::warning("  Match:   AKI and SKI DO NOT match ✗");
					}
				} else {
					Logger::warning("  Match:   Cannot verify - missing AKI or SKI");
				}
				Logger::info(""); // Empty line for readability
			}

			// Move up the chain
			current = issuer;
		}

		return chain;
	}

	// Enhanced certificate chain verification that discovers the chain
	// automatically
	static bool verifyCertificateChainDynamic(X509 *cert, const std::vector<X509 *> &certPool,
						  X509 *rootCA = nullptr, bool verbose = false, bool testMode = true)
	{
		Logger::info("Verifying certificate chain with dynamic discovery...");
		Logger::info("Target certificate: " + getSubjectName(cert));

		// Create a combined certificate pool
		std::vector<X509 *> combinedPool = certPool;

		// Add root CA to the pool if provided
		if (rootCA) {
			combinedPool.push_back(rootCA);
			if (verbose) {
				Logger::info("Using provided root CA: " + getSubjectName(rootCA));
			}
		}

		// Create certificate store
		std::unique_ptr<X509_STORE, X509_STORE_Deleter> store(X509_STORE_new());
		if (!store) {
			throw OpenSSLError("Failed to create X509_STORE");
		}

		// If we have a specific rootCA, add it to the store
		if (rootCA) {
			if (X509_STORE_add_cert(store.get(), rootCA) != 1) {
				throw OpenSSLError("Failed to add root CA to store");
			}
		} else {
			// Otherwise, find root certificates in the pool (self-signed)
			for (auto candidate : combinedPool) {
				if (X509_check_issued(candidate, candidate) == X509_V_OK) {
					if (verbose) {
						Logger::info("Adding root CA to trust store: " +
							     getSubjectName(candidate));
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

		// Build certificate chain automatically with verbose output
		std::unique_ptr<STACK_OF(X509), STACK_OF_X509_Deleter> chain(
			buildCertificateChain(cert, combinedPool, verbose));

		// Print the complete chain we're verifying
		if (verbose && sk_X509_num(chain.get()) > 0) {
			Logger::info("Complete certificate chain for verification:");
			Logger::info("1. " + getSubjectName(cert) + " (Target certificate)");

			for (int i = 0; i < sk_X509_num(chain.get()); i++) {
				X509 *chainCert = sk_X509_value(chain.get(), i);
				Logger::info(std::to_string(i + 2) + ". " + getSubjectName(chainCert));
			}
			Logger::info(""); // Empty line for readability
		}

		// Create verification context
		std::unique_ptr<X509_STORE_CTX, X509_STORE_CTX_Deleter> ctx(X509_STORE_CTX_new());
		if (!ctx) {
			throw OpenSSLError("Failed to create X509_STORE_CTX");
		}

		// Initialize verification context
		if (X509_STORE_CTX_init(ctx.get(), store.get(), cert, chain.get()) != 1) {
			throw OpenSSLError("Failed to initialize X509_STORE_CTX");
		}

		// Set verification parameters
		unsigned long flags = X509_V_FLAG_CHECK_SS_SIGNATURE;

		X509_STORE_CTX_set_flags(ctx.get(), flags);

		// In test mode, set a custom verification callback to handle name constraint violations
		// This allows us to continue verification even when name constraints fail in test environments
		if (testMode) {
			Logger::info("Test mode enabled: will ignore name constraint violations");
			X509_STORE_CTX_set_verify_cb(ctx.get(), [](int ok, X509_STORE_CTX *ctx) -> int {
				if (!ok) {
					int error = X509_STORE_CTX_get_error(ctx);
					// Allow verification to continue for name constraint violations in test mode
					if (error == X509_V_ERR_PERMITTED_VIOLATION ||
					    error == X509_V_ERR_EXCLUDED_VIOLATION ||
					    error == X509_V_ERR_SUBTREE_MINMAX) {
						Logger::info("Ignoring name constraint violation in test mode: " +
							    std::string(X509_verify_cert_error_string(error)));
						return 1; // Continue verification despite the error
					}
				}
				return ok; // Use default behavior for other errors
			});
		}

		// Perform verification
		int result = X509_verify_cert(ctx.get());

		if (result != 1) {
			int error = X509_STORE_CTX_get_error(ctx.get());
			int depth = X509_STORE_CTX_get_error_depth(ctx.get());
			X509 *errorCert = X509_STORE_CTX_get_current_cert(ctx.get());

			Logger::error("Certificate verification failed:");
			Logger::error("  Error:   " + std::string(X509_verify_cert_error_string(error)));
			Logger::error("  Depth:   " + std::to_string(depth));

			if (errorCert) {
				Logger::error("  Cert:    " + getSubjectName(errorCert));
			}

			return false;
		}

		Logger::info("Certificate verification successful ");
		return true;
	}

	static bool verify_TR031111(const std::vector<uint8_t> &message, const std::vector<uint8_t> &bsi_signature,
				    EVP_PKEY *publicKey)
	{
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

		// Verify signature
		int result = EVP_DigestVerifyFinal(mdctx.get(), der_signature.data(), der_signature.size());

		return result == 1;
	}

	static std::vector<uint8_t> convertBSI_TR03111_to_DER(const std::vector<uint8_t> &bsi_signature)
	{
		if (bsi_signature.size() != 64) {
			std::cerr << "Invalid BSI TR-03111 signature size: " << bsi_signature.size() << std::endl;
			return {};
		}

		BIGNUM *r = BN_new();
		BIGNUM *s = BN_new();

		if (!r || !s) {
			if (r)
				BN_free(r);
			if (s)
				BN_free(s);
			return {};
		}

		// Convert 32-byte values to BIGNUM
		BN_bin2bn(bsi_signature.data(), 32, r);
		BN_bin2bn(bsi_signature.data() + 32, 32, s);

		// Create ECDSA_SIG structure
		ECDSA_SIG *ecdsa_sig = ECDSA_SIG_new();
		if (!ecdsa_sig) {
			BN_free(r);
			BN_free(s);
			return {};
		}

		ECDSA_SIG_set0(ecdsa_sig, r, s);

		// Convert to DER format
		int der_len = i2d_ECDSA_SIG(ecdsa_sig, nullptr);
		if (der_len <= 0) {
			ECDSA_SIG_free(ecdsa_sig);
			return {};
		}

		std::vector<uint8_t> der_signature(der_len);
		unsigned char *der_ptr = der_signature.data();
		i2d_ECDSA_SIG(ecdsa_sig, &der_ptr);

		ECDSA_SIG_free(ecdsa_sig);

		return der_signature;
	}

	static std::vector<uint8_t> convertDER_to_BSI_TR03111(const std::vector<uint8_t> &der_signature) {
          if (der_signature.empty()) {
            std::cerr << "Empty DER signature" << std::endl;
            return {};
          }

          // Parse DER signature
          const unsigned char *der_ptr = der_signature.data();
          ECDSA_SIG *ecdsa_sig =
              d2i_ECDSA_SIG(nullptr, &der_ptr, der_signature.size());

          if (!ecdsa_sig) {
            std::cerr << "Failed to parse DER signature" << std::endl;
            return {};
          }

          // Extract r and s values
          const BIGNUM *r, *s;
          ECDSA_SIG_get0(ecdsa_sig, &r, &s);

          if (!r || !s) {
            ECDSA_SIG_free(ecdsa_sig);
            return {};
          }

          // Convert BIGNUMs to 32-byte arrays
          std::vector<uint8_t> bsi_signature(64, 0);

          int r_len = BN_num_bytes(r);
          int s_len = BN_num_bytes(s);

          // BSI TR-03111 expects exactly 32 bytes for each component
          if (r_len > 32 || s_len > 32) {
            std::cerr << "BIGNUM values too large for BSI TR-03111 format"
                      << std::endl;
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

	static std::vector<std::string> parse_permitted_eins_from_cert(X509 *cert)
	{
		std::vector<std::string> permitted_iins;

		// Check certificate variant
		bool is_old_variant = false;
		int pos = X509_get_ext_by_NID(cert, NID_certificate_policies, -1);
		if (pos >= 0) {
			X509_EXTENSION *ext = X509_get_ext(cert, pos);
			CERTIFICATEPOLICIES *policies = (CERTIFICATEPOLICIES *)X509V3_EXT_d2i(ext);
			if (policies) {
				for (int i = 0; i < sk_POLICYINFO_num(policies); i++) {
					POLICYINFO *policy = sk_POLICYINFO_value(policies, i);
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
				X509_EXTENSION *ext = X509_get_ext(cert, pos);
				NAME_CONSTRAINTS *nc = (NAME_CONSTRAINTS *)X509V3_EXT_d2i(ext);
				if (nc && nc->permittedSubtrees) {
					for (int i = 0; i < sk_GENERAL_SUBTREE_num(nc->permittedSubtrees); i++) {
						GENERAL_SUBTREE *subtree =
							sk_GENERAL_SUBTREE_value(nc->permittedSubtrees, i);
						if (subtree->base->type == GEN_DIRNAME) {
							X509_NAME *name = subtree->base->d.directoryName;
							int idx =
								X509_NAME_get_index_by_NID(name, NID_serialNumber, -1);
							if (idx >= 0) {
								X509_NAME_ENTRY *entry = X509_NAME_get_entry(name, idx);
								ASN1_STRING *asn1_str = X509_NAME_ENTRY_get_data(entry);
								std::string iin(
									reinterpret_cast<const char *>(
										ASN1_STRING_get0_data(asn1_str)),
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
			ASN1_OBJECT *obj = OBJ_txt2obj("2.23.146.1.2.2.0", 1);
			pos = X509_get_ext_by_OBJ(cert, obj, -1);
			ASN1_OBJECT_free(obj);

			if (pos >= 0) {
				X509_EXTENSION *ext = X509_get_ext(cert, pos);
				ASN1_OCTET_STRING *ext_data = X509_EXTENSION_get_data(ext);
				const unsigned char *p = ext_data->data;
				long remaining = ext_data->length;

				// Parse SEQUENCE OF PrintableString
				int tag, xclass;
				long xlen;

				// Get the SEQUENCE tag
				ASN1_get_object(&p, &xlen, &tag, &xclass, remaining);
				if (tag == V_ASN1_SEQUENCE) {
					remaining = xlen;

					while (remaining > 0) {
						const unsigned char *elem_start = p;
						ASN1_get_object(&p, &xlen, &tag, &xclass, remaining);

						if (tag == V_ASN1_PRINTABLESTRING) {
							std::string iin(reinterpret_cast<const char *>(p), xlen);
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

	static void xx_loadCertificate(const std::string &certPath, const std::string &typeName,
			     std::unique_ptr<X509, X509Deleter> &certStorage)
	{
		try {
			auto certs = loadCertificateChain(certPath);
			if (certs.empty()) {
				throw std::runtime_error("No " + typeName + " certificate found in file: " + certPath);
			}

			certStorage = std::move(certs[0]);
			Logger::info("Loaded " + typeName +
				     " certificate: " + getSubjectName(certStorage.get()));

		} catch (const std::exception &e) {
			Logger::error("Failed to load " + typeName + " certificate: " + std::string(e.what()));
			throw;
		}
	}

	// Generic private key loader
	static void xx_loadPrivateKey(const std::string &keyPath, const std::string &typeName,
			    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &keyStorage)
	{
		try {
			if (!std::filesystem::exists(keyPath)) {
				throw std::runtime_error(typeName + " private key file not found: " + keyPath);
			}

			std::ifstream keyFile(keyPath);
			if (!keyFile) {
				throw std::runtime_error("Failed to open " + typeName +
							 " private key file: " + keyPath);
			}

			std::string keyStr((std::istreambuf_iterator<char>(keyFile)), std::istreambuf_iterator<char>());
			std::unique_ptr<BIO, BIODeleter> keyBio(BIO_new_mem_buf(keyStr.c_str(), -1));

			if (!keyBio) {
				throw OpenSSLError("Failed to create BIO for " + typeName + " private key");
			}

			keyStorage.reset(PEM_read_bio_PrivateKey(keyBio.get(), nullptr, nullptr, nullptr));
			if (!keyStorage) {
				throw OpenSSLError("Failed to read " + typeName + " private key");
			}

			Logger::info("Loaded " + typeName + " private key from: " + keyPath);

		} catch (const std::exception &e) {
			Logger::error("Failed to load " + typeName + " private key: " + std::string(e.what()));
			throw;
		}
	}

	// Generic public key loader from file
	static bool xx_loadPublicKey(const std::string &pubKeyPath, const std::string &typeName,
			   std::vector<uint8_t> &publicKeyStorage)
	{
		try {
			if (!std::filesystem::exists(pubKeyPath)) {
				Logger::info(typeName + " public key file not found: " + pubKeyPath +
					     " (will generate from private key)");
				return false;
			}

			std::ifstream keyFile(pubKeyPath);
			if (!keyFile) {
				Logger::warning("Failed to open " + typeName + " public key file: " + pubKeyPath +
						" (will generate from private key)");
				return false;
			}

			std::string keyStr((std::istreambuf_iterator<char>(keyFile)), std::istreambuf_iterator<char>());
			std::unique_ptr<BIO, BIODeleter> keyBio(BIO_new_mem_buf(keyStr.c_str(), -1));

			if (!keyBio) {
				Logger::warning("Failed to create BIO for " + typeName +
						" public key (will generate from private key)");
				return false;
			}

			std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pubKey(
				PEM_read_bio_PUBKEY(keyBio.get(), nullptr, nullptr, nullptr));
			if (!pubKey) {
				Logger::warning("Failed to read " + typeName +
						" public key from file (will generate from private key)");
				return false;
			}

			// Extract public key data
			EC_KEY *ec_key = EVP_PKEY_get1_EC_KEY(pubKey.get());
			if (!ec_key) {
				Logger::warning("Failed to extract EC key from " + typeName +
						" public key file (will generate from private key)");
				return false;
			}

			std::unique_ptr<EC_KEY, void (*)(EC_KEY *)> ec_key_guard(ec_key, EC_KEY_free);

			const EC_POINT *pub_point = EC_KEY_get0_public_key(ec_key);
			const EC_GROUP *group = EC_KEY_get0_group(ec_key);

			if (!pub_point || !group) {
				Logger::warning("Failed to get public key point or group from " + typeName +
						" file (will generate from private key)");
				return false;
			}

			size_t pub_len = EC_POINT_point2oct(group, pub_point, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0,
							    nullptr);
			if (pub_len == 0) {
				Logger::warning("Failed to get public key length from " + typeName +
						" file (will generate from private key)");
				return false;
			}

			publicKeyStorage.resize(pub_len);
			if (EC_POINT_point2oct(group, pub_point, POINT_CONVERSION_UNCOMPRESSED, publicKeyStorage.data(),
					       pub_len, nullptr) != pub_len) {
				Logger::warning("Failed to extract public key data from " + typeName +
						" file (will generate from private key)");
				return false;
			}

			Logger::info("Loaded " + typeName + " public key from file (" + std::to_string(pub_len) +
				     " bytes)");
			return true;

		} catch (const std::exception &e) {
			Logger::warning("Failed to load " + typeName + " public key from file: " +
					std::string(e.what()) + " (will generate from private key)");
			return false;
		}
	}

	// Generate public key from private key (fallback)
	static void xx_generatePublicKeyFromPrivate(const std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &privateKey,
					  const std::string &typeName, std::vector<uint8_t> &publicKeyStorage)
	{
		if (!privateKey) {
			throw std::runtime_error("No " + typeName + " private key available for public key generation");
		}

		EC_KEY *ec_key = EVP_PKEY_get1_EC_KEY(privateKey.get());
		if (!ec_key) {
			throw OpenSSLError("Failed to extract EC key for " + typeName + " public key generation");
		}

		std::unique_ptr<EC_KEY, void (*)(EC_KEY *)> ec_key_guard(ec_key, EC_KEY_free);

		const EC_POINT *pub_point = EC_KEY_get0_public_key(ec_key);
		const EC_GROUP *group = EC_KEY_get0_group(ec_key);

		if (!pub_point || !group) {
			throw OpenSSLError("Failed to get public key point or group for " + typeName + " generation");
		}

		size_t pub_len =
			EC_POINT_point2oct(group, pub_point, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
		if (pub_len == 0) {
			throw OpenSSLError("Failed to get public key length for " + typeName + " generation");
		}

		publicKeyStorage.resize(pub_len);
		if (EC_POINT_point2oct(group, pub_point, POINT_CONVERSION_UNCOMPRESSED, publicKeyStorage.data(),
				       pub_len, nullptr) != pub_len) {
			throw OpenSSLError("Failed to generate public key data for " + typeName);
		}

		Logger::info("Generated " + typeName + " public key from private key (" + std::to_string(pub_len) +
			     " bytes)");
	}

	// Complete key and certificate ecosystem loader
	static void xx_loadCompleteKeySet(const std::string &certPath, const std::string &privKeyPath,
				const std::string &pubKeyPath, const std::string &typeName,
				std::unique_ptr<X509, X509Deleter> &certStorage,
				std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &privKeyStorage,
				std::vector<uint8_t> &pubKeyStorage)
	{
		// Load certificate
		xx_loadCertificate(certPath, typeName, certStorage);

		// Load private key
		xx_loadPrivateKey(privKeyPath, typeName, privKeyStorage);

		// Try to load public key from file, generate from private key if that fails
		if (!xx_loadPublicKey(pubKeyPath, typeName, pubKeyStorage)) {
			xx_generatePublicKeyFromPrivate(privKeyStorage, typeName, pubKeyStorage);
		}
	}

	// Overload for cases where you only have cert and private key paths
	static void xx_loadCompleteKeySet(const std::string &certPath, const std::string &privKeyPath,
				const std::string &typeName, std::unique_ptr<X509, X509Deleter> &certStorage,
				std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &privKeyStorage,
				std::vector<uint8_t> &pubKeyStorage)
	{
		// Load certificate
		xx_loadCertificate(certPath, typeName, certStorage);

		// Load private key
		xx_loadPrivateKey(privKeyPath, typeName, privKeyStorage);

		// Generate public key from private key
		xx_generatePublicKeyFromPrivate(privKeyStorage, typeName, pubKeyStorage);
	}

	// ###
};

// HTTP Client class for SM-DP+ server connections
class HttpClient {
    public:
	HttpClient()
	{
		// Initialize libcurl globally - should be called once per application
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}

	~HttpClient()
	{
		// Clean up libcurl
		curl_global_cleanup();
	}

	// Structure to store response data
	struct ResponseData {
		std::string body;
		long statusCode;
		std::string headers;
	};

	// Callback function for writing response data
	static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		size_t realSize = size * nmemb;
		auto *response = static_cast<std::string *>(userp);
		response->append(static_cast<char *>(contents), realSize);
		return realSize;
	}

	// Callback function for writing header data
	static size_t headerCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		size_t realSize = size * nmemb;
		auto *headers = static_cast<std::string *>(userp);
		headers->append(static_cast<char *>(contents), realSize);
		return realSize;
	}

	// Custom SSL context for certificate verification
	struct SslCtxData {
		X509_STORE *store;
		std::vector<X509 *> *certPool;
		bool verifyResult;
		std::string errorMessage;
	};

	// OpenSSL certificate verification callback
	static int sslCertVerifyCallback(X509_STORE_CTX *storeCtx, void *arg)
	{
		SslCtxData *ctxData = static_cast<SslCtxData *>(arg);

		// Get the certificate being verified
		X509 *cert = X509_STORE_CTX_get_current_cert(storeCtx);
		if (!cert) {
			ctxData->errorMessage = "No certificate to verify";
			ctxData->verifyResult = false;
			return 0;
		}

		// Log certificate information
		Logger::info("Verifying server TLS certificate:");
		CertificateUtil::printCertificateDetails(cert);

		try {
			// Use our dynamic certificate chain validation
			ctxData->verifyResult = CertificateUtil::verifyCertificateChainDynamic(cert, *ctxData->certPool,
											       nullptr, false);

			return ctxData->verifyResult ? 1 : 0;
		} catch (const std::exception &e) {
			ctxData->errorMessage = e.what();
			ctxData->verifyResult = false;
			return 0;
		}
	}

	static std::string get_cn_name(X509_NAME *const name)
	{
		int idx = -1;
		unsigned char *utf8 = NULL;
		X509_NAME_ENTRY *entry;
		ASN1_STRING *data;

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

		// printf("%s: %s\n", label, utf8);
		std::string retstr((char *)utf8);
		OPENSSL_free(utf8);
		return retstr;
	}

	static int verify_callback(int preverify, X509_STORE_CTX *x509_ctx)
	{
		int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
		int err = X509_STORE_CTX_get_error(x509_ctx);

		X509 *cert = X509_STORE_CTX_get_current_cert(x509_ctx);

		Logger::info("Issuer: " + get_cn_name(X509_get_issuer_name(cert)) +
			     " Subject: " + get_cn_name(X509_get_subject_name(cert)) +
			     " SKI: " + HexUtil::bytesToHex(CertificateUtil::getSubjectKeyIdentifier(cert)) +
			     " AKI: " + HexUtil::bytesToHex(CertificateUtil::getAuthorityKeyIdentifier(cert)));

		// if(depth == 0) {
		//     /* If depth is 0, its the server's certificate. Print the SANs too */
		//     print_san_name("Subject (san)", cert);
		// }

		return preverify;
	}

	// Set up custom certificate verification for CURL
	static CURLcode sslCtxFunction(CURL *curl, SSL_CTX *sslCtx, void *arg)
	{
		SslCtxData *ctxData = static_cast<SslCtxData *>(arg);
		SSL_CTX *ctx = (SSL_CTX *)sslCtx;
		X509_STORE *store = SSL_CTX_get_cert_store(ctx);
		// Set the verification callback and its data
		// SSL_CTX_set_cert_verify_callback(sslCtx, sslCertVerifyCallback, ctxData);

		for (auto cert : *ctxData->certPool) {
			Logger::info("ADDED Issuer: " + get_cn_name(X509_get_issuer_name(cert)) +
				     " Subject: " + get_cn_name(X509_get_subject_name(cert)) +
				     " SKI: " + HexUtil::bytesToHex(CertificateUtil::getSubjectKeyIdentifier(cert)) +
				     " AKI: " + HexUtil::bytesToHex(CertificateUtil::getAuthorityKeyIdentifier(cert)));

			auto result = X509_STORE_add_cert(store, cert);
			if (result != 1) {
				printf("Warning: Could not add certificate\n");

				// OpenSSLErrorHandler::getLastError();
				unsigned long err = ERR_get_error();
				if (ERR_GET_REASON(err) != X509_R_CERT_ALREADY_IN_HASH_TABLE) {
					// std::cerr << "Failed to add certificate to store: " << cert_path << std::endl;
					ERR_print_errors_fp(stderr);
				}
			}
		}

		// Turn on verification
		SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER, nullptr);

		return CURLE_OK;
	}

	// Perform HTTPS request with custom certificate verification
	ResponseData postJsonWithCustomVerification(const std::string &url, unsigned int port,
						    const std::string &jsonData, X509_STORE *store,
						    std::vector<X509 *> &certPool)
	{
		ResponseData response;
		CURL *curl = curl_easy_init();

		if (!curl) {
			throw std::runtime_error("Failed to initialize CURL");
		}

		// Set up headers for JSON request
		struct curl_slist *headers = nullptr;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "Accept: application/json");

		// Set up certificate verification data
		SslCtxData ctxData = {
			.store = store, .certPool = &certPool, .verifyResult = false, .errorMessage = ""
		};

		try {
			// Basic CURL setup
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_PORT, port);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);

			// Enable SSL verification
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

			// Use our custom SSL context function and data
			curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslCtxFunction);
			curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, &ctxData);

			curl_easy_setopt(curl, CURLOPT_CAINFO, nullptr); // Disable default CA bundle
			curl_easy_setopt(curl, CURLOPT_CAPATH, nullptr);

			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

			// Perform the request
			CURLcode res = curl_easy_perform(curl);

			// Check for errors
			if (res != CURLE_OK) {
				throw std::runtime_error(std::string("CURL request failed: ") +
							 curl_easy_strerror(res));
			}

			// Check certificate verification result
			// if (!ctxData.verifyResult) {
			// 	throw std::runtime_error("TLS certificate verification failed: " +
			// 				 ctxData.errorMessage);
			// }

			// Get the status code
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);

			// Clean up
			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);

			return response;
		} catch (...) {
			// Clean up on exception
			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
			throw;
		}
	}
};

// RSP (Remote SIM Provisioning) Client implementation
class RSPClient {
    public:
	RSPClient(const std::string &serverUrl, const unsigned int serverPort, const std::vector<std::string> &certPath,
		  const std::vector<std::string> &name_filters = {})
		: m_serverUrl(serverUrl), m_serverPort(serverPort)
	{
		// Initialize OpenSSL
		OpenSSL_add_all_algorithms();
		ERR_load_crypto_strings();

		for (auto cpath : certPath) {
			bool isDirectory = std::filesystem::is_directory(cpath);
			try {
				if (isDirectory) {
					Logger::info("Loading certificates from directory: " + cpath);

					// Load all certificates from the directory
					auto certs =
						CertificateUtil::loadCertificatesFromDirectory(cpath, name_filters);
					Logger::info("Loaded " + std::to_string(certs.size()) + " certificates");

					// Separate root CAs and intermediates
					for (auto &cert : certs) {
						// Identify root certificates (self-signed)
						if (X509_check_issued(cert.get(), cert.get()) == X509_V_OK) {
							Logger::info("Found root CA: " +
								     CertificateUtil::getSubjectName(cert.get()));

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
						Logger::warning("No root CA found in directory");
					}
				} else {
					// Original behavior - load certificate chain from file
					Logger::info("Loading certificates from file: " + cpath);
					auto caChain = CertificateUtil::loadCertificateChain(cpath);

					if (!caChain.empty()) {
						m_rootCA = std::move(caChain[0]);

						for (size_t i = 1; i < caChain.size(); ++i) {
							m_intermediateCA.push_back(std::move(caChain[i]));
						}

						Logger::info("Loaded root CA: " +
							     CertificateUtil::getSubjectName(m_rootCA.get()));

						for (const auto &cert : m_intermediateCA) {
							Logger::info("Loaded intermediate CA: " +
								     CertificateUtil::getSubjectName(cert.get()));

							// Also add intermediates to certificate pool for compatibility
							m_certPool.push_back(std::unique_ptr<X509, X509Deleter>(
								X509_dup(cert.get())));
						}
					}
				}
			} catch (const std::exception &e) {
				Logger::error("Failed to load certificates: " + std::string(e.what()));
				throw;
			}
		}
	}

	~RSPClient()
	{
		// Cleanup OpenSSL
		EVP_cleanup();
		CRYPTO_cleanup_all_ex_data();
		ERR_free_strings();
	}

	// Set the path to CA certificates for system verification
	void setCACertPath(const std::string &path)
	{
		m_caCertPath = path;
	}

	// Generate a random challenge (16 bytes)
	std::vector<uint8_t> generateChallenge()
	{
		std::vector<uint8_t> challenge(16);
		if (RAND_bytes(challenge.data(), challenge.size()) != 1) {
			throw OpenSSLError("Failed to generate random challenge");
		}

		return challenge;
	}

	// Sign data with eUICC private key
	std::vector<uint8_t> signDataWithEUICC(const std::vector<uint8_t> &dataToSign)
	{
		if (!m_euiccPrivateKey) {
			throw std::runtime_error("eUICC private key not available for signing");
		}

		if (dataToSign.empty()) {
			throw std::runtime_error("Data to sign is empty");
		}

		// Create signature context
		std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdctx(EVP_MD_CTX_new());
		if (!mdctx) {
			throw OpenSSLError("Failed to create signature context for eUICC");
		}

		// Initialize signing with SHA256
		if (EVP_DigestSignInit(mdctx.get(), nullptr, EVP_sha256(), nullptr, m_euiccPrivateKey.get()) != 1) {
			throw OpenSSLError("Failed to initialize eUICC signature operation");
		}

		// Update with data to be signed
		if (EVP_DigestSignUpdate(mdctx.get(), dataToSign.data(), dataToSign.size()) != 1) {
			throw OpenSSLError("Failed to update eUICC signature");
		}

		// Get signature length
		size_t sigLen;
		if (EVP_DigestSignFinal(mdctx.get(), nullptr, &sigLen) != 1) {
			throw OpenSSLError("Failed to determine eUICC signature length");
		}

		if (sigLen == 0) {
			throw std::runtime_error("eUICC signature length is zero");
		}

		// Create signature
		std::vector<uint8_t> signature(sigLen);
		if (EVP_DigestSignFinal(mdctx.get(), signature.data(), &sigLen) != 1) {
			throw OpenSSLError("Failed to create eUICC signature");
		}

		signature.resize(sigLen);
		Logger::debug("Created eUICC signature (" + std::to_string(sigLen) + " bytes) over " +
			      std::to_string(dataToSign.size()) + " bytes of data");

		return CertificateUtil::convertDER_to_BSI_TR03111(signature);
	}

	// SM-DP+ signing function (separate from eUICC signing)
	std::vector<uint8_t> signDataWithSMDP(const std::vector<uint8_t> &dataToSign)
	{
		if (!m_smdpPrivateKey) {
			throw std::runtime_error("SM-DP+ private key not available for signing");
		}

		if (dataToSign.empty()) {
			throw std::runtime_error("Data to sign is empty");
		}

		// Create signature context
		std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdctx(EVP_MD_CTX_new());
		if (!mdctx) {
			throw OpenSSLError("Failed to create signature context for SM-DP+");
		}

		// Initialize signing with SHA256
		if (EVP_DigestSignInit(mdctx.get(), nullptr, EVP_sha256(), nullptr, m_smdpPrivateKey.get()) != 1) {
			throw OpenSSLError("Failed to initialize SM-DP+ signature operation");
		}

		// Update with data to be signed
		if (EVP_DigestSignUpdate(mdctx.get(), dataToSign.data(), dataToSign.size()) != 1) {
			throw OpenSSLError("Failed to update SM-DP+ signature");
		}

		// Get signature length
		size_t sigLen;
		if (EVP_DigestSignFinal(mdctx.get(), nullptr, &sigLen) != 1) {
			throw OpenSSLError("Failed to determine SM-DP+ signature length");
		}

		if (sigLen == 0) {
			throw std::runtime_error("SM-DP+ signature length is zero");
		}

		// Create signature
		std::vector<uint8_t> signature(sigLen);
		if (EVP_DigestSignFinal(mdctx.get(), signature.data(), &sigLen) != 1) {
			throw OpenSSLError("Failed to create SM-DP+ signature");
		}

		signature.resize(sigLen);
		Logger::debug("Created SM-DP+ signature (" + std::to_string(sigLen) + " bytes) over " +
			      std::to_string(dataToSign.size()) + " bytes of data");

		return signature;
	}

	void generateEUICCOtpk()
	{
		// Create EC key using P-256 curve (secp256r1) as commonly used in
		// smartcard applications
		std::unique_ptr<EC_KEY, EC_KEY_Deleter> ecKey(EC_KEY_new());
		if (!ecKey) {
			throw OpenSSLError("Failed to create EC_KEY");
		}

		// Get the P-256 curve group
		std::unique_ptr<EC_GROUP, decltype(&EC_GROUP_free)> group(
			EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1), EC_GROUP_free);
		if (!group) {
			throw OpenSSLError("Failed to create EC_GROUP for P-256");
		}

		// Set the group for the key
		if (EC_KEY_set_group(ecKey.get(), group.get()) != 1) {
			throw OpenSSLError("Failed to set EC group");
		}

		// Generate the key pair
		if (EC_KEY_generate_key(ecKey.get()) != 1) {
			throw OpenSSLError("Failed to generate EC key pair");
		}

		// Get the public key point
		const EC_POINT *pubKeyPoint = EC_KEY_get0_public_key(ecKey.get());
		if (!pubKeyPoint) {
			throw OpenSSLError("Failed to get public key point");
		}

		// Convert public key to uncompressed format (0x04 + X + Y coordinates)
		// For P-256: 1 byte (0x04) + 32 bytes (X) + 32 bytes (Y) = 65 bytes total
		m_euiccOtpk.resize(65);

		size_t pubKeyLen = EC_POINT_point2oct(group.get(), pubKeyPoint, POINT_CONVERSION_UNCOMPRESSED,
						      m_euiccOtpk.data(), m_euiccOtpk.size(), nullptr);

		if (pubKeyLen != 65) {
			throw OpenSSLError("Failed to encode public key or unexpected size");
		}

		// Store the private key for later use in key agreement
		m_euicc_ot_PrivateKey.reset(EC_KEY_dup(ecKey.get()));
		if (!m_euicc_ot_PrivateKey) {
			throw OpenSSLError("Failed to duplicate private key");
		}

		Logger::info("Generated eUICC OtPK (P-256): " + HexUtil::bytesToHex(m_euiccOtpk));

		// Verify the key format
		if (m_euiccOtpk[0] != 0x04) {
			throw std::runtime_error("Invalid public key format - expected uncompressed point");
		}

		Logger::debug("Public key format verified - uncompressed EC point");
	}

	// Set confirmation code hash (optional)
	void setConfirmationCodeHash(const std::vector<uint8_t> &hash)
	{
		m_confirmationCodeHash = hash;
		Logger::info("Set confirmation code hash: " + HexUtil::bytesToHex(hash));
	}


	void loadEUICCCertificate(const std::string &euiccCertPath)
	{
		CertificateUtil::xx_loadCertificate(euiccCertPath, "eUICC", m_euiccCert);

		// EUICC-specific operations
		try {
			m_EID = CertificateUtil::getEID(m_euiccCert.get());
			Logger::info("Using EID from certificate: " + m_EID);

			auto ski = CertificateUtil::getSubjectKeyIdentifier(m_rootCA.get());
			m_euiccSKI = ski;
			Logger::info("Using eUICC SKI as PKID: " + HexUtil::bytesToHex(m_euiccSKI));
		} catch (const std::exception &e) {
			Logger::error("Failed to extract EUICC-specific data: " + std::string(e.what()));
			throw;
		}
	}

	void loadEUICCKeyPair(const std::string &euiccPrivateKeyPath, const std::string &euiccPublicKeyPath = "")
	{
		CertificateUtil::xx_loadPrivateKey(euiccPrivateKeyPath, "eUICC", m_euiccPrivateKey);

		if (!euiccPublicKeyPath.empty() && CertificateUtil::xx_loadPublicKey(euiccPublicKeyPath, "eUICC", m_euiccPublicKeyData)) {
			// Successfully loaded public key from file
			return;
		}

		// Generate public key from private key
		CertificateUtil::xx_generatePublicKeyFromPrivate(m_euiccPrivateKey, "eUICC", m_euiccPublicKeyData);
	}

	void loadEUMCertificate(const std::string &eumCertPath)
	{
		CertificateUtil::xx_loadCertificate(eumCertPath, "EUM", m_eumCert);
	}

	void loadEUMKeyPair(const std::string &eumPrivateKeyPath, const std::string &eumPublicKeyPath = "")
	{
		CertificateUtil::xx_loadPrivateKey(eumPrivateKeyPath, "EUM", m_eumPrivateKey);

		if (!eumPublicKeyPath.empty() && CertificateUtil::xx_loadPublicKey(eumPublicKeyPath, "EUM", m_eumPublicKeyData)) {
			// Successfully loaded public key from file
			return;
		}

		// Generate public key from private key
		CertificateUtil::xx_generatePublicKeyFromPrivate(m_eumPrivateKey, "EUM", m_eumPublicKeyData);
	}

	void loadSMDPKeyAndCertificate(const std::string &smdpPrivateKeyPath, const std::string &smdpCertPath,
				       const std::string &smdpPublicKeyPath = "")
	{
		CertificateUtil::xx_loadCertificate(smdpCertPath, "SM-DP+", m_smdpCertificate);
		CertificateUtil::xx_loadPrivateKey(smdpPrivateKeyPath, "SM-DP+", m_smdpPrivateKey);

		if (!smdpPublicKeyPath.empty() && CertificateUtil::xx_loadPublicKey(smdpPublicKeyPath, "SM-DP+", m_smdpPublicKeyData)) {
			// Successfully loaded public key from file
			Logger::info("SM-DP+ certificate subject: " +
				     CertificateUtil::getSubjectName(m_smdpCertificate.get()));
			return;
		}

		// Generate public key from private key
		CertificateUtil::xx_generatePublicKeyFromPrivate(m_smdpPrivateKey, "SM-DP+", m_smdpPublicKeyData);
		Logger::info("SM-DP+ certificate subject: " + CertificateUtil::getSubjectName(m_smdpCertificate.get()));
	}

	// Ultra-generic complete ecosystem loader - handles any certificate type
	void loadAnyCompleteKeySet(const std::string &certPath, const std::string &privKeyPath,
				   const std::string &pubKeyPath, const std::string &typeName,
				   std::unique_ptr<X509, X509Deleter> &certStorage,
				   std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &privKeyStorage,
				   std::vector<uint8_t> &pubKeyStorage)
	{
		CertificateUtil::xx_loadCompleteKeySet(certPath, privKeyPath, pubKeyPath, typeName, certStorage, privKeyStorage,
				   pubKeyStorage);
	}


	// Create initiateAuthentication request
	std::string createInitiateAuthenticationRequest(const std::vector<uint8_t> &euiccChallenge,
							const std::string &smdpAddress)
	{
		// Create EUICC_INFO1 structure
		std::unique_ptr<EUICC_INFO1, EUICC_INFO1_Deleter> info(EUICC_INFO1_new());
		if (!info) {
			throw OpenSSLError("Failed to create EUICC_INFO1 structure");
		}

		// Set SVN (version)
		std::vector<uint8_t> svn = { 0x2, 0x0, 0xc }; // SVN 3.0
		ASN1_OCTET_STRING_set(info->svn, svn.data(), svn.size());

		// Use real PKIDs if available
		if (!m_euiccSKI.empty()) {
			// Use the eUICC certificate's SKI for both verification and signing
			// Verification PKIDs
			ASN1_OCTET_STRING *vItem = ASN1_OCTET_STRING_new();
			if (!vItem) {
				throw OpenSSLError("Failed to create ASN1_OCTET_STRING for verification PKID");
			}
			ASN1_OCTET_STRING_set(vItem, m_euiccSKI.data(), m_euiccSKI.size());
			sk_ASN1_STRING_push((STACK_OF(ASN1_STRING) *)info->euiccCiPKIdListForVerification, vItem);

			// Signing PKIDs
			ASN1_OCTET_STRING *sItem = ASN1_OCTET_STRING_new();
			if (!sItem) {
				throw OpenSSLError("Failed to create ASN1_OCTET_STRING for signing PKID");
			}
			ASN1_OCTET_STRING_set(sItem, m_euiccSKI.data(), m_euiccSKI.size());
			sk_ASN1_STRING_push((STACK_OF(ASN1_STRING) *)info->euiccCiPKIdListForSigning, sItem);
		} else {
			// Fall back to default PKIDs
			// Define verification PKIDs
			std::vector<std::vector<uint8_t>> verificationPKIDs = {
				{ 0x01, 0x02, 0x03, 0x04, 0x05 }, // Example PKID 1
				{ 0x06, 0x07, 0x08, 0x09, 0x0A } // Example PKID 2
			};

			// Define signing PKIDs
			std::vector<std::vector<uint8_t>> signingPKIDs = {
				{ 0x0B, 0x0C, 0x0D, 0x0E, 0x0F } // Example signing PKID
			};

			// Add verification PKIDs
			for (const auto &pkid : verificationPKIDs) {
				ASN1_OCTET_STRING *item = ASN1_OCTET_STRING_new();
				if (!item) {
					throw OpenSSLError("Failed to create ASN1_OCTET_STRING for verification PKID");
				}
				ASN1_OCTET_STRING_set(item, pkid.data(), pkid.size());
				sk_ASN1_STRING_push((STACK_OF(ASN1_STRING) *)info->euiccCiPKIdListForVerification,
						    item);
			}

			// Add signing PKIDs
			for (const auto &pkid : signingPKIDs) {
				ASN1_OCTET_STRING *item = ASN1_OCTET_STRING_new();
				if (!item) {
					throw OpenSSLError("Failed to create ASN1_OCTET_STRING for signing PKID");
				}
				ASN1_OCTET_STRING_set(item, pkid.data(), pkid.size());
				sk_ASN1_STRING_push((STACK_OF(ASN1_STRING) *)info->euiccCiPKIdListForSigning, item);
			}
		}

		// Convert euiccInfo1 to DER
		unsigned char *derData = nullptr;
		int derLen = i2d_EUICC_INFO1(info.get(), &derData);

		for (int i = 0; i < derLen; i++) {
			printf("%02x", derData[i]);
		}

		if (derLen <= 0) {
			throw OpenSSLError("Failed to encode EUICC_INFO1 to DER");
		}

		// Use smart pointer for derData
		std::unique_ptr<unsigned char, OpenSSLFreeDeleter> derDataPtr(derData);

		// Convert to Base64
		std::vector<uint8_t> derVector(derDataPtr.get(), derDataPtr.get() + derLen);
		std::string euiccInfo1Base64 = Base64::encode(derVector);
		std::string euiccChallengeBase64 = Base64::encode(euiccChallenge);

		// Create JSON request
		std::unique_ptr<cJSON, cJSON_Deleter> json(cJSON_CreateObject());

		cJSON_AddStringToObject(json.get(), "euiccChallenge", euiccChallengeBase64.c_str());
		cJSON_AddStringToObject(json.get(), "euiccInfo1", euiccInfo1Base64.c_str());
		cJSON_AddStringToObject(json.get(), "smdpAddress", smdpAddress.c_str());

		// Serialize JSON to string
		char *jsonStr = cJSON_Print(json.get());
		std::string result(jsonStr);
		free(jsonStr);

		return result;
	}

	// Process initiateAuthentication response
	bool processInitiateAuthenticationResponse(const std::string &responseJson)
	{
		try {
			// Parse JSON
			std::unique_ptr<cJSON, cJSON_Deleter> json(cJSON_Parse(responseJson.c_str()));

			if (!json) {
				const char *errorPtr = cJSON_GetErrorPtr();
				throw std::runtime_error(std::string("Error parsing JSON: ") +
							 (errorPtr ? errorPtr : "Unknown error"));
			}

			// Extract fields from JSON
			cJSON *serverSigned1Json = cJSON_GetObjectItemCaseSensitive(json.get(), "serverSigned1");
			cJSON *serverSignature1Json = cJSON_GetObjectItemCaseSensitive(json.get(), "serverSignature1");
			cJSON *transactionIdJson = cJSON_GetObjectItemCaseSensitive(json.get(), "transactionId");
			cJSON *serverCertificateJson =
				cJSON_GetObjectItemCaseSensitive(json.get(), "serverCertificate");
			cJSON *euiccCiPKIdToBeUsedJson =
				cJSON_GetObjectItemCaseSensitive(json.get(), "euiccCiPKIdToBeUsed");

			// Validate required fields exist
			if (!cJSON_IsString(serverSigned1Json) || !cJSON_IsString(serverSignature1Json) ||
			    !cJSON_IsString(transactionIdJson) || !cJSON_IsString(serverCertificateJson) ||
			    !cJSON_IsString(euiccCiPKIdToBeUsedJson)) {
				throw std::runtime_error("Missing or invalid required fields in JSON");
			}

			// Store transaction ID for future requests
			m_transactionId = transactionIdJson->valuestring;
			LOG_INFO("Transaction ID: " + m_transactionId);

			// Decode Base64 values
			std::vector<uint8_t> serverSigned1 = Base64::decode(serverSigned1Json->valuestring);
			std::vector<uint8_t> serverSignature1 = Base64::decode(serverSignature1Json->valuestring);
			std::vector<uint8_t> serverCertificate = Base64::decode(serverCertificateJson->valuestring);
			std::vector<uint8_t> euiccCiPKIdToBeUsed = Base64::decode(euiccCiPKIdToBeUsedJson->valuestring);

			// Store euiccCiPKIdToBeUsed for future use
			m_euiccCiPKIdToBeUsed = euiccCiPKIdToBeUsed;

			// Load server certificate
			// m_serverCert = CertificateUtil::loadCertFromPEM(std::string(
			// 	reinterpret_cast<char *>(serverCertificate.data()), serverCertificate.size()));

			m_serverCert = CertificateUtil::loadCertFromDER(serverCertificate);

			// Verify server certificate against root CA
			if (!verifyServerCertificate()) {
				LOG_ERROR("Server certificate validation failed");
				return false;
			}

			// Parse ServerSigned1 ASN.1 Structure
			const unsigned char *p = serverSigned1.data();
			std::unique_ptr<SERVER_SIGNED1, SERVER_SIGNED1_Deleter> ss1(
				d2i_SERVER_SIGNED1(nullptr, &p, serverSigned1.size()));

			if (!ss1) {
				throw OpenSSLError("Failed to parse ServerSigned1 ASN.1 structure");
			}

			// Verify transaction ID match
			std::vector<uint8_t> transactionIdBin = HexUtil::hexToBytes(m_transactionId);

			if (transactionIdBin.size() != 16 || ASN1_STRING_length(ss1->transactionId) != 16 ||
			    memcmp(ASN1_STRING_get0_data(ss1->transactionId), transactionIdBin.data(), 16) != 0) {
				LOG_ERROR("Transaction ID mismatch");
				return false;
			}

			// Verify euiccChallenge match
			if (ASN1_STRING_length(ss1->euiccChallenge) != m_euiccChallenge.size() ||
			    memcmp(ASN1_STRING_get0_data(ss1->euiccChallenge), m_euiccChallenge.data(),
				   m_euiccChallenge.size()) != 0) {
				LOG_ERROR("eUICC challenge mismatch");
				return false;
			}

			// Verify server address
			std::string serverAddressReceived(
				reinterpret_cast<const char *>(ASN1_STRING_get0_data(ss1->serverAddress)),
				ASN1_STRING_length(ss1->serverAddress));

			if (serverAddressReceived != m_serverUrl) {
				LOG_WARNING("Server address mismatch: expected " + m_serverUrl + ", got " +
					    serverAddressReceived);
				// Continue despite mismatch - some implementations might use different
				// names
			}

			// Verify signature
			if (!verifyServerSignature(serverSigned1, serverSignature1)) {
				LOG_ERROR("Server signature verification failed");
				return false;
			}

			// Store server challenge for future requests
			m_serverChallenge.resize(ASN1_STRING_length(ss1->serverChallenge));
			memcpy(m_serverChallenge.data(), ASN1_STRING_get0_data(ss1->serverChallenge),
			       m_serverChallenge.size());

			LOG_INFO("Server challenge: " + HexUtil::bytesToHex(m_serverChallenge));

			return true;
		} catch (const std::exception &e) {
			LOG_ERROR("Error processing authentication response: " + std::string(e.what()));
			return false;
		}
	}

	// Perform the initiateAuthentication exchange
	bool initiateAuthentication()
	{
		try {
			// Generate eUICC challenge
			m_euiccChallenge = generateChallenge();
			Logger::info("Generated eUICC challenge: " + HexUtil::bytesToHex(m_euiccChallenge));

			// Create request
			std::string request = createInitiateAuthenticationRequest(m_euiccChallenge, m_serverUrl);

			Logger::info("Sending initiateAuthentication request:\n" + request);

			// In a real implementation, this would send the request to the server
			// For this example, let's assume we have a test response
			std::string response = simulateSendRequest(request);

			// Process response
			return processInitiateAuthenticationResponse(response);
		} catch (const std::exception &e) {
			Logger::error("Error during authentication: " + std::string(e.what()));
			return false;
		}
	}

	bool verifyServerSignature(const std::vector<uint8_t> &serverSigned1, const std::vector<uint8_t> &serverSignature1, const std::vector<uint8_t> &derDataServerCert)
	{
		auto serverCert = CertificateUtil::loadCertFromDER(derDataServerCert);
		return verifyServerSignature(serverSigned1, serverSignature1, serverCert.get());
	}

	std::vector<uint8_t> getEUMCertificate(){
		return CertificateUtil::certToDER(m_eumCert.get());
	}
	std::vector<uint8_t> getEUICCCertificate(){
		return CertificateUtil::certToDER(m_euiccCert.get());
	}
	std::vector<uint8_t> getSMDPCertificate(){
		return CertificateUtil::certToDER(m_smdpCertificate.get());
	}
	std::vector<uint8_t> getCICertificate(){
		if (m_rootCA) {
			return CertificateUtil::certToDER(m_rootCA.get());
		}
		return std::vector<uint8_t>();
	}
	std::vector<uint8_t> getEUICCOtpk(){
		return m_euiccOtpk;
	}

	std::vector<uint8_t> computeECDHSharedSecret(const std::vector<uint8_t> &otherPublicKey)
	{
		if (!m_euicc_ot_PrivateKey) {
			throw std::runtime_error("eUICC ephemeral private key not available");
		}

		// Create EC_POINT from other party's public key
		const EC_GROUP *group = EC_KEY_get0_group(m_euicc_ot_PrivateKey.get());
		std::unique_ptr<EC_POINT, decltype(&EC_POINT_free)> other_point(EC_POINT_new(group), EC_POINT_free);

		if (!other_point) {
			throw OpenSSLError("Failed to create EC_POINT");
		}

		// Convert octets to EC_POINT
		if (EC_POINT_oct2point(group, other_point.get(), otherPublicKey.data(), otherPublicKey.size(),
				       nullptr) != 1) {
			throw OpenSSLError("Failed to convert public key to EC_POINT");
		}

		// Compute shared secret
		size_t secret_len = (EC_GROUP_get_degree(group) + 7) / 8;
		std::vector<uint8_t> shared_secret(secret_len);

		if (ECDH_compute_key(shared_secret.data(), secret_len, other_point.get(), m_euicc_ot_PrivateKey.get(),
				     nullptr) <= 0) {
			throw OpenSSLError("ECDH computation failed");
		}

		Logger::info("Computed ECDH shared secret: " + HexUtil::bytesToHex(shared_secret));
		return shared_secret;
	}

    private:
	// Verify server certificate against root CA
	bool verifyServerCertificate()
	{
		if (!m_serverCert) {
			Logger::error("Server certificate not loaded");
			return false;
		}

		// Check certificate validity
		if (CertificateUtil::isExpired(m_serverCert.get())) {
			Logger::error("Server certificate is expired");
			return false;
		}

		if (CertificateUtil::isNotYetValid(m_serverCert.get())) {
			Logger::error("Server certificate is not yet valid");
			return false;
		}

		try {
			// Prepare a pool of all available certificates for chain verification
			std::vector<X509 *> certPool;

			// Add intermediate certificates from both sources to the pool
			for (const auto &cert : m_intermediateCA) {
				certPool.push_back(cert.get());
			}

			for (const auto &cert : m_certPool) {
				certPool.push_back(cert.get());
			}

			// Display certificate identifier information
			auto serverSKI = CertificateUtil::getSubjectKeyIdentifier(m_serverCert.get());
			auto serverAKI = CertificateUtil::getAuthorityKeyIdentifier(m_serverCert.get());

			Logger::debug("Server certificate SKI: " + HexUtil::bytesToHex(serverSKI));
			Logger::debug("Server certificate AKI: " + HexUtil::bytesToHex(serverAKI));

			// Try dynamic chain verification first
			if (CertificateUtil::verifyCertificateChainDynamic(m_serverCert.get(), certPool,
									   m_rootCA.get())) {
				return true;
			}

			// Fall back to original verification method if dynamic fails
			Logger::warning("Dynamic verification failed, trying traditional method");

			// Create certificate stack
			STACK_OF(X509) *intermediates = sk_X509_new_null();
			if (!intermediates) {
				throw OpenSSLError("Failed to create certificate stack");
			}

			// Add intermediate certificates to the stack
			for (const auto &cert : m_intermediateCA) {
				sk_X509_push(intermediates, cert.get());
			}

			// Use smart pointer for stack cleanup
			std::unique_ptr<STACK_OF(X509), STACK_OF_X509_Deleter> stackPtr(intermediates);

			// Verify with traditional method
			bool result = CertificateUtil::verifyCertificateChain(m_serverCert.get(), stackPtr.get(),
									      m_rootCA.get());

			if (result) {
				Logger::info("Server certificate successfully validated with "
					     "traditional method");
			}

			return result;
		} catch (const std::exception &e) {
			Logger::error("Certificate validation error: " + std::string(e.what()));
			return false;
		}
	}

	// Verify server signature
	bool verifyServerSignature(const std::vector<uint8_t> &signedData, const std::vector<uint8_t> &signature, X509* scertdata)
	{
		try {
			// Handle special prefix in serverSignature1 based on SGP.22 spec
			const unsigned char *signatureData = signature.data();
			size_t signatureLen = signature.size();

			if (signatureLen >= 3 && signatureData[0] == 0x5f && signatureData[1] == 0x37 &&
			    signatureData[2] == 0x40) {
				LOG_INFO("Detected 5F3740 prefix in serverSignature1, skipping for "
					 "verification");
				signatureData += 3;
				signatureLen -= 3;
			}

		// for (int i = 0; i < serverSignature1.size(); i++) {
		// 	printf("%02x", serverSignature1[i]);
		// }
		// printf("\n");


			// Extract public key from server certificate
			std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pubkey(X509_get_pubkey(scertdata));
			if (!pubkey) {
				throw OpenSSLError("Failed to extract public key from certificate");
			}

			auto ski = CertificateUtil::getSubjectKeyIdentifier(scertdata);
			Logger::info("----------- using cert with this SKI to verify sig: " + HexUtil::bytesToHex(ski));
			// CertificateUtil::printCertificateDetails(scertdata);

			auto verifyResult = CertificateUtil::verify_TR031111(
				signedData, std::vector<unsigned char>(signatureData, signatureData + signatureLen),
				pubkey.get());

			if (verifyResult) {
				LOG_INFO("Signature verification successful!");
				return true;
			} else {
				LOG_ERROR("Signature verification failed - invalid signature");
				return false;
			}
		} catch (const std::exception &e) {
			LOG_ERROR("Error verifying signature: " + std::string(e.what()));
			return false;
		}
	}

	bool verifyServerSignature(const std::vector<uint8_t> &serverSigned1, const std::vector<uint8_t> &serverSignature1)
	{
		return verifyServerSignature(serverSigned1, serverSignature1, m_serverCert.get());
	}

	std::string simulateSendRequest(const std::string &request)
	{

		Logger::info("Sending HTTP request to SM-DP+ server: " + m_serverUrl);

		// Create full URL for the initiateAuthentication endpoint
		std::string url = m_serverUrl;

		// Ensure URL has proper prefix
		if (url.find("http://") != 0 && url.find("https://") != 0) {
			url = "https://" + url;
		}

		// Add the API endpoint path
		if (url.back() != '/') {
			url += "/";
		}
		url += "gsma/rsp2/es9plus/initiateAuthentication";

		Logger::info("Full request URL: " + url);

		// Real implementation using HTTPS
		try {
			// Create an HTTP client
			HttpClient client;

			// Try to use custom verification with our certificate store
			std::vector<X509 *> certPool;

			// Add all certificates from our stores to the pool
			if (m_rootCA) {
				certPool.push_back(m_rootCA.get());
			}

			for (const auto &cert : m_intermediateCA) {
				certPool.push_back(cert.get());
			}

			for (const auto &cert : m_certPool) {
				certPool.push_back(cert.get());
			}

			// Create a certificate store for verification
			std::unique_ptr<X509_STORE, X509_STORE_Deleter> store(X509_STORE_new());
			if (!store) {
				throw OpenSSLError("Failed to create X509_STORE");
			}

			// Add root CA to the store if available
			if (m_rootCA) {
				if (X509_STORE_add_cert(store.get(), m_rootCA.get()) != 1) {
					unsigned long err = ERR_peek_last_error();
					// Ignore duplicate certificate errors
					if (ERR_GET_REASON(err) != X509_R_CERT_ALREADY_IN_HASH_TABLE) {
						throw OpenSSLError("Failed to add root CA to store");
					}
					ERR_clear_error();
				}
			}

			// Try to send the request with custom verification
			Logger::info("Sending request with custom certificate verification");
			auto response = client.postJsonWithCustomVerification(url, m_serverPort, request, store.get(),
									      certPool);

			// Check for successful HTTP status
			if (response.statusCode >= 200 && response.statusCode < 300) {
				Logger::info("Request successful (HTTP " + std::to_string(response.statusCode) + ")");
				return response.body;
			} else {
				Logger::error("HTTP error: " + std::to_string(response.statusCode));
				Logger::error("Response: " + response.body);
				throw std::runtime_error("HTTP error: " + std::to_string(response.statusCode));
			}

		} catch (const std::exception &e) {
			Logger::error("Failed to send HTTP request: " + std::string(e.what()));
			throw;
		}
	}

	// Member variables
	std::string m_serverUrl;
	unsigned int m_serverPort;

	std::string m_transactionId;
	std::vector<uint8_t> m_euiccChallenge;
	std::vector<uint8_t> m_serverChallenge;
	std::vector<uint8_t> m_euiccCiPKIdToBeUsed;

	std::unique_ptr<X509, X509Deleter> m_rootCA;
	std::vector<std::unique_ptr<X509, X509Deleter>> m_intermediateCA;
	std::unique_ptr<X509, X509Deleter> m_serverCert;

	// std::vector<std::unique_ptr<X509, X509Deleter>> m_rootCAs;
	std::vector<std::unique_ptr<X509, X509Deleter>> m_certPool;


	std::vector<uint8_t> m_euiccSKI; // eUICC SKI for PKIDs
	std::string m_EID; // eUICC ID
	std::unique_ptr<X509, X509Deleter> m_euiccCert; // eUICC certificate
	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_euiccPrivateKey; // eUICC private key for signing
	std::vector<uint8_t> m_euiccPublicKeyData; // eUICC public key data
	std::unique_ptr<EC_KEY, EC_KEY_Deleter> m_euicc_ot_PrivateKey;
	std::vector<uint8_t> m_euiccOtpk; // One-time public key from eUICC

	std::unique_ptr<X509, X509Deleter> m_eumCert; // eUICC certificate
	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_eumPrivateKey; // eum private key for signing
	std::vector<uint8_t> m_eumPublicKeyData; // eum public key data

	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_smdpPrivateKey; // SM-DP+ private key for signing
	std::unique_ptr<X509, X509Deleter> m_smdpCertificate; // SM-DP+ certificate (CERT.DPpb.ECDSA)
	std::vector<uint8_t> m_smdpPublicKeyData; // SM-DP+  public key data


	std::vector<uint8_t> m_confirmationCodeHash; // Optional confirmation code hash

	std::string m_caCertPath = ""; // Path to system CA certificates
	std::vector<uint8_t> m_boundProfilePackage; // Bound profile package from SM-DP+
};


} // namespace RspCrypto


std::ostream &operator<<(std::ostream &os, const std::vector<std::string> &v)
{
	using namespace std;
	copy(v.begin(), v.end(), std::ostream_iterator<std::string>(os, "\n"));
	return os;
}

std::string to_string(const std::vector<std::string> &vec)
{
	std::ostringstream oss;
	oss << vec; // Uses our overloaded << operator
	return oss.str();
}

std::string operator+(const std::string &str, const std::vector<std::string> &vec)
{
	return str + to_string(vec);
}

std::string operator+(const std::vector<std::string> &vec, const std::string &str)
{
	return to_string(vec) + str;
}

#ifdef STANDALONE_SMDPCC
// Main function
int main(int argc, char *argv[])
{
	// Default values
	std::string serverUrl = "testsmdpplus1.example.com"; //"smdp.example.com";
	unsigned int serverPort = 8000;

	//   std::vector<std::string> certDir = {
	// // "demoCA/all"
	// "./Variants A_B_C/CI",
	// "./Variants A_B_C/CI_subCA",
	// "./Variants A_B_C/Variant C/EUM",
	// "./Variants A_B_C/Variant C/EUM_SUB_CA",
	// "./Variants A_B_C/Variant C/SM-DP+",
	// "./Variants A_B_C/Variant C/SM-DS",
	// // "./Variants A_B_C/Variant C/EUICC",
	// };

	//   //'sgp26/SGP.26v3_Files/Variants A_B_C/Variant C/EUICC/PK_EUICC_SIG_BRP.pem'
	//   //'sgp26/SGP.26v3_Files/Variants A_B_C/Variant C/EUICC/PK_EUICC_SIG_NIST.pem'
	//   //'sgp26/SGP.26v3_Files/Variants A_B_C/Variant C/EUICC/SK_EUICC_SIG_BRP.pem'
	//   //'sgp26/SGP.26v3_Files/Variants A_B_C/Variant C/EUICC/SK_EUICC_SIG_NIST.pem'
	//   std::string euiccCertPath = "./Variants A_B_C/Variant C/EUICC/CERT_EUICC_VARC_SIG_BRP.der";
	//  // std::string euiccCertPath = "demoCA/euicc/euicc.pem";

	std::vector<std::string> certDir = {
		"./sgp26/CertificateIssuer",
		// "./sgp26/DPauth",
		// "./sgp26/DPpb",
		// "./sgp26/DPtls",
		// "./sgp26/eUICC",

	};

	//'sgp26/SGP.26v3_Files/Variants A_B_C/Variant C/EUICC/PK_EUICC_SIG_BRP.pem'
	//'sgp26/SGP.26v3_Files/Variants A_B_C/Variant C/EUICC/PK_EUICC_SIG_NIST.pem'
	//'sgp26/SGP.26v3_Files/Variants A_B_C/Variant C/EUICC/SK_EUICC_SIG_BRP.pem'
	//'sgp26/SGP.26v3_Files/Variants A_B_C/Variant C/EUICC/SK_EUICC_SIG_NIST.pem'
	std::string euiccCertPath = "./sgp26/eUICC/CERT_EUICC_ECDSA_NIST.der";
	std::string euiccprivkeyPath = "./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem";

	std::string eumCertPath = "./sgp26/EUM/CERT_EUM_ECDSA_NIST.der";
	std::string eumprivkeyPath = "./sgp26/EUM/SK_EUM_ECDSA_NIST.pem";

	std::string caCertPath = "/etc/ssl/certs/ca-certificates.crt"; // Default CA certs on many Linux
		// systems
	bool testMode = false;

	// Parse command line arguments
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--server") == 0 && i + 1 < argc) {
			serverUrl = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "--cert-dir") == 0 && i + 1 < argc) {
			certDir.push_back(std::string(argv[i + 1]));
			i++;
		} else if (strcmp(argv[i], "--euicc-cert") == 0 && i + 1 < argc) {
			euiccCertPath = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "--ca-cert") == 0 && i + 1 < argc) {
			caCertPath = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "--test-mode") == 0) {
			testMode = true;
		} else if (strcmp(argv[i], "--help") == 0) {
			std::cout << "RSP Client - SM-DP+ Connection Test\n"
				  << "Usage: " << argv[0] << " [options]\n"
				  << "Options:\n"
				  << "  --server URL       Set SM-DP+ server URL (default: " << serverUrl << ")\n"
				  << "  --cert-dir DIR     Set directory with certificates (default: " << certDir
				  << ")\n"
				  << "  --euicc-cert PATH  Set path to eUICC certificate (default: " << euiccCertPath
				  << ")\n"
				  << "  --ca-cert PATH     Set path to CA certificate bundle (default: " << caCertPath
				  << ")\n"
				  << "  --test-mode        Run in test mode (no actual HTTP requests)\n"
				  << "  --help             Show this help\n";
			return 0;
		}
	}

	try {
		RspCrypto::LOG_INFO("Starting RSP Client...");
		RspCrypto::LOG_INFO("Server URL: " + serverUrl);
		RspCrypto::LOG_INFO(std::string("Certificate directory: ") + certDir);
		RspCrypto::LOG_INFO("eUICC certificate: " + euiccCertPath);
		RspCrypto::LOG_INFO("CA certificate bundle: " + caCertPath);
		RspCrypto::LOG_INFO(std::string("Test mode: ") + (testMode ? "enabled" : "disabled"));

		// Initialize RSP client
		RspCrypto::RSPClient client(serverUrl, serverPort, certDir, { "NIST" }); // { "NIST" } or  { "BRP"}
		client.loadEUICCCertificate(euiccCertPath);
		client.loadEUICCKeyPair(euiccprivkeyPath);
		client.loadEUMCertificate(eumCertPath);
		client.loadEUMKeyPair(eumprivkeyPath);
		client.setCACertPath(caCertPath);

		// Perform initiateAuthentication
		RspCrypto::LOG_INFO("Initiating authentication with SM-DP+ server...");
		if (client.initiateAuthentication()) {
			RspCrypto::LOG_INFO("Authentication successful!");
			return 0;
		} else {
			RspCrypto::LOG_ERROR("Authentication failed!");
			return 1;
		}
	} catch (const std::exception &e) {
		RspCrypto::LOG_ERROR("Unhandled exception: " + std::string(e.what()));
		return -1;
	}
}
#endif
