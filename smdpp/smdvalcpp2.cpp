/* ============================================================================
 * RSP Cryptographic Operations Implementation
 * File: smdvalcpp2.cpp
 *
 * This file provides cryptographic operations for Remote SIM Provisioning (RSP)
 * protocol implementation according to GSMA SGP.22 specifications.
 * ============================================================================ */

// ============================================================================
// STANDARD LIBRARY HEADERS
// ============================================================================
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

// ============================================================================
// EXTERNAL C LIBRARY HEADERS
// ============================================================================
extern "C" {
// System headers
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

// OpenSSL headers - Core
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/types.h>
#include <openssl/param_build.h>
#include <openssl/core_names.h>

// OpenSSL headers - Cryptographic operations
#include <openssl/cmac.h>
#include <openssl/ec.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>

// OpenSSL headers - X.509 certificate handling
#include <openssl/safestack.h>
#include <openssl/stack.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>

// External libraries
#include <curl/curl.h>
#include <cJSON.h>

}

// ============================================================================
// PROJECT HEADERS
// ============================================================================
#include "logger.h"
#include "helpers.h"

// ============================================================================
// NAMESPACE: RspCrypto
// ============================================================================
namespace RspCrypto {

// ============================================================================
// CLASS: CertificateUtil
// Provides comprehensive X.509 certificate handling utilities
// ============================================================================
class CertificateUtil {
public:
    // ========================================================================
    // CERTIFICATE INFORMATION EXTRACTION
    // ========================================================================

    // Extract EID (eUICC ID) from certificate subject
    static std::string getEID(X509 *cert) {
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

    // Extract subject name from certificate
    static std::string getSubjectName(X509 *cert) {
        char subjectName[256];
        X509_NAME_oneline(X509_get_subject_name(cert), subjectName, sizeof(subjectName));
        return std::string(subjectName);
    }

    // Extract issuer name from certificate
    static std::string getIssuerName(X509 *cert) {
        char issuerName[256];
        X509_NAME_oneline(X509_get_issuer_name(cert), issuerName, sizeof(issuerName));
        return std::string(issuerName);
    }

    // Extract Subject Key Identifier from certificate
    static std::vector<uint8_t> getSubjectKeyIdentifier(X509 *cert) {
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
    static std::vector<uint8_t> getAuthorityKeyIdentifier(X509 *cert) {
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

    // ========================================================================
    // CERTIFICATE LOADING AND CONVERSION
    // ========================================================================

    // Load certificate from PEM string
    static std::unique_ptr<X509, X509Deleter> loadCertFromPEM(const std::string &pemData) {
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
    static std::unique_ptr<X509, X509Deleter> loadCertFromDER(const std::vector<uint8_t> &derData) {
        const unsigned char *p = derData.data();
        X509 *cert = d2i_X509(nullptr, &p, derData.size());

        if (!cert) {
            throw OpenSSLError("Failed to parse DER certificate");
        }

        return std::unique_ptr<X509, X509Deleter>(cert);
    }

    // Load certificate chain from file
    static std::vector<std::unique_ptr<X509, X509Deleter>> loadCertificateChain(const std::string &filename) {
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

    // Convert X509 to PEM format
    static std::string certToPEM(X509 *cert) {
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

    // Convert X509 to DER format
    static std::vector<uint8_t> certToDER(X509 *cert) {
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

    // ========================================================================
    // CERTIFICATE VALIDATION
    // ========================================================================

    // Check if certificate is expired
    static bool isExpired(X509 *cert) {
        return (X509_cmp_current_time(X509_get_notAfter(cert)) <= 0);
    }

    // Check if certificate is not yet valid
    static bool isNotYetValid(X509 *cert) {
        return (X509_cmp_current_time(X509_get_notBefore(cert)) >= 0);
    }

    // Print detailed certificate information
    static void printCertificateDetails(X509 *cert) {
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

    // ========================================================================
    // CERTIFICATE CHAIN OPERATIONS
    // ========================================================================

    // Find issuer certificate using AKI/SKI matching
    static X509 *findIssuerCertificate(X509 *cert, const std::vector<X509 *> &certPool) {
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
    static STACK_OF(X509) *buildCertificateChain(X509 *cert, const std::vector<X509 *> &certPool,
                                                 bool verbose = false) {
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
                        Logger::info("  Match:   AKI and SKI match ✓");
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

    // Enhanced certificate chain verification with proper trust model
    static bool verifyCertificateChainDynamic(X509 *cert, const std::vector<X509 *> &certPool,
                                              X509 *rootCA = nullptr, bool verbose = false) {
        Logger::info("Verifying certificate chain...");
        Logger::info("Target certificate: " + getSubjectName(cert));

        // Create certificate store for trusted roots only
        std::unique_ptr<X509_STORE, X509_STORE_Deleter> store(X509_STORE_new());
        if (!store) {
            throw OpenSSLError("Failed to create X509_STORE");
        }

        // Only add the explicitly provided root CA to the trust store
        if (rootCA) {
            if (verbose) {
                Logger::info("Adding trusted root CA: " + getSubjectName(rootCA));
            }
            if (X509_STORE_add_cert(store.get(), rootCA) != 1) {
                throw OpenSSLError("Failed to add root CA to store");
            }
        } else {
            // If no root CA provided, find self-signed certificates in the pool
            // WARNING: This should only be used in test environments
            Logger::warning("No explicit root CA provided - searching for self-signed certificates");
            for (auto candidate : certPool) {
                if (X509_check_issued(candidate, candidate) == X509_V_OK) {
                    if (verbose) {
                        Logger::info("Adding self-signed certificate as trusted root: " +
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

        // Build untrusted chain from cert pool (excluding the root if it's in there)
        // IMPORTANT: We create a custom deleter that only frees the stack, not the certificates
        auto stack_only_deleter = [](STACK_OF(X509)* stack) {
            if (stack) {
                sk_X509_free(stack);  // This only frees the stack structure, not the contents
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
            Logger::info("Untrusted chain contains " + std::to_string(sk_X509_num(untrusted_chain.get())) + " certificates");
        }

        // Create verification context
        std::unique_ptr<X509_STORE_CTX, X509_STORE_CTX_Deleter> ctx(X509_STORE_CTX_new());
        if (!ctx) {
            throw OpenSSLError("Failed to create X509_STORE_CTX");
        }

        // Initialize with trust store, target cert, and untrusted chain
        if (X509_STORE_CTX_init(ctx.get(), store.get(), cert, untrusted_chain.get()) != 1) {
            throw OpenSSLError("Failed to initialize X509_STORE_CTX");
        }

        // Set verification flags
        unsigned long flags = X509_V_FLAG_CHECK_SS_SIGNATURE;
        X509_STORE_CTX_set_flags(ctx.get(), flags);

        // Set up verification callback
        X509_STORE_CTX_set_verify_cb(ctx.get(), [](int ok, X509_STORE_CTX *ctx) -> int {
            if (!ok) {
                int error = X509_STORE_CTX_get_error(ctx);
                // SGP.22 EUM certificates use name constraints in a specific way
                if (error == X509_V_ERR_PERMITTED_VIOLATION ||
                    error == X509_V_ERR_EXCLUDED_VIOLATION ||
                    error == X509_V_ERR_SUBTREE_MINMAX) {

                    // Get the certificate that triggered this
                    X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
                    if (cert) {
                        char subject_buf[256];
                        X509_NAME_oneline(X509_get_subject_name(cert), subject_buf, sizeof(subject_buf));
                        Logger::info("Processing certificate with name constraint abuse: " + std::string(subject_buf));
                    }

                    return 1; // Accept - SGP.22 specific handling
                }
            }
            return ok; // Use default behavior for other errors
        });

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

        Logger::info("Certificate verification successful");

        // Print the verified chain if verbose
        if (verbose) {
            STACK_OF(X509) *verified_chain = X509_STORE_CTX_get1_chain(ctx.get());
            if (verified_chain) {
                Logger::info("Verified certificate chain:");
                for (int i = 0; i < sk_X509_num(verified_chain); i++) {
                    X509 *chainCert = sk_X509_value(verified_chain, i);
                    Logger::info("  " + std::to_string(i + 1) + ". " + getSubjectName(chainCert));
                }
                sk_X509_pop_free(verified_chain, X509_free);
            }
        }

        return true;
    }

    // ========================================================================
    // CERTIFICATE DIRECTORY OPERATIONS
    // ========================================================================

    // Find certificate files in directory with optional filters
    static std::vector<std::filesystem::path> find_cert_files(const std::filesystem::path &root_path,
                                                             const std::vector<std::string> &name_filters = {}) {
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
    loadCertificatesFromDirectory(const std::string &directory, const std::vector<std::string> &name_filters) {
        std::vector<std::unique_ptr<X509, X509Deleter>> certificates;

        auto files_in_dir = find_cert_files(directory, name_filters);

        for (const auto &entry : files_in_dir) {
            std::string ext = entry.extension().string();
            auto fpath = entry.string();

            try {
                // Handle PEM files
                if (ext == ".pem" || ext == ".crt") {
                    auto fileCerts = loadCertificateChain(entry);
                    for (auto &cert : fileCerts) {
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
                    }
                }
            } catch (const std::exception &e) {
                // Silent error handling - already logged in called functions
            }
        }

        return certificates;
    }

    // ========================================================================
    // KEY AND CERTIFICATE LOADING UTILITIES
    // ========================================================================

    // Load certificate from file
    static void xx_loadCertificate(const std::string &certPath, const std::string &typeName,
                                  std::unique_ptr<X509, X509Deleter> &certStorage) {
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

    // Load private key from file
    static void xx_loadPrivateKey(const std::string &keyPath, const std::string &typeName,
                                 std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &keyStorage) {
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

    // Load public key from file
    static bool xx_loadPublicKey(const std::string &pubKeyPath, const std::string &typeName,
                                std::vector<uint8_t> &publicKeyStorage) {
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

            // Extract public key data using OpenSSL 3.0+ API
            // Get the public key as an octet string directly
            size_t pub_len = 0;
            
            // First get the size needed
            if (EVP_PKEY_get_octet_string_param(pubKey.get(), OSSL_PKEY_PARAM_PUB_KEY, 
                                               nullptr, 0, &pub_len) != 1) {
                Logger::warning("Failed to get public key size from " + typeName +
                                " file (will generate from private key)");
                return false;
            }
            
            // Allocate and get the actual public key data
            publicKeyStorage.resize(pub_len);
            if (EVP_PKEY_get_octet_string_param(pubKey.get(), OSSL_PKEY_PARAM_PUB_KEY,
                                               publicKeyStorage.data(), pub_len, &pub_len) != 1) {
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

    // Generate public key from private key
    static void xx_generatePublicKeyFromPrivate(const std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &privateKey,
                                               const std::string &typeName, std::vector<uint8_t> &publicKeyStorage) {
        if (!privateKey) {
            throw std::runtime_error("No " + typeName + " private key available for public key generation");
        }

        // Using OpenSSL 3.0+ API to get public key from private key
        size_t pub_len = 0;
        
        // First get the size needed
        if (EVP_PKEY_get_octet_string_param(privateKey.get(), OSSL_PKEY_PARAM_PUB_KEY,
                                           nullptr, 0, &pub_len) != 1) {
            throw OpenSSLError("Failed to get public key size from " + typeName + " private key");
        }
        
        // Allocate and get the actual public key data
        publicKeyStorage.resize(pub_len);
        if (EVP_PKEY_get_octet_string_param(privateKey.get(), OSSL_PKEY_PARAM_PUB_KEY,
                                           publicKeyStorage.data(), pub_len, &pub_len) != 1) {
            throw OpenSSLError("Failed to extract public key from " + typeName + " private key");
        }

        Logger::info("Generated " + typeName + " public key from private key (" + std::to_string(pub_len) +
                     " bytes)");
    }

    // Load complete key set (certificate, private key, public key)
    static void xx_loadCompleteKeySet(const std::string &certPath, const std::string &privKeyPath,
                                     const std::string &pubKeyPath, const std::string &typeName,
                                     std::unique_ptr<X509, X509Deleter> &certStorage,
                                     std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &privKeyStorage,
                                     std::vector<uint8_t> &pubKeyStorage) {
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
                                     std::vector<uint8_t> &pubKeyStorage) {
        // Load certificate
        xx_loadCertificate(certPath, typeName, certStorage);

        // Load private key
        xx_loadPrivateKey(privKeyPath, typeName, privKeyStorage);

        // Generate public key from private key
        xx_generatePublicKeyFromPrivate(privKeyStorage, typeName, pubKeyStorage);
    }

    // ========================================================================
    // RSP-SPECIFIC CERTIFICATE OPERATIONS
    // ========================================================================

    // Parse permitted EINs from EUM certificate
    static std::vector<std::string> parse_permitted_eins_from_cert(X509 *cert) {
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
                        GENERAL_SUBTREE *subtree = sk_GENERAL_SUBTREE_value(nc->permittedSubtrees, i);
                        if (subtree->base->type == GEN_DIRNAME) {
                            X509_NAME *name = subtree->base->d.directoryName;
                            int idx = X509_NAME_get_index_by_NID(name, NID_serialNumber, -1);
                            if (idx >= 0) {
                                X509_NAME_ENTRY *entry = X509_NAME_get_entry(name, idx);
                                ASN1_STRING *asn1_str = X509_NAME_ENTRY_get_data(entry);
                                std::string iin(
                                    reinterpret_cast<const char *>(ASN1_STRING_get0_data(asn1_str)),
                                    ASN1_STRING_length(asn1_str));
                                permitted_iins.push_back(iin);
                            }
                        }
                    }
                }
                if (nc) NAME_CONSTRAINTS_free(nc);
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

    // Get permitted EINs as comma-separated string
    static std::string getPermittedEINs(const std::vector<uint8_t>& eumCertData) {
        try {
            auto cert = loadCertFromDER(eumCertData);
            auto eins = parse_permitted_eins_from_cert(cert.get());

            // Join EINs with comma
            std::string result;
            for (size_t i = 0; i < eins.size(); i++) {
                if (i > 0) result += ",";
                result += eins[i];
            }

            return result;
        } catch (const std::exception& e) {
            Logger::error("getPermittedEINs failed: " + std::string(e.what()));
            return "";
        }
    }

    // Validate EID against EUM certificate permitted EINs
    static bool validateEIDRange(const std::string& eid, const std::vector<uint8_t>& eumCertData) {
        try {
            auto eumCert = loadCertFromDER(eumCertData);
            std::vector<std::string> permittedEins = parse_permitted_eins_from_cert(eumCert.get());

            if (permittedEins.empty()) {
                Logger::warning("No permitted EINs found in EUM certificate");
                return false;
            }

            // Check if EID starts with any permitted EIN
            std::string eidNormalized = eid;
            std::transform(eidNormalized.begin(), eidNormalized.end(), eidNormalized.begin(), ::toupper);

            for (const auto& ein : permittedEins) {
                if (eidNormalized.find(ein) == 0) {
                    Logger::info("EID " + eidNormalized + " matches permitted EIN " + ein);
                    return true;
                }
            }

            Logger::error("EID " + eidNormalized + " is not in any permitted EIN list");
            return false;
        } catch (const std::exception& e) {
            Logger::error("validateEIDRange failed: " + std::string(e.what()));
            return false;
        }
    }

    // Check if certificate has a specific RSP role OID
    static bool hasRSPRole(const std::vector<uint8_t>& certData, const std::string& roleOid) {
        try {
            auto cert = loadCertFromDER(certData);

            // Check certificate policies for RSP role
            int pos = X509_get_ext_by_NID(cert.get(), NID_certificate_policies, -1);
            if (pos < 0) return false;

            X509_EXTENSION *ext = X509_get_ext(cert.get(), pos);
            CERTIFICATEPOLICIES *policies = (CERTIFICATEPOLICIES*)X509V3_EXT_d2i(ext);

            if (!policies) return false;

            for (int i = 0; i < sk_POLICYINFO_num(policies); i++) {
                POLICYINFO *policy = sk_POLICYINFO_value(policies, i);
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
            Logger::error("hasRSPRole failed: " + std::string(e.what()));
            return false;
        }
    }

    // Get curve OID from certificate public key
    static std::string getCurveOID(const std::vector<uint8_t>& certData) {
        try {
            auto cert = loadCertFromDER(certData);

            EVP_PKEY *pkey = X509_get0_pubkey(cert.get());
            if (!pkey) {
                Logger::error("Failed to get public key from certificate");
                return "";
            }

            if (EVP_PKEY_base_id(pkey) != EVP_PKEY_EC) {
                Logger::error("Certificate does not contain EC key");
                return "";
            }

            char curve_name[256] = {0};
            size_t curve_name_len = sizeof(curve_name);

            if (EVP_PKEY_get_utf8_string_param(pkey, "group", curve_name,
                                             sizeof(curve_name), &curve_name_len) != 1) {
                Logger::error("Failed to get curve name from EC key");
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
            Logger::error("getCurveOID failed: " + std::string(e.what()));
            return "";
        }
    }

    // Check if two EC public keys are ECDH compatible (same curve)
    static bool verifyECDHCompatible(const std::vector<uint8_t>& pubKey1,
                                    const std::vector<uint8_t>& pubKey2) {
        try {
            // Both should be uncompressed EC points (starting with 0x04)
            if (pubKey1.empty() || pubKey2.empty() || pubKey1[0] != 0x04 || pubKey2[0] != 0x04) {
                Logger::error("Invalid EC public key format");
                return false;
            }

            // For P-256, size should be 65 bytes (1 + 32 + 32)
            if (pubKey1.size() == 65 && pubKey2.size() == 65) {
                Logger::info("Both keys are P-256 format, ECDH compatible");
                return true;
            }

            // For P-384, size should be 97 bytes (1 + 48 + 48)
            if (pubKey1.size() == 97 && pubKey2.size() == 97) {
                Logger::info("Both keys are P-384 format, ECDH compatible");
                return true;
            }

            Logger::error("Key sizes don't match or unsupported curve");
            return false;
        } catch (const std::exception& e) {
            Logger::error("verifyECDHCompatible failed: " + std::string(e.what()));
            return false;
        }
    }

    // ========================================================================
    // SIGNATURE OPERATIONS
    // ========================================================================

    // Convert BSI TR-03111 signature format to DER
    static std::vector<uint8_t> convertBSI_TR03111_to_DER(const std::vector<uint8_t> &bsi_signature) {
        if (bsi_signature.size() != 64) {
            std::cerr << "Invalid BSI TR-03111 signature size: " << bsi_signature.size() << std::endl;
            return {};
        }

        BIGNUM *r = BN_new();
        BIGNUM *s = BN_new();

        if (!r || !s) {
            if (r) BN_free(r);
            if (s) BN_free(s);
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

    // Convert DER signature format to BSI TR-03111
    static std::vector<uint8_t> convertDER_to_BSI_TR03111(const std::vector<uint8_t> &der_signature) {
        if (der_signature.empty()) {
            std::cerr << "Empty DER signature" << std::endl;
            return {};
        }

        // Parse DER signature
        const unsigned char *der_ptr = der_signature.data();
        ECDSA_SIG *ecdsa_sig = d2i_ECDSA_SIG(nullptr, &der_ptr, der_signature.size());

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

    // Verify signature in BSI TR-03111 format
    static bool verify_TR031111(const std::vector<uint8_t> &message, const std::vector<uint8_t> &bsi_signature,
                               EVP_PKEY *publicKey) {
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
};

// ============================================================================
// CLASS: HttpClient
// Provides HTTP/HTTPS client functionality for SM-DP+ server communication
// ============================================================================
class HttpClient {
public:
    HttpClient() {
        // Initialize libcurl globally - should be called once per application
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~HttpClient() {
        // Clean up libcurl
        curl_global_cleanup();
    }

    // Structure to store response data
    struct ResponseData {
        std::string body;
        long statusCode;
        std::string headers;
    };

private:
    // Callback function for writing response data
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        size_t realSize = size * nmemb;
        auto *response = static_cast<std::string *>(userp);
        response->append(static_cast<char *>(contents), realSize);
        return realSize;
    }

    // Callback function for writing header data
    static size_t headerCallback(void *contents, size_t size, size_t nmemb, void *userp) {
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

    // Get common name from X509_NAME
    static std::string get_cn_name(X509_NAME *const name) {
        int idx = -1;
        unsigned char *utf8 = NULL;
        X509_NAME_ENTRY *entry;
        ASN1_STRING *data;

        if (!name) return {};

        idx = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
        if (idx < 0) return {};

        entry = X509_NAME_get_entry(name, idx);
        if (!entry) return {};

        data = X509_NAME_ENTRY_get_data(entry);
        if (!data) return {};

        if (!ASN1_STRING_to_UTF8(&utf8, data) || !utf8) return {};

        std::string retstr((char *)utf8);
        OPENSSL_free(utf8);
        return retstr;
    }

    // Set up custom certificate verification for CURL
    static CURLcode sslCtxFunction(CURL *curl, SSL_CTX *sslCtx, void *arg) {
        SslCtxData *ctxData = static_cast<SslCtxData *>(arg);
        SSL_CTX *ctx = (SSL_CTX *)sslCtx;
        X509_STORE *store = SSL_CTX_get_cert_store(ctx);

        for (auto cert : *ctxData->certPool) {
            Logger::info("ADDED Issuer: " + get_cn_name(X509_get_issuer_name(cert)) +
                         " Subject: " + get_cn_name(X509_get_subject_name(cert)) +
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

        // Turn on verification
        SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER, nullptr);

        return CURLE_OK;
    }

public:
    // Perform HTTPS request with custom certificate verification
    ResponseData postJsonWithCustomVerification(const std::string &url, unsigned int port,
                                               const std::string &jsonData, X509_STORE *store,
                                               std::vector<X509 *> &certPool) {
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

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Compute confirmation code hash according to SGP.22 specification
// Hashed Confirmation Code = SHA256(SHA256(Confirmation Code) | TransactionID)
static std::vector<uint8_t> computeConfirmationCodeHash(
    const std::string& confirmationCode,
    const std::vector<uint8_t>& transactionId) {
    
    // Convert confirmation code from hex string to bytes (like EID handling)
    std::vector<uint8_t> ccBytes;
    for (size_t i = 0; i < confirmationCode.length(); i += 2) {
        if (i + 1 < confirmationCode.length()) {
            std::string byteString = confirmationCode.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
            ccBytes.push_back(byte);
        }
    }
    
    // Step 1: SHA256(Confirmation Code)
    unsigned char firstHash[SHA256_DIGEST_LENGTH];
    SHA256(ccBytes.data(), ccBytes.size(), firstHash);
    
    // Step 2: Concatenate SHA256(CC) with TransactionID
    std::vector<uint8_t> concatenated;
    concatenated.insert(concatenated.end(), firstHash, firstHash + SHA256_DIGEST_LENGTH);
    concatenated.insert(concatenated.end(), transactionId.begin(), transactionId.end());
    
    // Step 3: SHA256(SHA256(CC) | TransactionID)
    unsigned char finalHash[SHA256_DIGEST_LENGTH];
    SHA256(concatenated.data(), concatenated.size(), finalHash);
    
    return std::vector<uint8_t>(finalHash, finalHash + SHA256_DIGEST_LENGTH);
}

// ============================================================================
// CLASS: RSPClient
// Main RSP client implementation for Remote SIM Provisioning operations
// ============================================================================
class RSPClient {
public:
    // Constructor
    RSPClient(const std::string &serverUrl, const unsigned int serverPort,
              const std::vector<std::string> &certPath,
              const std::vector<std::string> &name_filters = {})
        : m_serverUrl(serverUrl), m_serverPort(serverPort) {
        // OpenSSL 3.0+ initializes automatically

        for (auto cpath : certPath) {
            bool isDirectory = std::filesystem::is_directory(cpath);
            try {
                if (isDirectory) {
                    Logger::info("Loading certificates from directory: " + cpath);

                    // Load all certificates from the directory
                    auto certs = CertificateUtil::loadCertificatesFromDirectory(cpath, name_filters);
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

    ~RSPClient() {
        // OpenSSL 3.0+ handles cleanup automatically
    }

    // ========================================================================
    // PUBLIC INTERFACE - CERTIFICATE AND KEY MANAGEMENT
    // ========================================================================

    // Set the path to CA certificates for system verification
    void setCACertPath(const std::string &path) {
        m_caCertPath = path;
    }

    // Load eUICC certificate
    void loadEUICCCertificate(const std::string &euiccCertPath) {
        CertificateUtil::xx_loadCertificate(euiccCertPath, "eUICC", m_euiccCert);

        // EUICC-specific operations
        try {
            m_EID = CertificateUtil::getEID(m_euiccCert.get());
            Logger::info("Using EID from certificate: " + m_EID);

            auto ski = CertificateUtil::getSubjectKeyIdentifier(m_euiccCert.get());
            m_euiccSKI = ski;
            Logger::info("Using eUICC SKI as PKID: " + HexUtil::bytesToHex(m_euiccSKI));
        } catch (const std::exception &e) {
            Logger::error("Failed to extract EUICC-specific data: " + std::string(e.what()));
            throw;
        }
    }

    // Load eUICC key pair
    void loadEUICCKeyPair(const std::string &euiccPrivateKeyPath, const std::string &euiccPublicKeyPath = "") {
        CertificateUtil::xx_loadPrivateKey(euiccPrivateKeyPath, "eUICC", m_euiccPrivateKey);

        if (!euiccPublicKeyPath.empty() &&
            CertificateUtil::xx_loadPublicKey(euiccPublicKeyPath, "eUICC", m_euiccPublicKeyData)) {
            // Successfully loaded public key from file
            return;
        }

        // Generate public key from private key
        CertificateUtil::xx_generatePublicKeyFromPrivate(m_euiccPrivateKey, "eUICC", m_euiccPublicKeyData);
    }

    // Load EUM certificate
    void loadEUMCertificate(const std::string &eumCertPath) {
        CertificateUtil::xx_loadCertificate(eumCertPath, "EUM", m_eumCert);
    }

    // Load EUM key pair
    void loadEUMKeyPair(const std::string &eumPrivateKeyPath, const std::string &eumPublicKeyPath = "") {
        CertificateUtil::xx_loadPrivateKey(eumPrivateKeyPath, "EUM", m_eumPrivateKey);

        if (!eumPublicKeyPath.empty() &&
            CertificateUtil::xx_loadPublicKey(eumPublicKeyPath, "EUM", m_eumPublicKeyData)) {
            // Successfully loaded public key from file
            return;
        }

        // Generate public key from private key
        CertificateUtil::xx_generatePublicKeyFromPrivate(m_eumPrivateKey, "EUM", m_eumPublicKeyData);
    }

    // Generic complete key set loader
    void loadAnyCompleteKeySet(const std::string &certPath, const std::string &privKeyPath,
                               const std::string &pubKeyPath, const std::string &typeName,
                               std::unique_ptr<X509, X509Deleter> &certStorage,
                               std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> &privKeyStorage,
                               std::vector<uint8_t> &pubKeyStorage) {
        CertificateUtil::xx_loadCompleteKeySet(certPath, privKeyPath, pubKeyPath, typeName,
                                              certStorage, privKeyStorage, pubKeyStorage);
    }

    // ========================================================================
    // PUBLIC INTERFACE - CRYPTOGRAPHIC OPERATIONS
    // ========================================================================

    // Generate a random challenge (16 bytes)
    std::vector<uint8_t> generateChallenge() {
        std::vector<uint8_t> challenge(16);
        if (RAND_bytes(challenge.data(), challenge.size()) != 1) {
            throw OpenSSLError("Failed to generate random challenge");
        }
        return challenge;
    }

    // Generate eUICC one-time public key (OtPK)
    void generateEUICCOtpk() {
        // Create EC key context for P-256 curve using OpenSSL 3.0+ API
        std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> 
            pctx(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr), EVP_PKEY_CTX_free);
        if (!pctx) {
            throw OpenSSLError("Failed to create EVP_PKEY_CTX");
        }

        // Initialize key generation
        if (EVP_PKEY_keygen_init(pctx.get()) <= 0) {
            throw OpenSSLError("Failed to initialize key generation");
        }

        // Set the curve to P-256 (prime256v1)
        if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx.get(), NID_X9_62_prime256v1) <= 0) {
            throw OpenSSLError("Failed to set P-256 curve");
        }

        // Generate the key pair
        EVP_PKEY *pkey_raw = nullptr;
        if (EVP_PKEY_keygen(pctx.get(), &pkey_raw) <= 0) {
            throw OpenSSLError("Failed to generate EC key pair");
        }
        
        // Store the private key for later use in key agreement
        m_euicc_ot_PrivateKey.reset(pkey_raw);

        // Get the public key in uncompressed format
        size_t pub_len = 0;
        
        // First get the size needed
        if (EVP_PKEY_get_octet_string_param(m_euicc_ot_PrivateKey.get(), OSSL_PKEY_PARAM_PUB_KEY,
                                           nullptr, 0, &pub_len) != 1) {
            throw OpenSSLError("Failed to get public key size");
        }
        
        // For P-256: should be 65 bytes (0x04 + 32 bytes X + 32 bytes Y)
        if (pub_len != 65) {
            throw OpenSSLError("Unexpected public key size for P-256");
        }
        
        // Get the actual public key data
        m_euiccOtpk.resize(pub_len);
        if (EVP_PKEY_get_octet_string_param(m_euicc_ot_PrivateKey.get(), OSSL_PKEY_PARAM_PUB_KEY,
                                           m_euiccOtpk.data(), pub_len, &pub_len) != 1) {
            throw OpenSSLError("Failed to extract public key");
        }

        Logger::info("Generated eUICC OtPK (P-256): " + HexUtil::bytesToHex(m_euiccOtpk));

        // Verify the key format
        if (m_euiccOtpk[0] != 0x04) {
            throw std::runtime_error("Invalid public key format - expected uncompressed point");
        }

        Logger::debug("Public key format verified - uncompressed EC point");
    }

    // Compute ECDH shared secret
    std::vector<uint8_t> computeECDHSharedSecret(const std::vector<uint8_t> &otherPublicKey) {
        if (!m_euicc_ot_PrivateKey) {
            throw std::runtime_error("eUICC ephemeral private key not available");
        }

        // Create EVP_PKEY for the other party's public key using OpenSSL 3.0+ API
        std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>
            pctx(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr), EVP_PKEY_CTX_free);
        if (!pctx) {
            throw OpenSSLError("Failed to create EVP_PKEY_CTX");
        }

        if (EVP_PKEY_fromdata_init(pctx.get()) <= 0) {
            throw OpenSSLError("Failed to initialize fromdata");
        }

        // Build OSSL_PARAM array for the public key
        OSSL_PARAM_BLD *param_bld = OSSL_PARAM_BLD_new();
        if (!param_bld) {
            throw OpenSSLError("Failed to create OSSL_PARAM_BLD");
        }

        // Set the curve name (P-256)
        if (!OSSL_PARAM_BLD_push_utf8_string(param_bld, OSSL_PKEY_PARAM_GROUP_NAME, "P-256", 0)) {
            OSSL_PARAM_BLD_free(param_bld);
            throw OpenSSLError("Failed to set curve name");
        }

        // Set the public key
        if (!OSSL_PARAM_BLD_push_octet_string(param_bld, OSSL_PKEY_PARAM_PUB_KEY,
                                             otherPublicKey.data(), otherPublicKey.size())) {
            OSSL_PARAM_BLD_free(param_bld);
            throw OpenSSLError("Failed to set public key");
        }

        OSSL_PARAM *params = OSSL_PARAM_BLD_to_param(param_bld);
        if (!params) {
            OSSL_PARAM_BLD_free(param_bld);
            throw OpenSSLError("Failed to build params");
        }

        EVP_PKEY *other_pkey_raw = nullptr;
        if (EVP_PKEY_fromdata(pctx.get(), &other_pkey_raw, EVP_PKEY_PUBLIC_KEY, params) <= 0) {
            OSSL_PARAM_free(params);
            OSSL_PARAM_BLD_free(param_bld);
            throw OpenSSLError("Failed to create public key from data");
        }

        OSSL_PARAM_free(params);
        OSSL_PARAM_BLD_free(param_bld);

        std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> other_pkey(other_pkey_raw);

        // Perform ECDH using EVP_PKEY_derive
        std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>
            derive_ctx(EVP_PKEY_CTX_new(m_euicc_ot_PrivateKey.get(), nullptr), EVP_PKEY_CTX_free);
        if (!derive_ctx) {
            throw OpenSSLError("Failed to create derive context");
        }

        if (EVP_PKEY_derive_init(derive_ctx.get()) <= 0) {
            throw OpenSSLError("Failed to initialize key derivation");
        }

        if (EVP_PKEY_derive_set_peer(derive_ctx.get(), other_pkey.get()) <= 0) {
            throw OpenSSLError("Failed to set peer key");
        }

        // Determine shared secret length
        size_t secret_len = 0;
        if (EVP_PKEY_derive(derive_ctx.get(), nullptr, &secret_len) <= 0) {
            throw OpenSSLError("Failed to get shared secret length");
        }

        // Perform the derivation
        std::vector<uint8_t> shared_secret(secret_len);
        if (EVP_PKEY_derive(derive_ctx.get(), shared_secret.data(), &secret_len) <= 0) {
            throw OpenSSLError("ECDH computation failed");
        }

        // Resize to actual length (should be the same)
        shared_secret.resize(secret_len);

        Logger::info("Computed ECDH shared secret: " + HexUtil::bytesToHex(shared_secret));
        return shared_secret;
    }

    // Sign data with eUICC private key
    std::vector<uint8_t> signDataWithEUICC(const std::vector<uint8_t> &dataToSign) {
        if (!m_euiccPrivateKey) {
            throw std::runtime_error("eUICC private key not available for signing");
        }
        auto rv = signDataWithKey(dataToSign, m_euiccPrivateKey.get());
        return CertificateUtil::convertDER_to_BSI_TR03111(rv);
    }

    // Verify server signature (with DER certificate data)
    bool verifyServerSignature(const std::vector<uint8_t> &serverSigned1,
                              const std::vector<uint8_t> &serverSignature1,
                              const std::vector<uint8_t> &derDataServerCert) {
        auto serverCert = CertificateUtil::loadCertFromDER(derDataServerCert);
        return verifyServerSignature(serverSigned1, serverSignature1, serverCert.get());
    }

    // Set confirmation code (calculates hash according to SGP.22)
    void setConfirmationCode(const std::string &confirmationCode) {
        m_confirmationCode = confirmationCode;
        if (!m_transactionId.empty()) {
            m_confirmationCodeHash = computeConfirmationCodeHash(m_confirmationCode, m_transactionId);
            Logger::info("Set confirmation code, computed hash: " + HexUtil::bytesToHex(m_confirmationCodeHash));
        } else {
            Logger::warning("Cannot compute confirmation code hash - transaction ID not set");
        }
    }
    
    // Set transaction ID (needed for confirmation code hash)
    void setTransactionId(const std::vector<uint8_t> &transactionId) {
        m_transactionId = transactionId;
        Logger::info("Set transaction ID: " + HexUtil::bytesToHex(transactionId));
        // If confirmation code was already set, recalculate hash
        if (!m_confirmationCode.empty()) {
            m_confirmationCodeHash = computeConfirmationCodeHash(m_confirmationCode, m_transactionId);
            Logger::info("Recalculated confirmation code hash: " + HexUtil::bytesToHex(m_confirmationCodeHash));
        }
    }
    
    // Get computed confirmation code hash
    std::vector<uint8_t> getConfirmationCodeHash() const {
        return m_confirmationCodeHash;
    }
    
    // Set confirmation code hash directly (for backwards compatibility)
    void setConfirmationCodeHash(const std::vector<uint8_t> &hash) {
        m_confirmationCodeHash = hash;
        Logger::info("Set confirmation code hash directly: " + HexUtil::bytesToHex(hash));
    }

    // ========================================================================
    // PUBLIC INTERFACE - CERTIFICATE GETTERS
    // ========================================================================

    std::vector<uint8_t> getEUMCertificate() {
        return CertificateUtil::certToDER(m_eumCert.get());
    }

    std::vector<uint8_t> getEUICCCertificate() {
        return CertificateUtil::certToDER(m_euiccCert.get());
    }

    std::vector<uint8_t> getCICertificate() {
        if (m_rootCA) {
            return CertificateUtil::certToDER(m_rootCA.get());
        }
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> getEUICCOtpk() {
        return m_euiccOtpk;
    }

private:
    // ========================================================================
    // PRIVATE METHODS
    // ========================================================================

    // Sign data with a specific key
    std::vector<uint8_t> signDataWithKey(const std::vector<uint8_t> &dataToSign, EVP_PKEY* pkey) {
        if (dataToSign.empty()) {
            throw std::runtime_error("Data to sign is empty");
        }

        // Create signature context
        std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdctx(EVP_MD_CTX_new());
        if (!mdctx) {
            throw OpenSSLError("Failed to create signature context");
        }

        // Initialize signing with SHA256
        if (EVP_DigestSignInit(mdctx.get(), nullptr, EVP_sha256(), nullptr, pkey) != 1) {
            throw OpenSSLError("Failed to initialize signature operation");
        }

        // Update with data to be signed
        if (EVP_DigestSignUpdate(mdctx.get(), dataToSign.data(), dataToSign.size()) != 1) {
            throw OpenSSLError("Failed to update signature");
        }

        // Get signature length
        size_t sigLen;
        if (EVP_DigestSignFinal(mdctx.get(), nullptr, &sigLen) != 1) {
            throw OpenSSLError("Failed to determine signature length");
        }

        if (sigLen == 0) {
            throw std::runtime_error("signature length is zero");
        }

        // Create signature
        std::vector<uint8_t> signature(sigLen);
        if (EVP_DigestSignFinal(mdctx.get(), signature.data(), &sigLen) != 1) {
            throw OpenSSLError("Failed to create signature");
        }

        signature.resize(sigLen);
        Logger::debug("Created signature (" + std::to_string(sigLen) + " bytes) over " +
                      std::to_string(dataToSign.size()) + " bytes of data");

        return signature;
    }

    // Verify server signature
    bool verifyServerSignature(const std::vector<uint8_t> &signedData,
                              const std::vector<uint8_t> &signature, X509* scertdata) {
        try {
            // Handle special prefix in serverSignature1 based on SGP.22 spec
            const unsigned char *signatureData = signature.data();
            size_t signatureLen = signature.size();

            if (signatureLen >= 3 && signatureData[0] == 0x5f && signatureData[1] == 0x37 &&
                signatureData[2] == 0x40) {
                LOG_INFO("Detected 5F3740 prefix in serverSignature1, skipping for verification");
                signatureData += 3;
                signatureLen -= 3;
            }

            // Extract public key from server certificate
            std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pubkey(X509_get_pubkey(scertdata));
            if (!pubkey) {
                throw OpenSSLError("Failed to extract public key from certificate");
            }

            auto ski = CertificateUtil::getSubjectKeyIdentifier(scertdata);
            Logger::info("----------- using cert with this SKI to verify sig: " + HexUtil::bytesToHex(ski));

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

    bool verifyServerSignature(const std::vector<uint8_t> &serverSigned1,
                              const std::vector<uint8_t> &serverSignature1) {
        return verifyServerSignature(serverSigned1, serverSignature1, m_serverCert.get());
    }

    // ========================================================================
    // MEMBER VARIABLES
    // ========================================================================

    // Server configuration
    std::string m_serverUrl;
    unsigned int m_serverPort;

    // Certificate storage
    std::unique_ptr<X509, X509Deleter> m_rootCA;
    std::vector<std::unique_ptr<X509, X509Deleter>> m_intermediateCA;
    std::unique_ptr<X509, X509Deleter> m_serverCert;
    std::vector<std::unique_ptr<X509, X509Deleter>> m_certPool;

    // eUICC data
    std::vector<uint8_t> m_euiccSKI; // eUICC SKI for PKIDs
    std::string m_EID; // eUICC ID
    std::unique_ptr<X509, X509Deleter> m_euiccCert; // eUICC certificate
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_euiccPrivateKey; // eUICC private key for signing
    std::vector<uint8_t> m_euiccPublicKeyData; // eUICC public key data
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_euicc_ot_PrivateKey; // One-time private key
    std::vector<uint8_t> m_euiccOtpk; // One-time public key from eUICC

    // EUM data
    std::unique_ptr<X509, X509Deleter> m_eumCert; // EUM certificate
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_eumPrivateKey; // EUM private key for signing
    std::vector<uint8_t> m_eumPublicKeyData; // EUM public key data

    // Other data
    std::string m_confirmationCode; // Confirmation code (PIN)
    std::vector<uint8_t> m_confirmationCodeHash; // Computed confirmation code hash
    std::vector<uint8_t> m_transactionId; // Transaction ID for hash computation
    std::string m_caCertPath = ""; // Path to system CA certificates
};

} // namespace RspCrypto