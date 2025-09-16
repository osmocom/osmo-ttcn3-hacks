/* HTTP Client Internal Header
 *
 * Author: Eric Wild <ewild@sysmocom.de> / sysmocom - s.f.m.c. GmbH
 *
 * Released under the terms of GNU General Public License, Version 2 or
 * (at your option) any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <vector>
#include <openssl/x509.h>

namespace RspCrypto {

class HttpClient {
    public:
	HttpClient();
	~HttpClient();

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
	static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
	static size_t headerCallback(void* contents, size_t size, size_t nmemb, void* userp);

	struct SslCtxData {
		X509_STORE* store;
		std::vector<X509*>* certPool;
		bool verifyResult;
		std::string errorMessage;
	};

	static std::string get_cn_name(X509_NAME* const name);
	static CURLcode sslCtxFunction(CURL* curl, SSL_CTX* sslCtx, void* arg);
};

} // namespace RspCrypto

#endif // HTTP_CLIENT_H