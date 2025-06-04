
#include <cstddef>
#include <filesystem>
#include <iterator>
extern "C" {
#include <openssl/asn1t.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/crypto.h>
#include <openssl/cmac.h>
#include <openssl/err.h>
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
}

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
#include <cJSON.h>
#include "rsp_ossl_imp.h"
}

DEFINE_STACK_OF(ASN1_OCTET_STRING)

namespace RspCrypto
{
class SMDPResponseGenerator;
class SMDPResponseValidator;

// Enhanced Logger class with filename and line number information
class Logger {
    public:
	enum class Level { DEBUG, INFO, WARNING, ERROR };

	static void log(Level level, const std::string &message, const char *filename = nullptr, int line = 0)
	{
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);

		std::string levelStr;
		switch (level) {
		case Level::DEBUG:
			levelStr = "DEBUG";
			break;
		case Level::INFO:
			levelStr = "INFO";
			break;
		case Level::WARNING:
			levelStr = "WARNING";
			break;
		case Level::ERROR:
			levelStr = "ERROR";
			break;
		}

		std::stringstream ss;
		ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " << std::setw(7)
		   << std::left << levelStr;

		// Add filename and line information if provided
		if (filename) {
			// Extract just the base filename without the full path
			const char *base_filename = strrchr(filename, '/');
			if (!base_filename) {
				base_filename = strrchr(filename, '\\'); // For Windows paths
			}
			base_filename = base_filename ? base_filename + 1 : filename;

			ss << " [" << base_filename << ":" << line << "]";
		}

		ss << " " << message;

		std::cout << ss.str() << std::endl;
	}

	// Helper methods with filename and line support
	static void debug(const std::string &message, const char *filename = nullptr, int line = 0)
	{
		log(Level::DEBUG, message, filename, line);
	}

	static void info(const std::string &message, const char *filename = nullptr, int line = 0)
	{
		log(Level::INFO, message, filename, line);
	}

	static void warning(const std::string &message, const char *filename = nullptr, int line = 0)
	{
		log(Level::WARNING, message, filename, line);
	}

	static void error(const std::string &message, const char *filename = nullptr, int line = 0)
	{
		log(Level::ERROR, message, filename, line);
	}
};

// Define macros to automatically include file and line information
#define LOG_DEBUG(message) Logger::debug(message, __FILE__, __LINE__)
#define LOG_INFO(message) Logger::info(message, __FILE__, __LINE__)
#define LOG_WARNING(message) Logger::warning(message, __FILE__, __LINE__)
#define LOG_ERROR(message) Logger::error(message, __FILE__, __LINE__)

// Class for OpenSSL error handling
class OpenSSLError : public std::runtime_error {
    public:
	OpenSSLError(const std::string &message) : std::runtime_error(message + getOpenSSLErrors())
	{
	}

    private:
	static std::string getOpenSSLErrors()
	{
		std::stringstream ss;
		unsigned long err;

		ss << " - OpenSSL Errors: ";
		while ((err = ERR_get_error()) != 0) {
			char *err_str = ERR_error_string(err, nullptr);
			ss << err_str << "; ";
		}

		return ss.str();
	}
};

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <string>
#include <sstream>
#include <iostream>

class OpenSSLErrorHandler {
    public:
	// Get detailed error information from OpenSSL error queue
	static std::string getLastError()
	{
		std::stringstream errorStream;
		unsigned long errorCode;

		while ((errorCode = ERR_get_error()) != 0) {
			char errorBuffer[256];
			ERR_error_string_n(errorCode, errorBuffer, sizeof(errorBuffer));

			const char *library = ERR_lib_error_string(errorCode);

#if OPENSSL_VERSION_NUMBER < 0x30000000
			const char *function = ERR_func_error_string(errorCode);
#endif
			const char *reason = ERR_reason_error_string(errorCode);

			errorStream << "OpenSSL Error:\n";
			errorStream << "  Code: 0x" << std::hex << errorCode << std::dec << "\n";
			errorStream << "  Library: " << (library ? library : "unknown") << "\n";

#if OPENSSL_VERSION_NUMBER < 0x30000000
			errorStream << "  Function: " << (function ? function : "unknown") << "\n";
#endif
			errorStream << "  Reason: " << (reason ? reason : "unknown") << "\n";
			errorStream << "  Full: " << errorBuffer << "\n\n";
		}

		return errorStream.str();
	}

	// Get SSL-specific error with context
	static std::string getSSLError(SSL *ssl, int result)
	{
		int sslError = SSL_get_error(ssl, result);
		std::stringstream errorStream;

		errorStream << "SSL Error Details:\n";
		errorStream << "  Return code: " << result << "\n";
		errorStream << "  SSL error code: " << sslError << "\n";
		errorStream << "  SSL error type: " << getSSLErrorType(sslError) << "\n";

		// Add OpenSSL error queue information
		std::string queueErrors = getLastError();
		if (!queueErrors.empty()) {
			errorStream << "\nError Queue:\n" << queueErrors;
		}

		return errorStream.str();
	}

	// Get certificate verification error
	static std::string getCertificateError(long verifyResult)
	{
		std::stringstream errorStream;

		if (verifyResult != X509_V_OK) {
			const char *errorString = X509_verify_cert_error_string(verifyResult);
			errorStream << "Certificate Verification Error:\n";
			errorStream << "  Code: " << verifyResult << "\n";
			errorStream << "  Description: " << (errorString ? errorString : "unknown") << "\n";
		}

		return errorStream.str();
	}

	// Pretty print all SSL errors with context
	static void printSSLErrors(const std::string &context = "")
	{
		if (!context.empty()) {
			std::cerr << "=== " << context << " ===" << std::endl;
		}

		std::string errors = getLastError();
		if (!errors.empty()) {
			std::cerr << errors << std::endl;
		} else {
			std::cerr << "No OpenSSL errors in queue." << std::endl;
		}
	}

	// Initialize error strings (call once at startup)
	static void initializeErrorStrings()
	{
		SSL_load_error_strings();
		ERR_load_crypto_strings();
		OpenSSL_add_all_algorithms();
	}

    private:
	static std::string getSSLErrorType(int sslError)
	{
		switch (sslError) {
		case SSL_ERROR_NONE:
			return "SSL_ERROR_NONE - No error";
		case SSL_ERROR_SSL:
			return "SSL_ERROR_SSL - SSL library error";
		case SSL_ERROR_WANT_READ:
			return "SSL_ERROR_WANT_READ - Need more data to read";
		case SSL_ERROR_WANT_WRITE:
			return "SSL_ERROR_WANT_WRITE - Need to write data";
		case SSL_ERROR_WANT_X509_LOOKUP:
			return "SSL_ERROR_WANT_X509_LOOKUP - X509 lookup required";
		case SSL_ERROR_SYSCALL:
			return "SSL_ERROR_SYSCALL - System call error";
		case SSL_ERROR_ZERO_RETURN:
			return "SSL_ERROR_ZERO_RETURN - Connection closed";
		case SSL_ERROR_WANT_CONNECT:
			return "SSL_ERROR_WANT_CONNECT - Want connect";
		case SSL_ERROR_WANT_ACCEPT:
			return "SSL_ERROR_WANT_ACCEPT - Want accept";
		default:
			return "Unknown SSL error type";
		}
	}
};

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

struct SMDP_SIGNED2_Deleter {
	void operator()(SMDP_SIGNED2 *ss2) const
	{
		if (ss2)
			SMDP_SIGNED2_free(ss2);
	}
};

struct PREPARE_DOWNLOAD_REQUEST_Deleter {
	void operator()(PREPARE_DOWNLOAD_REQUEST *pdr) const
	{
		if (pdr)
			PREPARE_DOWNLOAD_REQUEST_free(pdr);
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

struct EUICC_SIGNED2_Deleter {
	void operator()(EUICC_SIGNED2 *es2) const
	{
		EUICC_SIGNED2_free(es2);
	}
};

struct PREPARE_DOWNLOAD_RESPONSE_OK_Deleter {
	void operator()(PREPARE_DOWNLOAD_RESPONSE_OK *pdro) const
	{
		PREPARE_DOWNLOAD_RESPONSE_OK_free(pdro);
	}
};

struct PREPARE_DOWNLOAD_RESPONSE_ERROR_Deleter {
	void operator()(PREPARE_DOWNLOAD_RESPONSE_ERROR *pdre) const
	{
		PREPARE_DOWNLOAD_RESPONSE_ERROR_free(pdre);
	}
};

struct PREPARE_DOWNLOAD_RESPONSE_Deleter {
	void operator()(PREPARE_DOWNLOAD_RESPONSE *pdr) const
	{
		PREPARE_DOWNLOAD_RESPONSE_free(pdr);
	}
};

struct GET_BOUND_PROFILE_PACKAGE_REQUEST_Deleter {
	void operator()(GET_BOUND_PROFILE_PACKAGE_REQUEST *gbppr) const
	{
		GET_BOUND_PROFILE_PACKAGE_REQUEST_free(gbppr);
	}
};

using EUICCSigned2Ptr = std::unique_ptr<EUICC_SIGNED2, EUICC_SIGNED2_Deleter>;
using PrepareDownloadResponseOkPtr =
	std::unique_ptr<PREPARE_DOWNLOAD_RESPONSE_OK, PREPARE_DOWNLOAD_RESPONSE_OK_Deleter>;
using PrepareDownloadResponseErrorPtr =
	std::unique_ptr<PREPARE_DOWNLOAD_RESPONSE_ERROR, PREPARE_DOWNLOAD_RESPONSE_ERROR_Deleter>;
using PrepareDownloadResponsePtr = std::unique_ptr<PREPARE_DOWNLOAD_RESPONSE, PREPARE_DOWNLOAD_RESPONSE_Deleter>;
using GetBoundProfilePackageRequestPtr =
	std::unique_ptr<GET_BOUND_PROFILE_PACKAGE_REQUEST, GET_BOUND_PROFILE_PACKAGE_REQUEST_Deleter>;

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

	// Dynamic chain verification
	static bool verifyDynamicChain(X509 *cert, const std::vector<X509 *> &certPool, X509 *rootCA)
	{
		// Create a store with the root CA
		std::unique_ptr<X509_STORE, X509_STORE_Deleter> store(X509_STORE_new());
		if (!store) {
			throw OpenSSLError("Failed to create X509_STORE");
		}

		if (X509_STORE_add_cert(store.get(), rootCA) != 1) {
			throw OpenSSLError("Failed to add root CA to store");
		}

		// Build the chain dynamically
		std::unique_ptr<STACK_OF(X509), STACK_OF_X509_Deleter> chain(buildCertificateChain(cert, certPool));

		// Create verification context
		std::unique_ptr<X509_STORE_CTX, X509_STORE_CTX_Deleter> ctx(X509_STORE_CTX_new());
		if (!ctx) {
			throw OpenSSLError("Failed to create X509_STORE_CTX");
		}

		// Initialize verification context
		if (X509_STORE_CTX_init(ctx.get(), store.get(), cert, chain.get()) != 1) {
			throw OpenSSLError("Failed to initialize X509_STORE_CTX");
		}

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

	// Verify a certificate with a dynamically discovered chain
	static bool verifyWithDiscoveredChain(X509 *cert, const std::vector<X509 *> &certPool,
					      bool requireValidPath = true)
	{
		// Find the chain automatically
		auto chain = findCertificateChain(cert, certPool);

		if (chain.size() <= 1 && requireValidPath) {
			LOG_ERROR("Could not build a complete certificate chain");
			return false;
		}

		// Display the chain
		LOG_INFO("Certificate chain discovered:");
		for (size_t i = 0; i < chain.size(); i++) {
			LOG_INFO("  " + std::to_string(i) + ": " + getSubjectName(chain[i].get()));
		}

		// The last certificate should be trusted (typically a root)
		X509 *trustedCert = chain.back().get();

		// Verify the chain
		std::unique_ptr<X509_STORE, X509_STORE_Deleter> store(X509_STORE_new());
		if (!store) {
			throw OpenSSLError("Failed to create X509_STORE");
		}

		// Add the trusted certificate (typically a root CA)
		if (X509_STORE_add_cert(store.get(), trustedCert) != 1) {
			throw OpenSSLError("Failed to add trusted certificate to store");
		}

		// Create a certificate stack for the intermediate chain
		std::unique_ptr<STACK_OF(X509), STACK_OF_X509_Deleter> intermediates(
			createCertificateStack(chain, true));

		// Create a verification context
		std::unique_ptr<X509_STORE_CTX, X509_STORE_CTX_Deleter> ctx(X509_STORE_CTX_new());
		if (!ctx) {
			throw OpenSSLError("Failed to create X509_STORE_CTX");
		}

		// Initialize the verification context
		if (X509_STORE_CTX_init(ctx.get(), store.get(), cert, intermediates.get()) != 1) {
			throw OpenSSLError("Failed to initialize X509_STORE_CTX");
		}

		// Perform verification
		int result = X509_verify_cert(ctx.get());

		if (result != 1) {
			int error = X509_STORE_CTX_get_error(ctx.get());
			LOG_ERROR("Certificate verification failed: " +
				  std::string(X509_verify_cert_error_string(error)));
			return false;
		}

		LOG_INFO("Certificate chain verification succeeded");
		return true;
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
						printCertificateDetails(cert.get());
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
						printCertificateDetails(cert);
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
		buildCertificateChain(X509 *cert, const std::vector<X509 *> &certPool, bool verbose = true)
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
						  X509 *rootCA = nullptr, bool verbose = true)
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
		X509_STORE_CTX_set_flags(ctx.get(), X509_V_FLAG_CHECK_SS_SIGNATURE);

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

// SM-DP+ Response Generator
class SMDPResponseGenerator {
    public:
	SMDPResponseGenerator()
	{
		// Initialize OpenSSL
		OpenSSL_add_all_algorithms();
		ERR_load_crypto_strings();
	}

	~SMDPResponseGenerator()
	{
		// Cleanup OpenSSL
		EVP_cleanup();
		CRYPTO_cleanup_all_ex_data();
		ERR_free_strings();
	}

	std::string generateResponse(const std::string &transactionIdHex, const std::string &euiccChallengeHex,
				     const std::string &serverAddress, const std::string &serverChallengeHex,
				     const std::string &privateKeyPath, const std::string &certificatePath)
	{
		try {
			// Create ServerSigned1 structure
			std::unique_ptr<SERVER_SIGNED1, SERVER_SIGNED1_Deleter> ss1(SERVER_SIGNED1_new());

			if (!ss1) {
				throw OpenSSLError("Failed to create ServerSigned1 structure");
			}

			// Set transactionId (16 bytes)
			std::vector<uint8_t> transId = HexUtil::hexToBytes(transactionIdHex);
			ASN1_OCTET_STRING_set(ss1->transactionId, transId.data(), transId.size());

			// Set euiccChallenge (16 bytes)
			std::vector<uint8_t> euicc = HexUtil::hexToBytes(euiccChallengeHex);
			ASN1_OCTET_STRING_set(ss1->euiccChallenge, euicc.data(), euicc.size());

			// Set serverAddress
			ASN1_STRING_set(ss1->serverAddress,
					reinterpret_cast<const unsigned char *>(serverAddress.c_str()),
					serverAddress.length());

			// Set serverChallenge
			std::vector<uint8_t> challenge = HexUtil::hexToBytes(serverChallengeHex);
			ASN1_OCTET_STRING_set(ss1->serverChallenge, challenge.data(), challenge.size());

			// Encode the ServerSigned1 structure to DER
			unsigned char *derData = nullptr;
			int derLen = i2d_SERVER_SIGNED1(ss1.get(), &derData);

			if (derLen <= 0) {
				throw OpenSSLError("Failed to encode ServerSigned1 to DER");
			}

			// Custom deleter for OpenSSL allocated memory
			struct OpenSSLFreeDeleter {
				void operator()(void *p) const
				{
					OPENSSL_free(p);
				}
			};

			// Use unique_ptr with custom deleter for derData
			std::unique_ptr<unsigned char, OpenSSLFreeDeleter> derDataPtr(derData);

			// Write the DER data to a file for debugging
			std::ofstream derFile("serverSigned1.der", std::ios::binary);
			if (derFile) {
				derFile.write(reinterpret_cast<const char *>(derDataPtr.get()), derLen);
			}

			// Load the SM-DP+ private key
			std::ifstream keyFile(privateKeyPath);
			if (!keyFile) {
				throw std::runtime_error("Failed to open private key file: " + privateKeyPath);
			}

			// Create a BIO from the private key file
			std::string keyStr((std::istreambuf_iterator<char>(keyFile)), std::istreambuf_iterator<char>());
			std::unique_ptr<BIO, BIODeleter> keyBio(BIO_new_mem_buf(keyStr.c_str(), -1));

			if (!keyBio) {
				throw OpenSSLError("Failed to create BIO for private key");
			}

			// Read the private key
			std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pkey(
				PEM_read_bio_PrivateKey(keyBio.get(), nullptr, nullptr, nullptr));

			if (!pkey) {
				throw OpenSSLError("Failed to read private key");
			}

			// Create and initialize a signature context
			std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdctx(EVP_MD_CTX_new());

			if (!mdctx) {
				throw OpenSSLError("Failed to create signature context");
			}

			// Initialize the signature operation
			if (EVP_DigestSignInit(mdctx.get(), nullptr, EVP_sha256(), nullptr, pkey.get()) != 1) {
				throw OpenSSLError("Failed to initialize signature operation");
			}

			// Update the signature with the data
			if (EVP_DigestSignUpdate(mdctx.get(), derDataPtr.get(), derLen) != 1) {
				throw OpenSSLError("Failed to update signature");
			}

			// Finalize the signature
			size_t sigLen;
			if (EVP_DigestSignFinal(mdctx.get(), nullptr, &sigLen) != 1) {
				throw OpenSSLError("Failed to determine signature length");
			}

			std::vector<uint8_t> sig(sigLen);

			if (EVP_DigestSignFinal(mdctx.get(), sig.data(), &sigLen) != 1) {
				throw OpenSSLError("Failed to create signature");
			}

			sig.resize(sigLen); // Adjust size to actual signature length

			// Write the signature to a file for debugging
			std::ofstream sigFile("serverSignature1.bin", std::ios::binary);
			if (sigFile) {
				sigFile.write(reinterpret_cast<const char *>(sig.data()), sig.size());
			}

			// Read the SM-DP+ certificate
			std::ifstream certFile(certificatePath);
			if (!certFile) {
				throw std::runtime_error("Failed to open certificate file: " + certificatePath);
			}

			std::string certStr((std::istreambuf_iterator<char>(certFile)),
					    std::istreambuf_iterator<char>());
			std::vector<uint8_t> certData(certStr.begin(), certStr.end());

			// Base64 encode the data for JSON output
			std::vector<uint8_t> derDataVector(derDataPtr.get(), derDataPtr.get() + derLen);
			std::string serverSigned1B64 = Base64::encode(derDataVector);
			std::string serverSignature1B64 = Base64::encode(sig);
			std::string serverCertificateB64 = Base64::encode(certData);

			// Generate a dummy euiccCiPKIdToBeUsed
			std::vector<uint8_t> euiccCiPKId = { 0x04, 0x14, 0xF5, 0x41, 0x72, 0xBD, 0xF9, 0x8A,
							     0x95, 0xD6, 0x5C, 0xBE, 0xB8, 0x8A, 0x38, 0xA1,
							     0xC1, 0x1D, 0x80, 0x0A, 0x85, 0xC3 };
			std::string euiccCiPKIdB64 = Base64::encode(euiccCiPKId);

			// Generate JSON output
			std::stringstream jsonSS;
			jsonSS << "{\n";
			jsonSS << "  \"transactionId\": \"" << transactionIdHex << "\",\n";
			jsonSS << "  \"serverSigned1\": \"" << serverSigned1B64 << "\",\n";
			jsonSS << "  \"serverSignature1\": \"" << serverSignature1B64 << "\",\n";
			jsonSS << "  \"serverCertificate\": \"" << serverCertificateB64 << "\",\n";
			jsonSS << "  \"euiccCiPKIdToBeUsed\": \"" << euiccCiPKIdB64 << "\",\n";
			jsonSS << "  \"header\": {\n";
			jsonSS << "    \"functionExecutionStatus\": {\n";
			jsonSS << "      \"status\": \"Executed-Success\"\n";
			jsonSS << "    }\n";
			jsonSS << "  }\n";
			jsonSS << "}\n";

			return jsonSS.str();

		} catch (const std::exception &e) {
			std::cerr << "Error generating response: " << e.what() << std::endl;
			return "{}"; // Return empty JSON on error
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

	// Set test mode (use mock responses instead of real HTTP requests)
	void setTestMode(bool testMode)
	{
		m_testMode = testMode;
		Logger::info(std::string("Test mode ") + (testMode ? "enabled" : "disabled"));
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
	std::unique_ptr<SMDP_SIGNED2, SMDP_SIGNED2_Deleter> createSmdpSigned2()
	{
		Logger::debug("Creating SMDP_SIGNED2 structure...");

		std::unique_ptr<SMDP_SIGNED2, SMDP_SIGNED2_Deleter> smdpSigned2(SMDP_SIGNED2_new());
		if (!smdpSigned2) {
			throw OpenSSLError("Failed to create SMDP_SIGNED2 structure");
		}

		Logger::debug("SMDP_SIGNED2 structure created successfully");

		// Set transactionId [0] - context-specific tagged
		// For context-tagged fields, the ASN1_OCTET_STRING should be auto-allocated
		if (!smdpSigned2->transactionId) {
			Logger::debug("Allocating transactionId field...");
			smdpSigned2->transactionId = ASN1_OCTET_STRING_new();
			if (!smdpSigned2->transactionId) {
				throw OpenSSLError("Failed to allocate ASN1_OCTET_STRING for transactionId");
			}
		}

		std::vector<uint8_t> transactionIdBin = HexUtil::hexToBytes(m_transactionId);
		Logger::debug("Setting transactionId: " + m_transactionId + " (" +
			      std::to_string(transactionIdBin.size()) + " bytes)");

		if (ASN1_OCTET_STRING_set(smdpSigned2->transactionId, transactionIdBin.data(),
					  transactionIdBin.size()) != 1) {
			throw OpenSSLError("Failed to set transaction ID");
		}

		// Set ccRequiredFlag BOOLEAN (typically false for standard profile downloads)
		// ASN1_BOOLEAN is just an int, not a pointer, so no allocation needed
		Logger::debug("Setting ccRequiredFlag to false");
		smdpSigned2->ccRequiredFlag = 0; // FALSE

		// Set bppEuiccOtpk [APPLICATION 73] OCTET STRING OPTIONAL
		// This is the eUICC's one-time public key (otPK.EUICC.ECKA)

		// OPTION 1: Include the optional public key (if available)
		if (!m_euiccPublicKeyData.empty()) {
			Logger::debug("Setting bppEuiccOtpk (" + std::to_string(m_euiccPublicKeyData.size()) +
				      " bytes)...");

			// IMPORTANT: For optional fields, we need to allocate the ASN1_OCTET_STRING first
			smdpSigned2->bppEuiccOtpk = ASN1_OCTET_STRING_new();
			if (!smdpSigned2->bppEuiccOtpk) {
				throw OpenSSLError("Failed to allocate ASN1_OCTET_STRING for bppEuiccOtpk");
			}

			if (ASN1_OCTET_STRING_set(smdpSigned2->bppEuiccOtpk, m_euiccPublicKeyData.data(),
						  m_euiccPublicKeyData.size()) != 1) {
				throw OpenSSLError("Failed to set eUICC one-time public key");
			}
			Logger::info("Included eUICC one-time public key (" +
				     std::to_string(m_euiccPublicKeyData.size()) + " bytes)");
		} else {
			// OPTION 2: Skip the optional field (safer for testing)
			smdpSigned2->bppEuiccOtpk = nullptr;
			Logger::info("eUICC one-time public key not included (optional field)");
		}

		Logger::debug("SMDP_SIGNED2 structure completed successfully");
		return smdpSigned2;
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


	// Sign SMDP_SIGNED2 with eUICC private key
	std::vector<uint8_t> signSmdpSigned2(SMDP_SIGNED2 *smdpSigned2)
	{
		if (!m_euiccPrivateKey) {
			throw std::runtime_error("eUICC private key not available for signing");
		}

		// Encode SMDP_SIGNED2 to DER
		unsigned char *derData = nullptr;
		int derLen = i2d_SMDP_SIGNED2(smdpSigned2, &derData);
		if (derLen <= 0) {
			throw OpenSSLError("Failed to encode SMDP_SIGNED2 to DER");
		}

		std::unique_ptr<unsigned char, OpenSSLFreeDeleter> derDataPtr(derData);

		// Create signature context
		std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdctx(EVP_MD_CTX_new());
		if (!mdctx) {
			throw OpenSSLError("Failed to create signature context");
		}

		// Initialize signing with SHA256
		if (EVP_DigestSignInit(mdctx.get(), nullptr, EVP_sha256(), nullptr, m_euiccPrivateKey.get()) != 1) {
			throw OpenSSLError("Failed to initialize signature operation");
		}

		// Update with data to be signed
		if (EVP_DigestSignUpdate(mdctx.get(), derDataPtr.get(), derLen) != 1) {
			throw OpenSSLError("Failed to update signature");
		}

		// Get signature length
		size_t sigLen;
		if (EVP_DigestSignFinal(mdctx.get(), nullptr, &sigLen) != 1) {
			throw OpenSSLError("Failed to determine signature length");
		}

		// Create signature
		std::vector<uint8_t> signature(sigLen);
		if (EVP_DigestSignFinal(mdctx.get(), signature.data(), &sigLen) != 1) {
			throw OpenSSLError("Failed to create signature");
		}

		signature.resize(sigLen);
		Logger::info("Created eUICC signature over SMDP_SIGNED2 (" + std::to_string(sigLen) + " bytes)");

		return signature;
	}

	// // Generate one-time public key (simplified - in real implementation this would be more complex)
	// void generateEUICCOtpk()
	// {
	// 	// For demo purposes, generate a dummy OtPK
	// 	// In real implementation, this would involve proper ECKA key generation
	// 	m_euiccOtpk.resize(65); // Uncompressed EC public key (1 + 32 + 32 bytes)
	// 	m_euiccOtpk[0] = 0x04; // Uncompressed point indicator

	// 	if (RAND_bytes(m_euiccOtpk.data() + 1, 64) != 1) {
	// 		throw OpenSSLError("Failed to generate eUICC OtPK");
	// 	}

	// 	Logger::info("Generated eUICC OtPK: " + HexUtil::bytesToHex(m_euiccOtpk));
	// }

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

	// Create ASN.1 EUICCSigned2 structure
	EUICCSigned2Ptr createEUICCSigned2()
	{
		if (m_transactionId.empty()) {
			throw std::runtime_error("Transaction ID not available");
		}

		if (m_euiccOtpk.empty()) {
			generateEUICCOtpk(); // Auto-generate if not set
		}

		// Create EUICCSigned2 structure
		EUICCSigned2Ptr euiccSigned2(EUICC_SIGNED2_new());
		if (!euiccSigned2) {
			throw OpenSSLError("Failed to create EUICC_SIGNED2 structure");
		}

		// Set transactionId [0] - convert from hex string to bytes
		std::vector<uint8_t> transactionIdBytes = HexUtil::hexToBytes(m_transactionId);

		euiccSigned2->transactionId = ASN1_OCTET_STRING_new();
		if (!euiccSigned2->transactionId ||
		    ASN1_OCTET_STRING_set(euiccSigned2->transactionId, transactionIdBytes.data(),
					  transactionIdBytes.size()) != 1) {
			throw OpenSSLError("Failed to set transaction ID in EUICC_SIGNED2");
		}

		// Set euiccOtpk [APPLICATION 73] OCTET STRING (tag 5F49)
		euiccSigned2->euiccOtpk = ASN1_OCTET_STRING_new();
		if (!euiccSigned2->euiccOtpk ||
		    ASN1_OCTET_STRING_set(euiccSigned2->euiccOtpk, m_euiccOtpk.data(), m_euiccOtpk.size()) != 1) {
			throw OpenSSLError("Failed to set eUICC OtPK in EUICC_SIGNED2");
		}

		// Set hashCc (optional)
		if (!m_confirmationCodeHash.empty()) {
			euiccSigned2->hashCc = ASN1_OCTET_STRING_new();
			if (!euiccSigned2->hashCc ||
			    ASN1_OCTET_STRING_set(euiccSigned2->hashCc, m_confirmationCodeHash.data(),
						  m_confirmationCodeHash.size()) != 1) {
				throw OpenSSLError("Failed to set confirmation code hash");
			}
		}

		Logger::info("Created EUICC_SIGNED2 structure");
		return euiccSigned2;
	}

	// Sign EUICCSigned2 to create euiccSignature2
	std::vector<uint8_t> signEUICCSigned2(EUICC_SIGNED2 *euiccSigned2)
	{
		if (!m_euiccPrivateKey) {
			throw std::runtime_error("eUICC private key not loaded. Call loadEUICCPrivateKey() first.");
		}

		try {
			// Encode EUICC_SIGNED2 to DER for signing
			unsigned char *derData = nullptr;
			int derLen = i2d_EUICC_SIGNED2(euiccSigned2, &derData);

			if (derLen <= 0) {
				throw OpenSSLError("Failed to encode EUICC_SIGNED2 to DER");
			}

			// Use smart pointer for derData cleanup
			std::unique_ptr<unsigned char, OpenSSLFreeDeleter> derDataPtr(derData);

			// Create signature context
			std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdCtx(EVP_MD_CTX_new());
			if (!mdCtx) {
				throw OpenSSLError("Failed to create signature context");
			}

			// Initialize signature with SHA256
			if (EVP_DigestSignInit(mdCtx.get(), nullptr, EVP_sha256(), nullptr, m_euiccPrivateKey.get()) !=
			    1) {
				throw OpenSSLError("Failed to initialize signature");
			}

			// Update with DER data
			if (EVP_DigestSignUpdate(mdCtx.get(), derDataPtr.get(), derLen) != 1) {
				throw OpenSSLError("Failed to update signature");
			}

			// Get signature length
			size_t sigLen;
			if (EVP_DigestSignFinal(mdCtx.get(), nullptr, &sigLen) != 1) {
				throw OpenSSLError("Failed to determine signature length");
			}

			// Create signature
			std::vector<uint8_t> signature(sigLen);
			if (EVP_DigestSignFinal(mdCtx.get(), signature.data(), &sigLen) != 1) {
				throw OpenSSLError("Failed to create signature");
			}

			signature.resize(sigLen);
			Logger::info("Created eUICC signature: " + HexUtil::bytesToHex(signature));

			return signature;
		} catch (const std::exception &e) {
			Logger::error("Failed to sign EUICC_SIGNED2: " + std::string(e.what()));
			throw;
		}
	}

	// Create PrepareDownloadResponseOk
	PrepareDownloadResponseOkPtr createPrepareDownloadResponseOk()
	{
		// Create EUICC_SIGNED2
		auto euiccSigned2 = createEUICCSigned2();

		// Sign it to get euiccSignature2
		std::vector<uint8_t> euiccSignature2 = signEUICCSigned2(euiccSigned2.get());

		// Create PrepareDownloadResponseOk structure
		PrepareDownloadResponseOkPtr prepareDownloadResponseOk(PREPARE_DOWNLOAD_RESPONSE_OK_new());
		if (!prepareDownloadResponseOk) {
			throw OpenSSLError("Failed to create PREPARE_DOWNLOAD_RESPONSE_OK structure");
		}

		// Set euiccSigned2 (transfer ownership)
		prepareDownloadResponseOk->euiccSigned2 = euiccSigned2.release();

		// Set euiccSignature2 [APPLICATION 55] OCTET STRING (tag 5F37)
		prepareDownloadResponseOk->euiccSignature2 = ASN1_OCTET_STRING_new();
		if (!prepareDownloadResponseOk->euiccSignature2 ||
		    ASN1_OCTET_STRING_set(prepareDownloadResponseOk->euiccSignature2, euiccSignature2.data(),
					  euiccSignature2.size()) != 1) {
			throw OpenSSLError("Failed to set eUICC signature in PREPARE_DOWNLOAD_RESPONSE_OK");
		}

		Logger::info("Created PREPARE_DOWNLOAD_RESPONSE_OK structure");
		return prepareDownloadResponseOk;
	}

  void debugASN1Structure(const std::vector<uint8_t>& derData) {
    Logger::debug("=== ASN.1 Structure Analysis ===");
    Logger::debug("Total length: " + std::to_string(derData.size()) + " bytes");
    Logger::debug("Hex: " + HexUtil::bytesToHex(derData));

    if (derData.size() < 4) {
        Logger::error("ASN.1 data too short");
        return;
    }

    // Check the outer tag
    if (derData[0] == 0xBF && derData[1] == 0x21) {
        Logger::info(" Found [33] tag (BF21) - PrepareDownloadResponse");

        // Parse length
        size_t lengthBytes = 1;
        size_t contentLength = derData[2];
        if (contentLength & 0x80) {
            lengthBytes = contentLength & 0x7F;
            contentLength = 0;
            for (size_t i = 0; i < lengthBytes; i++) {
                contentLength = (contentLength << 8) | derData[3 + i];
            }
            lengthBytes++; // Include the length-of-length byte
        }

        Logger::info("Content length: " + std::to_string(contentLength) + " bytes");

        // Check if it's a SEQUENCE (PrepareDownloadResponseOk)
        size_t contentStart = 2 + lengthBytes;
        if (contentStart < derData.size() && derData[contentStart] == 0x30) {
            Logger::info(" Found SEQUENCE (30) - PrepareDownloadResponseOk");
        } else {
            Logger::warning("Expected SEQUENCE (30) but got: " +
                          HexUtil::bytesToHex(std::vector<uint8_t>(derData.begin() + contentStart,
                                                                 derData.begin() + contentStart + 4)));
        }
    } else {
        Logger::error("Expected [33] tag (BF21) but got: " +
                     HexUtil::bytesToHex(std::vector<uint8_t>(derData.begin(), derData.begin() + 4)));
    }

    // Try to decode it back to verify
    const unsigned char *p = derData.data();
    PREPARE_DOWNLOAD_RESPONSE *decoded = d2i_PREPARE_DOWNLOAD_RESPONSE(nullptr, &p, derData.size());
    if (decoded) {
        Logger::info(" ASN.1 structure can be decoded successfully");

        // Check the choice type
        Logger::info("Choice type: " + std::to_string(decoded->type));
        if (decoded->type == 0) {
            Logger::info(" Choice is downloadResponseOk");
            if (decoded->d.downloadResponseOk) {
                Logger::info(" PrepareDownloadResponseOk structure exists");
            }
        }

        PREPARE_DOWNLOAD_RESPONSE_free(decoded);
    } else {
        Logger::error("✗ Failed to decode ASN.1 structure");
        ERR_print_errors_fp(stderr);
    }
    Logger::debug("=== End ASN.1 Analysis ===");
}

	// Fixed createGetBoundProfilePackageRequest method
	std::string createGetBoundProfilePackageRequest()
	{
		if (m_transactionId.empty()) {
			throw std::runtime_error("Transaction ID not available. Must authenticate first.");
		}

		try {
			Logger::info("Creating GetBoundProfilePackageRequest with proper ASN.1 structures");

			// Create GetBoundProfilePackageRequest [58] SEQUENCE (tag BF3A)
			// GetBoundProfilePackageRequestPtr getBoundProfilePackageRequest(
			// 	GET_BOUND_PROFILE_PACKAGE_REQUEST_new());
			// if (!getBoundProfilePackageRequest) {
			// 	throw OpenSSLError("Failed to create GET_BOUND_PROFILE_PACKAGE_REQUEST structure");
			// }

			// // Set transactionId [0]
			// std::vector<uint8_t> transactionIdBytes = HexUtil::hexToBytes(m_transactionId);

			// getBoundProfilePackageRequest->transactionId = ASN1_OCTET_STRING_new();
			// if (!getBoundProfilePackageRequest->transactionId ||
			//     ASN1_OCTET_STRING_set(getBoundProfilePackageRequest->transactionId,
			// 			  transactionIdBytes.data(), transactionIdBytes.size()) != 1) {
			// 	throw OpenSSLError("Failed to set transaction ID in GET_BOUND_PROFILE_PACKAGE_REQUEST");
			// }

			// Create PrepareDownloadResponse [33] CHOICE (tag BF21)
			PrepareDownloadResponsePtr prepareDownloadResponse(PREPARE_DOWNLOAD_RESPONSE_new());
			if (!prepareDownloadResponse) {
				throw OpenSSLError("Failed to create PREPARE_DOWNLOAD_RESPONSE structure");
			}

			// Set the choice to downloadResponseOk
			auto prepareDownloadResponseOk = createPrepareDownloadResponseOk();
			prepareDownloadResponse->type = 0; // downloadResponseOk choice
			prepareDownloadResponse->d.downloadResponseOk = prepareDownloadResponseOk.release();

			// Encode to DER
			unsigned char *derData = nullptr;
			int derLen = i2d_PREPARE_DOWNLOAD_RESPONSE(prepareDownloadResponse.get(), &derData);

			if (derLen <= 0) {
				throw OpenSSLError("Failed to encode GET_BOUND_PROFILE_PACKAGE_REQUEST to DER");
			}

			Logger::info("---");
			for (int i = 0; i < derLen; i++) {
				printf("%02x", derData[i]);
			}

			Logger::info("---");

			// Use smart pointer for derData cleanup
			std::unique_ptr<unsigned char, OpenSSLFreeDeleter> derDataPtr(derData);

			// Convert to vector for Base64 encoding
			std::vector<uint8_t> derVector(derDataPtr.get(), derDataPtr.get() + derLen);

        debugASN1Structure(derVector);

			// Base64 encode the ASN.1 structure
			std::string getBoundProfilePackageRequestB64 = Base64::encode(derVector);

			// Create JSON with the proper ASN.1 structure
			std::unique_ptr<cJSON, cJSON_Deleter> json(cJSON_CreateObject());

			cJSON_AddStringToObject(json.get(), "transactionId", m_transactionId.c_str());

			cJSON_AddStringToObject(json.get(), "prepareDownloadResponse",
						getBoundProfilePackageRequestB64.c_str());

			// Serialize JSON to string
			char *jsonStr = cJSON_Print(json.get());
			std::string result(jsonStr);
			free(jsonStr);

			Logger::info("Created GetBoundProfilePackageRequest (" + std::to_string(derLen) +
				     " bytes DER)");
			Logger::debug("ASN.1 hex: " + HexUtil::bytesToHex(derVector));

			return result;

		} catch (const std::exception &e) {
			Logger::error("Failed to create GetBoundProfilePackageRequest: " + std::string(e.what()));
			throw;
		}
	}

	// Process getBoundProfilePackage response
	bool processGetBoundProfilePackageResponse(const std::string &responseJson)
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
			cJSON *headerJson = cJSON_GetObjectItemCaseSensitive(json.get(), "header");
			cJSON *boundProfilePackageJson =
				cJSON_GetObjectItemCaseSensitive(json.get(), "boundProfilePackage");

			// Check for success status
			if (cJSON_IsObject(headerJson)) {
				cJSON *funcStatusJson =
					cJSON_GetObjectItemCaseSensitive(headerJson, "functionExecutionStatus");

				if (cJSON_IsObject(funcStatusJson)) {
					cJSON *statusJson = cJSON_GetObjectItemCaseSensitive(funcStatusJson, "status");

					if (cJSON_IsString(statusJson)) {
						std::string status = statusJson->valuestring;
						Logger::info("Function execution status: " + status);

						if (status != "Executed-Success") {
							// Get the detailed status code and reason
							cJSON *detailsJson = cJSON_GetObjectItemCaseSensitive(
								funcStatusJson, "statusCodeData");

							if (cJSON_IsObject(detailsJson)) {
								cJSON *subjectCodeJson =
									cJSON_GetObjectItemCaseSensitive(detailsJson,
													 "subjectCode");
								cJSON *reasonCodeJson =
									cJSON_GetObjectItemCaseSensitive(detailsJson,
													 "reasonCode");
								cJSON *messageJson = cJSON_GetObjectItemCaseSensitive(
									detailsJson, "message");

								std::string errorDetails = "Error: ";

								if (cJSON_IsString(subjectCodeJson)) {
									errorDetails +=
										"Subject=" +
										std::string(
											subjectCodeJson->valuestring) +
										", ";
								}

								if (cJSON_IsString(reasonCodeJson)) {
									errorDetails +=
										"Reason=" +
										std::string(
											reasonCodeJson->valuestring) +
										", ";
								}

								if (cJSON_IsString(messageJson)) {
									errorDetails +=
										"Message=" +
										std::string(messageJson->valuestring);
								}

								Logger::error(errorDetails);
							}

							return false;
						}
					}
				}
			}

			// Process the boundProfilePackage if present
			if (cJSON_IsString(boundProfilePackageJson)) {
				std::string boundProfilePackage = boundProfilePackageJson->valuestring;
				Logger::info("Received bound profile package (length: " +
					     std::to_string(boundProfilePackage.length()) + ")");

				// Store for later processing
				m_boundProfilePackage = Base64::decode(boundProfilePackage);

				return true;
			} else {
				Logger::error("No boundProfilePackage in response");
				return false;
			}
		} catch (const std::exception &e) {
			Logger::error("Error processing getBoundProfilePackage response: " + std::string(e.what()));
			return false;
		}
	}

	// Get the bound profile package from SM-DP+
	bool getBoundProfilePackage()
	{
		try {
			// Create request
			std::string request = createGetBoundProfilePackageRequest();
			Logger::info("Sending getBoundProfilePackage request:\n" + request);

			// Build endpoint URL
			std::string url = m_serverUrl;

			// Ensure URL has proper prefix
			if (url.find("http://") != 0 && url.find("https://") != 0) {
				url = "https://" + url;
			}

			// Add the API endpoint path
			if (url.back() != '/') {
				url += "/";
			}
			url += "gsma/rsp2/es9plus/getBoundProfilePackage";

			Logger::info("Full request URL: " + url);

			// Create HTTP client
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

			// Send request
			auto response = client.postJsonWithCustomVerification(url, m_serverPort, request, store.get(),
									      certPool);

			// Check for successful HTTP status
			if (response.statusCode >= 200 && response.statusCode < 300) {
				// Process the response
				return processGetBoundProfilePackageResponse(response.body);
			} else {
				LOG_ERROR("HTTP error: " + std::to_string(response.statusCode));
				LOG_ERROR("Response: " + response.body);
				return false;
			}
		} catch (const std::exception &e) {
			LOG_ERROR("Error in getBoundProfilePackage: " + std::string(e.what()));
			return false;
		}
	}

	// // Create EUICC_INFO1 structure
	// std::unique_ptr<EUICC_INFO1, EUICC_INFO1_Deleter>
	// createEuiccInfo1(const std::vector<uint8_t> &svn,
	//                  const std::vector<std::vector<uint8_t>> &verificationPKIDs,
	//                  const std::vector<std::vector<uint8_t>> &signingPKIDs) {

	//   std::unique_ptr<EUICC_INFO1, EUICC_INFO1_Deleter> info(EUICC_INFO1_new());
	//   if (!info) {
	//     throw OpenSSLError("Failed to create EUICC_INFO1 structure");
	//   }

	//   // Set SVN (version)
	//   ASN1_OCTET_STRING_set(info->svn, svn.data(), svn.size());

	//   // Set verification PKIDs
	//   for (const auto &pkid : verificationPKIDs) {
	//     ASN1_OCTET_STRING *item = ASN1_OCTET_STRING_new();
	//     if (!item) {
	//       throw OpenSSLError(
	//           "Failed to create ASN1_OCTET_STRING for verification PKID");
	//     }

	//     ASN1_OCTET_STRING_set(item, pkid.data(), pkid.size());
	//     sk_ASN1_OCTET_STRING_push(info->euiccCiPKIdListForVerification, item);
	//   }

	//   // Set signing PKIDs
	//   for (const auto &pkid : signingPKIDs) {
	//     ASN1_OCTET_STRING *item = ASN1_OCTET_STRING_new();
	//     if (!item) {
	//       throw OpenSSLError(
	//           "Failed to create ASN1_OCTET_STRING for signing PKID");
	//     }

	//     ASN1_OCTET_STRING_set(item, pkid.data(), pkid.size());
	//     sk_ASN1_OCTET_STRING_push(info->euiccCiPKIdListForSigning, item);
	//   }

	//   return info;
	// }

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

			Logger::info("----------- using this cert to verify sig: -----------");
			CertificateUtil::printCertificateDetails(scertdata);

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
		// Check if we're in test mode (use the existing mock response generator)
		if (m_testMode) {
			// For testing, generate a valid response using the mock generator
			RspCrypto::SMDPResponseGenerator generator;
			return generator.generateResponse(
				"F57D9ED494814FBDAED374800610DEAC", // Transaction ID
				HexUtil::bytesToHex(m_euiccChallenge), // eUICC challenge (from request)
				m_serverUrl, // Server address
				"1122334455667788", // Server challenge
				"demoCA/smdp/smdp.key", "demoCA/smdp/smdp.pem");
		}

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

	bool m_testMode = false; // Flag for test mode
	std::string m_caCertPath = ""; // Path to system CA certificates
	std::vector<uint8_t> m_boundProfilePackage; // Bound profile package from SM-DP+
};

// SM-DP+ Response Validator
class SMDPResponseValidator {
    public:
	SMDPResponseValidator()
	{
		// Initialize OpenSSL
		OpenSSL_add_all_algorithms();
		ERR_load_crypto_strings();
	}

	~SMDPResponseValidator()
	{
		// Cleanup OpenSSL
		EVP_cleanup();
		CRYPTO_cleanup_all_ex_data();
		ERR_free_strings();
	}

	enum class ValidationResult {
		Success, // Signature verification successful
		InvalidSignature, // Signature verification failed
		Error // Error during validation process
	};

	ValidationResult validate(const std::string &jsonString)
	{
		try {
			// Parse JSON
			std::unique_ptr<cJSON, cJSON_Deleter> json(cJSON_Parse(jsonString.c_str()));

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
			cJSON *headerJson = cJSON_GetObjectItemCaseSensitive(json.get(), "header");

			// Validate required fields exist
			if (!cJSON_IsString(serverSigned1Json) || !cJSON_IsString(serverSignature1Json) ||
			    !cJSON_IsString(transactionIdJson) || !cJSON_IsString(serverCertificateJson)) {
				throw std::runtime_error("Missing or invalid required fields in JSON");
			}

			std::cout << "TransactionID: " << transactionIdJson->valuestring << std::endl;

			// Check function execution status if header exists
			if (cJSON_IsObject(headerJson)) {
				cJSON *funcStatusJson =
					cJSON_GetObjectItemCaseSensitive(headerJson, "functionExecutionStatus");
				if (cJSON_IsObject(funcStatusJson)) {
					cJSON *statusJson = cJSON_GetObjectItemCaseSensitive(funcStatusJson, "status");
					if (cJSON_IsString(statusJson)) {
						std::cout << "Function Execution Status: " << statusJson->valuestring
							  << std::endl;
						if (std::string(statusJson->valuestring) != "Executed-Success") {
							std::cerr
								<< "Warning: Function execution status is not successful"
								<< std::endl;
						}
					}
				}
			}

			// Decode Base64 values
			std::vector<uint8_t> serverSigned1 = Base64::decode(serverSigned1Json->valuestring);
			std::vector<uint8_t> serverSignature1 = Base64::decode(serverSignature1Json->valuestring);
			std::vector<uint8_t> serverCertificate = Base64::decode(serverCertificateJson->valuestring);

			// Parse ServerSigned1 ASN.1 Structure
			const unsigned char *p = serverSigned1.data();
			std::unique_ptr<SERVER_SIGNED1, SERVER_SIGNED1_Deleter> ss1(
				d2i_SERVER_SIGNED1(nullptr, &p, serverSigned1.size()));

			if (!ss1) {
				throw OpenSSLError("Failed to parse ServerSigned1 ASN.1 structure");
			}

			// Convert transaction ID from hex to binary for comparison
			std::vector<uint8_t> transactionIdBin = HexUtil::hexToBytes(transactionIdJson->valuestring);

			// Validate Transaction ID Match
			if (transactionIdBin.size() != 16 || ASN1_STRING_length(ss1->transactionId) != 16 ||
			    memcmp(ASN1_STRING_get0_data(ss1->transactionId), transactionIdBin.data(), 16) != 0) {
				std::cerr << "Transaction ID mismatch" << std::endl;
				return ValidationResult::InvalidSignature;
			}

			// Handle special prefix in serverSignature1 based on SGP.22 spec
			const unsigned char *signatureData = serverSignature1.data();
			size_t signatureLen = serverSignature1.size();

			if (signatureLen >= 3 && signatureData[0] == 0x5f && signatureData[1] == 0x37 &&
			    signatureData[2] == 0x40) {
				std::cout << "Detected 5F3740 prefix in serverSignature1, skipping for "
					     "verification"
					  << std::endl;
				signatureData += 3;
				signatureLen -= 3;
			}

			// Parse server certificate
			std::unique_ptr<BIO, BIODeleter> bio(
				BIO_new_mem_buf(serverCertificate.data(), serverCertificate.size()));

			std::unique_ptr<X509, X509Deleter> cert(
				PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr));

			if (!cert) {
				throw OpenSSLError("Failed to parse server certificate");
			}

			// Extract certificate information
			char subject[256];
			X509_NAME_oneline(X509_get_subject_name(cert.get()), subject, sizeof(subject));
			std::cout << "Certificate Subject: " << subject << std::endl;

			// Extract public key from certificate
			std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pubkey(X509_get_pubkey(cert.get()));

			if (!pubkey) {
				throw OpenSSLError("Failed to extract public key from certificate");
			}

			// Create verification context
			std::unique_ptr<EVP_MD_CTX, EVP_MD_CTX_Deleter> mdCtx(EVP_MD_CTX_new());

			if (!mdCtx) {
				throw OpenSSLError("Failed to create message digest context");
			}

			// Initialize verification with SHA256
			if (EVP_DigestVerifyInit(mdCtx.get(), nullptr, EVP_sha256(), nullptr, pubkey.get()) != 1) {
				throw OpenSSLError("Failed to initialize verification");
			}

			// Process the message (serverSigned1)
			if (EVP_DigestVerifyUpdate(mdCtx.get(), serverSigned1.data(), serverSigned1.size()) != 1) {
				throw OpenSSLError("Failed to update verification");
			}

			// Verify signature
			int verifyResult = EVP_DigestVerifyFinal(mdCtx.get(), signatureData, signatureLen);

			if (verifyResult == 1) {
				std::cout << "Signature verification successful!" << std::endl;

				// Print serverSigned1 content (hexdump)
				std::cout << "ServerSigned1 content (hexdump):" << std::endl;
				for (size_t i = 0; i < serverSigned1.size(); i++) {
					printf("%02X", serverSigned1[i]);
					if ((i + 1) % 16 == 0)
						printf("\n");
					else if ((i + 1) % 8 == 0)
						printf("  ");
					else
						printf(" ");
				}
				std::cout << std::endl;

				return ValidationResult::Success;
			} else if (verifyResult == 0) {
				std::cerr << "Signature verification failed - invalid signature" << std::endl;
				return ValidationResult::InvalidSignature;
			} else {
				throw OpenSSLError("Signature verification error");
			}
		} catch (const std::exception &e) {
			std::cerr << "Error: " << e.what() << std::endl;
			return ValidationResult::Error;
		}
	}
};

} // namespace RspCrypto

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

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
		client.setTestMode(testMode);

		// Perform initiateAuthentication
		RspCrypto::LOG_INFO("Initiating authentication with SM-DP+ server...");
		if (client.initiateAuthentication()) {
			RspCrypto::LOG_INFO("Authentication successful!");

			// Get bound profile package (this would be the next step in the RSP flow)
			RspCrypto::LOG_INFO("Retrieving bound profile package...");
			if (client.getBoundProfilePackage()) {
				RspCrypto::LOG_INFO("Successfully retrieved bound profile package");
			} else {
				RspCrypto::LOG_ERROR("Failed to retrieve bound profile package");
				return 1;
			}

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

// find Variants\ A_B_C/Variant\ C/ -iname "*CERT*" -exec cp {} cert_c \;
// find Variants\ A_B_C/CI -iname "*CERT*" -exec cp {} cert_c \;
// find Variants\ A_B_C/CI_subCA -iname "*CERT*" -exec cp {} cert_c \;

// openssl verify -trusted cert_c/CERT_CI_SIG_NIST.pem -untrusted cert_c/CERT_CISubCA_SIG_NIST.pem $(for file in $(find cert_c  -type f -not -iname "_CI_" -and -iname "*NIST*" -and -not -iname "*euicc*"); do echo " -untrusted $file"; done) cert_c/CERT_EUICC_VARC_SIG_NIST.der
// openssl verify -partial_chain -trusted cert_c/CERT_EUM_VARC_SIG_NIST.der -untrusted cert_c/CERT_EUMSubCA_VARC_SIG_NIST.der cert_c/CERT_EUICC_VARC_SIG_NIST.der
// openssl x509 -in cert_c/CERT_EUM_VARC_SIG_NIST.der -inform DER -noout -text -certopt no_pubkey -certopt no_sigdump -certopt no_header

// find sgp26 -iname "CERT*NIST*der" | xargs -ti  openssl x509 -in {} -inform DER -noout -ext subjectKeyIdentifier,authorityKeyIdentifier
// openssl x509 -in sgp26/DPpb/CERT_S_SM_DP2pb_ECDSA_NIST.der -inform DER -noout -ext subjectKeyIdentifier,authorityKeyIdentifier
// X509v3 Authority Key Identifier:
//     F5:41:72:BD:F9:8A:95:D6:5C:BE:B8:8A:38:A1:C1:1D:80:0A:85:C3
// X509v3 Subject Key Identifier:
//     20:A3:A8:30:E9:2E:E7:A4:68:C5:EB:27:BA:8D:F1:84:59:AD:FD:D7
// openssl x509 -in sgp26/DPpb/CERT_S_SM_DPpb_ECDSA_NIST.der -inform DER -noout -ext subjectKeyIdentifier,authorityKeyIdentifier
// X509v3 Authority Key Identifier:
//     F5:41:72:BD:F9:8A:95:D6:5C:BE:B8:8A:38:A1:C1:1D:80:0A:85:C3
// X509v3 Subject Key Identifier:
//     E6:EA:F7:1E:E0:FB:94:30:EC:CD:1E:BB:42:1F:88:14:37:C1:32:63
// openssl x509 -in sgp26/eUICC/CERT_EUICC_ECDSA_NIST.der -inform DER -noout -ext subjectKeyIdentifier,authorityKeyIdentifier
// X509v3 Authority Key Identifier:
//     DD:3D:A2:4D:35:0C:1C:C5:D0:AF:09:65:F4:0E:C3:4C:5E:E4:09:F1
// X509v3 Subject Key Identifier:
//     A5:24:76:AF:5D:50:AA:37:64:37:CC:B1:DA:21:72:EF:45:F4:84:F0
// openssl x509 -in sgp26/DPtls/CERT_S_SM_DP_TLS_NIST.der -inform DER -noout -ext subjectKeyIdentifier,authorityKeyIdentifier
// X509v3 Subject Key Identifier:
//     27:FE:F1:F2:29:18:7E:C7:83:ED:F6:E0:29:64:A4:51:8D:57:D4:A9
// X509v3 Authority Key Identifier:
//     F5:41:72:BD:F9:8A:95:D6:5C:BE:B8:8A:38:A1:C1:1D:80:0A:85:C3
// openssl x509 -in sgp26/DPauth/CERT_S_SM_DPauth_ECDSA_NIST.der -inform DER -noout -ext subjectKeyIdentifier,authorityKeyIdentifier
// X509v3 Authority Key Identifier:
//     F5:41:72:BD:F9:8A:95:D6:5C:BE:B8:8A:38:A1:C1:1D:80:0A:85:C3
// X509v3 Subject Key Identifier:
//     BD:5A:82:CC:1A:96:60:21:18:BA:75:60:A1:FF:83:A7:8B:21:0B:E5
// openssl x509 -in sgp26/DPauth/CERT_S_SM_DP2auth_ECDSA_NIST.der -inform DER -noout -ext subjectKeyIdentifier,authorityKeyIdentifier
// X509v3 Authority Key Identifier:
//     F5:41:72:BD:F9:8A:95:D6:5C:BE:B8:8A:38:A1:C1:1D:80:0A:85:C3
// X509v3 Subject Key Identifier:
//     95:9E:F7:E6:50:C1:BE:21:6A:39:19:74:27:6D:26:B8:A9:35:61:71
// openssl x509 -in sgp26/CertificateIssuer/CERT_CI_ECDSA_NIST.der -inform DER -noout -ext subjectKeyIdentifier,authorityKeyIdentifier
// X509v3 Subject Key Identifier:
//     F5:41:72:BD:F9:8A:95:D6:5C:BE:B8:8A:38:A1:C1:1D:80:0A:85:C3

// // # find . -type f -iname "*CERT*" -and -iname "*NIST*" -and -iname "*pem" -exec cp {} cert \;
// // ➜  ~/work/smdp/sgp26/_SGP.26 v3.0.2 Files$ openssl s_client -showcerts -connect localhost:8000 < /dev/null > cert_chain.pem
// // ➜  ~/work/smdp/sgp26/_SGP.26 v3.0.2 Files$ openssl verify -CAfile cert/CERT_CI_SIG_NIST.pem $(for file in $(find cert  -type f -not -iname "_CI_"); do echo " -untrusted $file"; done) cert_chain.pem
// // cert_chain.pem: OK

// // openssl x509 -in cert_c/CERT_EUICC_VARC_SIG_NIST.der -inform DER -noout -text -certopt no_pubkey -certopt no_sigdump -certopt no_header
// // openssl x509 -in cert_c/CERT_EUMSubCA_VARC_SIG_NIST.der -inform DER -noout -text -certopt no_pubkey -certopt no_sigdump -certopt no_header
// // openssl x509 -in cert_c/CERT_EUM_VARC_SIG_NIST.der -inform DER -noout -text -certopt no_pubkey -certopt no_sigdump -certopt no_header
// // openssl x509 -in cert_c/CERT_CISubCA_SIG_NIST.der -inform DER -noout -text -certopt no_pubkey -certopt no_sigdump -certopt no_header
// // openssl x509 -in cert_c/CERT_CI_SIG_NIST.der -inform DER -noout -text -certopt no_pubkey -certopt no_sigdump -certopt no_header

//         Version: 3 (0x2)
//         Serial Number:
//             89:04:90:32:12:34:51:23:45:12:34:56:78:90:12:35
//         Signature Algorithm: ecdsa-with-SHA256
//         Issuer: C = ES, O = RSP Test EUM, CN = EUM Test
//         Validity
//             Not Before: Jul 16 10:00:47 2024 GMT
//             Not After : May 10 10:00:47 7500 GMT
//         Subject: C = ES, O = RSP Test EUM, serialNumber = 89049032123451234512345678901235, CN = Test eUICC
//         X509v3 extensions:
//             X509v3 Authority Key Identifier:
//                 57:F5:DD:0D:56:D4:9C:5A:38:5A:7A:3E:4E:E0:A0:AC:ED:C9:02:4A
//             X509v3 Subject Key Identifier:
//                 53:03:0B:DF:03:41:0C:1B:99:01:F8:1C:5E:37:AB:20:ED:52:A8:57
//             X509v3 Key Usage: critical
//                 Digital Signature
//             X509v3 Certificate Policies: critical
//                 Policy: 2.23.146.1.2.1.0.0.0.0.0
//         Version: 3 (0x2)
//         Serial Number: 289 (0x121)
//         Signature Algorithm: ecdsa-with-SHA256
//         Issuer: C = ES, O = RSP Test EUM, CN = EUM Test
//         Validity
//             Not Before: Jul 16 09:59:41 2024 GMT
//             Not After : Jul  8 09:59:41 2058 GMT
//         Subject: C = ES, O = RSP Test EUM, CN = EUM Test
//         X509v3 extensions:
//             X509v3 Authority Key Identifier:
//                 A5:3E:A2:29:D2:19:F7:AE:4B:D7:FC:D6:B0:67:95:B3:6D:D4:53:62
//             X509v3 Subject Key Identifier:
//                 57:F5:DD:0D:56:D4:9C:5A:38:5A:7A:3E:4E:E0:A0:AC:ED:C9:02:4A
//             X509v3 Key Usage: critical
//                 Certificate Sign
//             X509v3 Certificate Policies:
//                 Policy: 2.23.146.1.2.1.0.0.0.0
//             X509v3 Subject Alternative Name:
//                 Registered ID:2.999.101
//             X509v3 Basic Constraints: critical
//                 CA:TRUE, pathlen:0
//             X509v3 CRL Distribution Points:
//                 Full Name:
//                   URI:http://eum.test.example.com/CRL.crl
//         Version: 3 (0x2)
//         Serial Number: 257 (0x101)
//         Signature Algorithm: ecdsa-with-SHA256
//         Issuer: CN = Test CI SubCA, OU = TESTCERT, O = RSPTEST, C = ES
//         Validity
//             Not Before: Jul 16 09:58:33 2024 GMT
//             Not After : Jul  8 09:58:33 2058 GMT
//         Subject: C = ES, O = RSP Test EUM, CN = EUM Test
//         X509v3 extensions:
//             X509v3 Authority Key Identifier:
//                 EE:1F:71:78:B7:0C:23:82:6C:DC:EA:7B:D2:8C:EA:B1:87:4F:55:2D
//             X509v3 Subject Key Identifier:
//                 A5:3E:A2:29:D2:19:F7:AE:4B:D7:FC:D6:B0:67:95:B3:6D:D4:53:62
//             X509v3 Key Usage: critical
//                 Certificate Sign, CRL Sign
//             X509v3 Certificate Policies: critical
//                 Policy: 2.23.146.1.2.1.0.0.0
//             X509v3 Subject Alternative Name:
//                 Registered ID:2.999.101
//             X509v3 Basic Constraints: critical
//                 CA:TRUE, pathlen:1
//             X509v3 CRL Distribution Points:
//                 Full Name:
//                   URI:http://ci.test.example.com/CRL-2.crl
//             X509v3 Name Constraints: critical
//                 Permitted:
//                   DirName:O = RSP Test EUM
//             2.23.146.1.2.2.0:
//                 0
// ..89049032
//         Version: 3 (0x2)
//         Serial Number: 513 (0x201)
//         Signature Algorithm: ecdsa-with-SHA256
//         Issuer: CN = Test CI, OU = TESTCERT, O = RSPTEST, C = IT
//         Validity
//             Not Before: Jun 26 07:18:58 2024 GMT
//             Not After : Jun 26 07:18:58 2059 GMT
//         Subject: CN = Test CI SubCA, OU = TESTCERT, O = RSPTEST, C = ES
//         X509v3 extensions:
//             X509v3 Authority Key Identifier:
//                 9D:E6:4F:BA:14:BE:57:EF:D1:C0:D8:1B:0B:3C:DC:80:63:09:99:9A
//             X509v3 Subject Key Identifier:
//                 EE:1F:71:78:B7:0C:23:82:6C:DC:EA:7B:D2:8C:EA:B1:87:4F:55:2D
//             X509v3 Key Usage: critical
//                 Certificate Sign, CRL Sign
//             X509v3 Certificate Policies: critical
//                 Policy: 2.23.146.1.2.1.0.0
//             X509v3 Basic Constraints: critical
//                 CA:TRUE
//             X509v3 Subject Alternative Name:
//                 Registered ID:2.999.1
//             X509v3 CRL Distribution Points:
//                 Full Name:
//                   URI:http://ci.test.example.com/CRL-1.crl
//                 Full Name:
//                   URI:http://ci.test.example.com/CRL-2.crl
//         Version: 3 (0x2)
//         Serial Number: 0 (0x0)
//         Signature Algorithm: ecdsa-with-SHA256
//         Issuer: CN = Test CI, OU = TESTCERT, O = RSPTEST, C = IT
//         Validity
//             Not Before: Jun 26 07:17:37 2024 GMT
//             Not After : Jun 26 07:17:37 2059 GMT
//         Subject: CN = Test CI, OU = TESTCERT, O = RSPTEST, C = IT
//         X509v3 extensions:
//             X509v3 Subject Key Identifier:
//                 9D:E6:4F:BA:14:BE:57:EF:D1:C0:D8:1B:0B:3C:DC:80:63:09:99:9A
//             X509v3 Basic Constraints: critical
//                 CA:TRUE
//             X509v3 Certificate Policies: critical
//                 Policy: 2.23.146.1.2.1.0
//             X509v3 Key Usage: critical
//                 Certificate Sign, CRL Sign
//             X509v3 Subject Alternative Name:
//                 Registered ID:2.999.1
// (.venv) ➜  ~/work/smdp/test-native git:(master) ✗

// openssl s_client -showcerts -connect localhost:8000 < /dev/null > cert_chain.pem
// openssl verify -verbose -trusted sgp26/CertificateIssuer/CERT_CI_ECDSA_NIST.der $(for file in $(find sgp26  -type f -not -iname "_CI_" -and -iname "*NIST*" -and -iname "*CERT*"); do echo " -untrusted $file"; done) cert_chain.pem

// // ex for proper tags

// // asn1 struct with tag
// // EUICCInfo1 ::= [32] SEQUENCE { -- Tag 'BF20'
// //    svn [2] VersionType,    -- GSMA SGP.22 version supported (SVN)
// //    euiccCiPKIdListForVerification [9] SEQUENCE OF SubjectKeyIdentifier, -- List of CI Public Key Identifiers supported on the eUICC for signature verification
// //    euiccCiPKIdListForSigning [10] SEQUENCE OF SubjectKeyIdentifier -- List of CI Public Key Identifier supported on the eUICC for signature creation
// // }

// typedef struct EUICC_INFO1_INNER_st EUICC_INFO1_INNER;

// /* EUICC_INFO1_INNER structure */
// struct EUICC_INFO1_INNER_st {
//     VersionType *svn;
//     STACK_OF(ASN1_OCTET_STRING) *euiccCiPKIdListForVerification;
//     STACK_OF(ASN1_OCTET_STRING) *euiccCiPKIdListForSigning;
// };
// DECLARE_ASN1_FUNCTIONS(EUICC_INFO1_INNER)

// /* EUICCInfo1 with [32] tag - Tag 'BF20' */
// typedef EUICC_INFO1_INNER EUICC_INFO1;
// DECLARE_ASN1_FUNCTIONS(EUICC_INFO1)

// /* EUICC_INFO1 */
// ASN1_SEQUENCE(EUICC_INFO1_INNER) = {
//     ASN1_IMP(EUICC_INFO1_INNER, svn, VersionType, 2),
//     ASN1_IMP_SEQUENCE_OF(EUICC_INFO1_INNER, euiccCiPKIdListForVerification, ASN1_OCTET_STRING, 9),
//     ASN1_IMP_SEQUENCE_OF(EUICC_INFO1_INNER, euiccCiPKIdListForSigning, ASN1_OCTET_STRING, 10)
// } ASN1_SEQUENCE_END(EUICC_INFO1_INNER)
// IMPLEMENT_ASN1_FUNCTIONS(EUICC_INFO1_INNER)

// ASN1_ITEM_TEMPLATE(EUICC_INFO1) =
//     ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG  | ASN1_TFLG_CONTEXT, 32, EUICC_INFO1, EUICC_INFO1_INNER)
// ASN1_ITEM_TEMPLATE_END(EUICC_INFO1)
// IMPLEMENT_ASN1_FUNCTIONS(EUICC_INFO1)
