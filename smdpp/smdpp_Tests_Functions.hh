/* ============================================================================
 * HEADER FILE: smdpp_Tests_Functions.hh
 * TTCN-3 External Function Declarations for RSP Testing
 * ============================================================================ */

#ifndef SMDPP_TESTS_FUNCTIONS_HH
#define SMDPP_TESTS_FUNCTIONS_HH

#include <TTCN3.hh>
#include <memory>
#include <map>
#include <mutex>

// Forward declarations for RSP classes
namespace RspCrypto {
    class RSPClient;
    class CertificateUtil;
    class Base64;
    class HexUtil;
    class Logger;
    class OpenSSLErrorHandler;
}

/* ============================================================================
 * TTCN-3 External Function Prototypes
 * Function names must exactly match TTCN-3 external function declarations
 * ============================================================================ */

namespace smdpp__Tests {


/* RSP Client Management */
INTEGER ext__RSPClient__create(const CHARSTRING& serverUrl, const INTEGER& serverPort,
                              const CHARSTRING& certPath, const CHARSTRING& nameFilter);

INTEGER ext__RSPClient__destroy(const INTEGER& clientHandle);

INTEGER ext__RSPClient__loadEUICCCertificate(const INTEGER& clientHandle,
                                            const CHARSTRING& euiccCertPath);

INTEGER ext__RSPClient__loadEUICCKeyPair(const INTEGER& clientHandle,
                                        const CHARSTRING& euiccPrivateKeyPath);

INTEGER ext__RSPClient__loadSMDPKeyAndCertificate(const INTEGER& clientHandle,
                                                 const CHARSTRING& smdpPrivateKeyPath,
                                                 const CHARSTRING& smdpCertPath);

INTEGER ext__RSPClient__setTestMode(const INTEGER& clientHandle, const BOOLEAN& testMode);

/* Challenge Generation and Session Management */
OCTETSTRING ext__RSPClient__generateChallenge(const INTEGER& clientHandle);

OCTETSTRING ext__RSPClient__getTransactionId(const INTEGER& clientHandle);

OCTETSTRING ext__RSPClient__getServerChallenge(const INTEGER& clientHandle);

OCTETSTRING ext__RSPClient__getEuiccChallenge(const INTEGER& clientHandle);

BOOLEAN ext__RSPClient__isAuthenticationComplete(const INTEGER& clientHandle);

/* Authentication Flow */
CHARSTRING ext__RSPClient__createInitiateAuthenticationRequest(const INTEGER& clientHandle,
                                                              const OCTETSTRING& euiccChallenge,
                                                              const CHARSTRING& smdpAddress);

INTEGER ext__RSPClient__processInitiateAuthenticationResponse(const INTEGER& clientHandle,
                                                            const CHARSTRING& responseJson);

BOOLEAN ext__RSPClient__initiateAuthentication(const INTEGER& clientHandle);

/* Bound Profile Package */
CHARSTRING ext__RSPClient__createGetBoundProfilePackageRequest(const INTEGER& clientHandle);

INTEGER ext__RSPClient__processGetBoundProfilePackageResponse(const INTEGER& clientHandle,
                                                            const CHARSTRING& responseJson);

BOOLEAN ext__RSPClient__getBoundProfilePackage(const INTEGER& clientHandle);

/* Cryptographic Operations */
OCTETSTRING ext__RSPClient__signDataWithEUICC(const INTEGER& clientHandle,
                                             const OCTETSTRING& dataToSign);

OCTETSTRING ext__RSPClient__signDataWithSMDP(const INTEGER& clientHandle,
                                            const OCTETSTRING& dataToSign);

OCTETSTRING ext__RSPClient__generateEUICCOtpk(const INTEGER& clientHandle);
OCTETSTRING ext__RSPClient__getEUICCOtpk(const INTEGER& clientHandle);


INTEGER ext__RSPClient__setConfirmationCodeHash(const INTEGER& clientHandle,
                                               const OCTETSTRING& hash);

BOOLEAN ext__RSPClient__verifyServerSignature(const INTEGER& clientHandle, const OCTETSTRING& serverSigned, const OCTETSTRING& serverSignature1, const OCTETSTRING& serverCert);

/* ASN.1 Structure Creation */
OCTETSTRING ext__RSPClient__createSmdpSigned2(const INTEGER& clientHandle);

OCTETSTRING ext__RSPClient__createEUICCSigned2(const INTEGER& clientHandle);

OCTETSTRING ext__RSPClient__signEUICCSigned2(const INTEGER& clientHandle,
                                            const OCTETSTRING& euiccSigned2);

OCTETSTRING ext__RSPClient__createPrepareDownloadResponseOk(const INTEGER& clientHandle);

/* Certificate Utilities */
OCTETSTRING ext__CertificateUtil__loadCertFromPEM(const CHARSTRING& pemData);

OCTETSTRING ext__CertificateUtil__loadCertFromDER(const OCTETSTRING& derData);

CHARSTRING ext__CertificateUtil__getSubjectName(const OCTETSTRING& certData);

CHARSTRING ext__CertificateUtil__getIssuerName(const OCTETSTRING& certData);

BOOLEAN ext__CertificateUtil__isExpired(const OCTETSTRING& certData);

OCTETSTRING ext__CertificateUtil__getSubjectKeyIdentifier(const OCTETSTRING& certData);

CHARSTRING ext__CertificateUtil__getEID(const OCTETSTRING& certData);

// BOOLEAN ext__CertificateUtil__validateEIDRange(const CHARSTRING& eid, const OCTETSTRING& eumCertData);

BOOLEAN ext__CertificateUtil__hasPolicyOID(const OCTETSTRING& certData, const CHARSTRING& oidStr);

BOOLEAN ext__CertificateUtil__hasRSPRole(const OCTETSTRING& certData, const CHARSTRING& roleOid);

BOOLEAN ext__RSPClient__verifyInitialiseSecureChannelRequest(const INTEGER& clientHandle,
                                                            const OCTETSTRING& iscReqData,
                                                            const OCTETSTRING& signature,
                                                            const OCTETSTRING& dpPbCert);

CHARSTRING ext__CertificateUtil__getPermittedEINs(const OCTETSTRING& eumCertData);

BOOLEAN ext__CertificateUtil__verifyECDHCompatible(const OCTETSTRING& pubKey1,
                                                   const OCTETSTRING& pubKey2);

OCTETSTRING ext__CertificateUtil__getAuthorityKeyIdentifier(const OCTETSTRING& certData);

BOOLEAN ext__CertificateUtil__verifyCertificateChainDynamic(const OCTETSTRING& cert,
                                                          const CHARSTRING& certPoolDir,
                                                          const OCTETSTRING& rootCA);

INTEGER ext__CertificateUtil__printCertificateDetails(const OCTETSTRING& certData);

BOOLEAN ext__CertificateUtil__verify_TR031111(const OCTETSTRING& message,
                                             const OCTETSTRING& bsiSignature,
                                             const OCTETSTRING& publicKey);

/* Utility Functions */
// OCTETSTRING ext__Base64__decode(const CHARSTRING& b64message);

// CHARSTRING ext__Base64__encode(const OCTETSTRING& data);

// OCTETSTRING ext__HexUtil__hexToBytes(const CHARSTRING& hex);

// CHARSTRING ext__HexUtil__bytesToHex(const OCTETSTRING& bytes);

/* OpenSSL Error Handling */
CHARSTRING ext__OpenSSLErrorHandler__getLastError();

INTEGER ext__OpenSSLErrorHandler__printSSLErrors(const CHARSTRING& context);

/* Logger Functions */
INTEGER ext__Logger__debug(const CHARSTRING& message, const CHARSTRING& filename, const INTEGER& line);

INTEGER ext__Logger__info(const CHARSTRING& message, const CHARSTRING& filename, const INTEGER& line);

INTEGER ext__Logger__warning(const CHARSTRING& message, const CHARSTRING& filename, const INTEGER& line);

INTEGER ext__Logger__error(const CHARSTRING& message, const CHARSTRING& filename, const INTEGER& line);

void ext__logInfo(const CHARSTRING& message);

void ext__logError(const CHARSTRING& message);

void ext__logDebug(const CHARSTRING& message);


/* ============================================================================
 * C++ CLIENT REGISTRY
 * Thread-safe registry for managing RSP client instances
 * ============================================================================ */

class RSPClientRegistry {
public:
    static RSPClientRegistry& getInstance();

    int createClient(const std::string& serverUrl, unsigned int serverPort,
                    const std::string& certPath, const std::string& nameFilter);

    RspCrypto::RSPClient* getClient(int handle);

    bool destroyClient(int handle);

    void destroyAllClients();

private:
    RSPClientRegistry() = default;
    ~RSPClientRegistry();

    std::mutex m_mutex;
    std::map<int, std::unique_ptr<RspCrypto::RSPClient>> m_clients;
    int m_nextHandle = 1;
};

/* ============================================================================
 * UTILITY FUNCTIONS FOR TYPE CONVERSION
 * Convert between TTCN-3 types and C++ types
 * ============================================================================ */

std::vector<uint8_t> octetstring_to_bytes(const OCTETSTRING& octetstr);
OCTETSTRING bytes_to_octetstring(const std::vector<uint8_t>& bytes);
std::string charstring_to_string(const CHARSTRING& charstr);
CHARSTRING string_to_charstring(const std::string& str);

} // extern "C"

#endif // SMDPP_TESTS_FUNCTIONS_HH

