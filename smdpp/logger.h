
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
}

namespace RspCrypto {
class SMDPResponseGenerator;
class SMDPResponseValidator;

class Logger {
    public:
	enum class Level { DEBUG, INFO, WARNING, ERROR };

	static void log(Level level, const std::string& message, const char* filename = nullptr, int line = 0) {
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
		ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " << std::setw(7) << std::left << levelStr;

		// Add filename and line information if provided
		if (filename) {
			// Extract just the base filename without the full path
			const char* base_filename = strrchr(filename, '/');
			if (!base_filename) {
				base_filename = strrchr(filename, '\\'); // For Windows paths
			}
			base_filename = base_filename ? base_filename + 1 : filename;

			ss << " [" << base_filename << ":" << line << "]";
		}

		ss << " " << message;

		std::cout << ss.str() << std::endl;
	}

	static void debug(const std::string& message, const char* filename = nullptr, int line = 0) {
		log(Level::DEBUG, message, filename, line);
	}

	static void info(const std::string& message, const char* filename = nullptr, int line = 0) {
		log(Level::INFO, message, filename, line);
	}

	static void warning(const std::string& message, const char* filename = nullptr, int line = 0) {
		log(Level::WARNING, message, filename, line);
	}

	static void error(const std::string& message, const char* filename = nullptr, int line = 0) {
		log(Level::ERROR, message, filename, line);
	}
};

// Define macros to automatically include file and line information
#define LOG_DEBUG(message) Logger::debug(message, __FILE__, __LINE__)
#define LOG_INFO(message) Logger::info(message, __FILE__, __LINE__)
#define LOG_WARNING(message) Logger::warning(message, __FILE__, __LINE__)
#define LOG_ERROR(message) Logger::error(message, __FILE__, __LINE__)

class OpenSSLError : public std::runtime_error {
    public:
	OpenSSLError(const std::string& message) : std::runtime_error(message + getOpenSSLErrors()) {
	}

    private:
	static std::string getOpenSSLErrors() {
		std::stringstream ss;
		unsigned long err;

		ss << " - OpenSSL Errors: ";
		while ((err = ERR_get_error()) != 0) {
			char* err_str = ERR_error_string(err, nullptr);
			ss << err_str << "; ";
		}

		return ss.str();
	}
};

class OpenSSLErrorHandler {
    public:
	// Get detailed error information from OpenSSL error queue
	static std::string getLastError() {
		std::stringstream errorStream;
		unsigned long errorCode;

		while ((errorCode = ERR_get_error()) != 0) {
			char errorBuffer[256];
			ERR_error_string_n(errorCode, errorBuffer, sizeof(errorBuffer));

			const char* library = ERR_lib_error_string(errorCode);

#if OPENSSL_VERSION_NUMBER < 0x30000000
			const char* function = ERR_func_error_string(errorCode);
#endif
			const char* reason = ERR_reason_error_string(errorCode);

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

	static std::string getSSLError(SSL* ssl, int result) {
		int sslError = SSL_get_error(ssl, result);
		std::stringstream errorStream;

		errorStream << "SSL Error Details:\n";
		errorStream << "  Return code: " << result << "\n";
		errorStream << "  SSL error code: " << sslError << "\n";
		errorStream << "  SSL error type: " << getSSLErrorType(sslError) << "\n";

		std::string queueErrors = getLastError();
		if (!queueErrors.empty()) {
			errorStream << "\nError Queue:\n" << queueErrors;
		}

		return errorStream.str();
	}

	static std::string getCertificateError(long verifyResult) {
		std::stringstream errorStream;

		if (verifyResult != X509_V_OK) {
			const char* errorString = X509_verify_cert_error_string(verifyResult);
			errorStream << "Certificate Verification Error:\n";
			errorStream << "  Code: " << verifyResult << "\n";
			errorStream << "  Description: " << (errorString ? errorString : "unknown") << "\n";
		}

		return errorStream.str();
	}

	static void printSSLErrors(const std::string& context = "") {
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

    private:
	static std::string getSSLErrorType(int sslError) {
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

} // namespace RspCrypto