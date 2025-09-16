/* RSP Cryptographic Operations Header
 *
 * Author: Eric Wild <ewild@sysmocom.de> / sysmocom - s.f.m.c. GmbH
 *
 * Released under the terms of GNU General Public License, Version 2 or
 * (at your option) any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef RSP_CLIENT_H
#define RSP_CLIENT_H

#include <memory>
#include <string>
#include <vector>
#include "helpers.h" // For deleter functors

// Forward declarations for OpenSSL types
typedef struct x509_st X509;
typedef struct evp_pkey_st EVP_PKEY;
typedef struct x509_store_st X509_STORE;

namespace RspCrypto {

// Forward declarations for internal types
class HttpClient;

/**
 * RSPClient - Remote SIM Provisioning Client
 *
 * This class implements the cryptographic operations and certificate management
 * required for RSP (Remote SIM Provisioning) protocol operations.
 */
class RSPClient {
    public:
	/**
     * Constructor
     * @param serverUrl The server URL
     * @param serverPort The server port
     * @param certPath Vector of certificate paths (files or directories)
     * @param name_filters Vector of name filters for certificate selection
     */
	RSPClient(const std::string& serverUrl, const unsigned int serverPort, const std::vector<std::string>& certPath,
		  const std::vector<std::string>& name_filters = {});

	/**
     * Destructor
     */
	~RSPClient();

	// Certificate and key management
	void setCACertPath(const std::string& path);
	void loadEUICCCertificate(const std::string& euiccCertPath);
	void loadEUICCKeyPair(const std::string& euiccPrivateKeyPath, const std::string& euiccPublicKeyPath = "");
	void loadEUMCertificate(const std::string& eumCertPath);
	void loadEUMKeyPair(const std::string& eumPrivateKeyPath, const std::string& eumPublicKeyPath = "");

	// Certificate retrieval
	std::vector<uint8_t> getEUMCertificate();
	std::vector<uint8_t> getEUICCCertificate();
	std::vector<uint8_t> getCICertificate();

	// Cryptographic operations
	std::vector<uint8_t> generateChallenge();
	void generateEUICCOtpk();
	std::vector<uint8_t> getEUICCOtpk();
	std::vector<uint8_t> signDataWithEUICC(const std::vector<uint8_t>& dataToSign);
	bool verifyServerSignature(const std::vector<uint8_t>& serverSigned1, const std::vector<uint8_t>& signature);
	bool verifyServerSignature(const std::vector<uint8_t>& serverSigned1, const std::vector<uint8_t>& serverSignature1,
				   const std::vector<uint8_t>& derDataServerCert);
	std::vector<uint8_t> computeECDHSharedSecret(const std::vector<uint8_t>& otherPublicKey);

	// Confirmation code and transaction handling
	void setConfirmationCode(const std::string& confirmationCode);
	void setTransactionId(const std::vector<uint8_t>& transactionId);
	std::vector<uint8_t> getConfirmationCodeHash() const;
	void setConfirmationCodeHash(const std::vector<uint8_t>& hash);

	// HTTP client configuration
	void configureHttpClient(bool useCustomTlsCert, const std::string& customTlsCertPath = "");
	void setAuthParams(bool useMutualTLS, const std::string& clientCertPath, const std::string& clientKeyPath);

	// HTTP operations
	std::string sendHttpsPost(const std::string& endpoint, const std::string& body, int& httpStatusCode, unsigned int portOverride);
	std::string sendHttpsPostWithAuth(const std::string& endpoint, const std::string& body, int& httpStatusCode, unsigned int portOverride);
	std::string sendHttpsPostWithContentType(const std::string& endpoint, const std::string& body, const std::string& contentType, int& httpStatusCode,
						 unsigned int portOverride);
	std::string sendHttpsPostUnified(const std::string& endpoint, const std::string& body, int& httpStatusCode, unsigned int portOverride,
					 bool useMutualTLS = false, const std::string& clientCertPath = "", const std::string& clientKeyPath = "",
					 const std::string& contentType = "application/json");

    private:
	// Private helper methods
	void loadCertificate(const std::string& certPath, const std::string& certType, std::unique_ptr<X509, X509Deleter>& certStorage);
	void loadKeyPair(const std::string& privateKeyPath, const std::string& publicKeyPath, const std::string& keyType,
			 std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter>& privateKeyStorage, std::vector<uint8_t>& publicKeyStorage);
	std::vector<uint8_t> signDataWithKey(const std::vector<uint8_t>& dataToSign, EVP_PKEY* pkey);
	bool verifyServerSignature(const std::vector<uint8_t>& signedData, const std::vector<uint8_t>& signature, X509* serverCert);
	bool verifyServerSignature(const std::vector<uint8_t>& serverSigned1, const std::vector<uint8_t>& signature, X509* serverCert,
				   const std::string& certSource);
	int getEUICCCurveNID();

	// Member variables
	std::string m_serverUrl;

	std::unique_ptr<X509, X509Deleter> m_rootCA;
	std::vector<std::unique_ptr<X509, X509Deleter>> m_intermediateCA;
	std::unique_ptr<X509, X509Deleter> m_serverCert;
	std::vector<std::unique_ptr<X509, X509Deleter>> m_certPool;

	std::vector<uint8_t> m_euiccSKI;
	std::string m_EID;
	std::unique_ptr<X509, X509Deleter> m_euiccCert;
	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_euiccPrivateKey;
	std::vector<uint8_t> m_euiccPublicKeyData;
	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_euicc_ot_PrivateKey;
	std::vector<uint8_t> m_euiccOtpk;

	std::unique_ptr<X509, X509Deleter> m_eumCert;
	std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_eumPrivateKey;
	std::vector<uint8_t> m_eumPublicKeyData;

	std::string m_confirmationCode;
	std::vector<uint8_t> m_confirmationCodeHash;
	std::vector<uint8_t> m_transactionId;
	std::string m_caCertPath;

	std::unique_ptr<HttpClient> m_httpClient;
	bool m_useCustomTlsCert;
	std::string m_customTlsCertPath;

	bool m_useMutualTLS;
	std::string m_clientCertPath;
	std::string m_clientKeyPath;
};

// Utility classes used by RSPClient (exposed for use in external functions)
class CertificateUtil {
    public:
	static std::unique_ptr<X509, X509Deleter> loadCertFromDER(const std::vector<uint8_t>& derData);
	static std::vector<std::unique_ptr<X509, X509Deleter>> loadCertificateChain(const std::string& filename);
	static std::vector<std::unique_ptr<X509, X509Deleter>> loadCertificatesFromDirectory(const std::string& directory,
											     const std::vector<std::string>& name_filters);
	static std::string getEID(X509* cert);
	static std::string getSubjectName(X509* cert);
	static std::vector<uint8_t> getSubjectKeyIdentifier(X509* cert);
	static std::vector<uint8_t> getAuthorityKeyIdentifier(X509* cert);
	static bool validateEIDRange(const std::string& eid, const std::vector<uint8_t>& eumCertData);
	static bool hasRSPRole(const std::vector<uint8_t>& certData, const std::string& roleOid);
	static std::string getPermittedEINs(const std::vector<uint8_t>& eumCertData);
	static bool verifyECDHCompatible(const std::vector<uint8_t>& pubKey1, const std::vector<uint8_t>& pubKey2);
	static std::string getCurveOID(const std::vector<uint8_t>& certData);
	static bool verifyCertificateChainDynamic(X509* cert, const std::vector<X509*>& certPool, X509* rootCA, bool verbose);
};

} // namespace RspCrypto

#endif // RSP_CLIENT_H