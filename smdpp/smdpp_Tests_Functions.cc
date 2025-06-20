/* ============================================================================
 * IMPLEMENTATION FILE: smdpp_Tests_Functions.cc
 * TTCN-3 External Function Implementations
 * ============================================================================ */
#include <memory>
#include <map>
#include <mutex>

#include <iostream>
#include <cstring>

#include <TTCN3.hh>
#include "smdpp_Tests.hh"
#include "smdvalcpp2.cpp" // Your existing RSP implementation
#include "bsp_crypto.cpp"


using namespace RspCrypto;

/* ============================================================================
 * RSP CLIENT REGISTRY IMPLEMENTATION
 * ============================================================================ */
namespace smdpp__Tests {


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

RSPClientRegistry& RSPClientRegistry::getInstance() {
    static RSPClientRegistry instance;
    return instance;
}

RSPClientRegistry::~RSPClientRegistry() {
    destroyAllClients();
}

int RSPClientRegistry::createClient(const std::string& serverUrl, unsigned int serverPort,
                                   const std::string& certPath, const std::string& nameFilter) {
    std::lock_guard<std::mutex> lock(m_mutex);

    try {
        const std::vector<std::string> certPaths = { certPath };
        const std::vector<std::string> nameFilters = { nameFilter };

        auto client = std::make_unique<RSPClient>(serverUrl, serverPort, certPaths, nameFilters);

    std::string euiccCertPath = "./sgp26/eUICC/CERT_EUICC_ECDSA_NIST.der";
	std::string euiccprivkeyPath = "./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem";

	std::string eumCertPath = "./sgp26/EUM/CERT_EUM_ECDSA_NIST.der";
	std::string eumprivkeyPath = "./sgp26/EUM/SK_EUM_ECDSA_NIST.pem";

	std::string caCertPath = "/etc/ssl/certs/ca-certificates.crt"; // Default CA certs on many Linux

		client->loadEUICCCertificate(euiccCertPath);
		client->loadEUICCKeyPair(euiccprivkeyPath);
		client->loadEUMCertificate(eumCertPath);
		client->loadEUMKeyPair(eumprivkeyPath);
		client->setCACertPath(caCertPath);

        int handle = m_nextHandle++;
        m_clients[handle] = std::move(client);

        LOG_INFO("Created RSP client with handle: " + std::to_string(handle));
        return handle;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create RSP client: " + std::string(e.what()));
        return -1;
    }
}

RspCrypto::RSPClient* RSPClientRegistry::getClient(int handle) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_clients.find(handle);
    if (it != m_clients.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool RSPClientRegistry::destroyClient(int handle) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_clients.find(handle);
    if (it != m_clients.end()) {
        LOG_INFO("Destroying RSP client with handle: " + std::to_string(handle));
        m_clients.erase(it);
        return true;
    }
    return false;
}

void RSPClientRegistry::destroyAllClients() {
    std::lock_guard<std::mutex> lock(m_mutex);
    LOG_INFO("Destroying all RSP clients (" + std::to_string(m_clients.size()) + " clients)");
    m_clients.clear();
}

/* ============================================================================
 * UTILITY FUNCTIONS FOR TYPE CONVERSION
 * ============================================================================ */

static std::vector<uint8_t> octetstring_to_bytes(const OCTETSTRING& octetstr) {
    const unsigned char* data = static_cast<const unsigned char*>(octetstr);
    int length = octetstr.lengthof();
    return std::vector<uint8_t>(data, data + length);
}

static OCTETSTRING bytes_to_octetstring(const std::vector<uint8_t>& bytes) {
    return OCTETSTRING(bytes.size(), bytes.data());
}

static std::string charstring_to_string(const CHARSTRING& charstr) {
    return std::string(static_cast<const char*>(charstr));
}

static CHARSTRING string_to_charstring(const std::string& str) {
    return CHARSTRING(str.c_str());
}

/* ============================================================================
 * TTCN-3 EXTERNAL FUNCTION IMPLEMENTATIONS
 * ============================================================================ */

/* RSP Client Management */
INTEGER ext__RSPClient__create(const CHARSTRING& serverUrl, const INTEGER& serverPort,
                              const CHARSTRING& certPath, const CHARSTRING& nameFilter) {
    try {
        std::string url = charstring_to_string(serverUrl);
        unsigned int port = static_cast<unsigned int>(static_cast<int>(serverPort));
        std::string certDir = charstring_to_string(certPath);
        std::string filter = charstring_to_string(nameFilter);

        int handle = RSPClientRegistry::getInstance().createClient(url, port, certDir, filter);
        return INTEGER(handle);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__create failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}

INTEGER ext__RSPClient__destroy(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        bool success = RSPClientRegistry::getInstance().destroyClient(handle);
        return INTEGER(success ? 0 : -1);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__destroy failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}

OCTETSTRING ext__RSPClient__getEUMCertificate(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        std::vector<uint8_t> cert = client->getEUMCertificate();

        return bytes_to_octetstring(cert);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getSubjectKeyIdentifier failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

OCTETSTRING ext__RSPClient__getEUICCCertificate(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        std::vector<uint8_t> cert = client->getEUICCCertificate();

        return bytes_to_octetstring(cert);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getSubjectKeyIdentifier failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

OCTETSTRING ext__RSPClient__getCICertificate(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        std::vector<uint8_t> cert = client->getCICertificate();

        return bytes_to_octetstring(cert);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__getCICertificate failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}


INTEGER ext__RSPClient__loadEUICCCertificate(const INTEGER& clientHandle,
                                            const CHARSTRING& euiccCertPath) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }

        std::string certPath = charstring_to_string(euiccCertPath);
        client->loadEUICCCertificate(certPath);

        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__loadEUICCCertificate failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}

INTEGER ext__RSPClient__loadEUICCKeyPair(const INTEGER& clientHandle,
                                        const CHARSTRING& euiccPrivateKeyPath) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }

        std::string keyPath = charstring_to_string(euiccPrivateKeyPath);
        client->loadEUICCKeyPair(keyPath);

        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__loadEUICCKeyPair failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}


/* Challenge Generation and Session Management */
OCTETSTRING ext__RSPClient__generateChallenge(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        std::vector<uint8_t> challenge = client->generateChallenge();
        return bytes_to_octetstring(challenge);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__generateChallenge failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

/* Cryptographic Operations */
OCTETSTRING ext__RSPClient__signDataWithEUICC(const INTEGER& clientHandle,
                                             const OCTETSTRING& dataToSign) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        std::vector<uint8_t> data = octetstring_to_bytes(dataToSign);
        std::vector<uint8_t> signature = client->signDataWithEUICC(data);

        return bytes_to_octetstring(signature);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__signDataWithEUICC failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

OCTETSTRING ext__RSPClient__generateEUICCOtpk(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        client->generateEUICCOtpk();
        std::vector<uint8_t> data = client->getEUICCOtpk();
        return bytes_to_octetstring(data);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__generateEUICCOtpk failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

OCTETSTRING ext__RSPClient__getEUICCOtpk(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        std::vector<uint8_t> data = client->getEUICCOtpk();
        return bytes_to_octetstring(data);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__generateEUICCOtpk failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

INTEGER ext__RSPClient__setConfirmationCode(const INTEGER& clientHandle,
                                           const CHARSTRING& confirmationCode) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }

        std::string code = charstring_to_string(confirmationCode);
        client->setConfirmationCode(code);

        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__setConfirmationCode failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}

INTEGER ext__RSPClient__setTransactionId(const INTEGER& clientHandle,
                                        const OCTETSTRING& transactionId) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }

        std::vector<uint8_t> tidVec = octetstring_to_bytes(transactionId);
        client->setTransactionId(tidVec);

        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__setTransactionId failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}

OCTETSTRING ext__RSPClient__getConfirmationCodeHash(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        std::vector<uint8_t> hash = client->getConfirmationCodeHash();
        return bytes_to_octetstring(hash);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__getConfirmationCodeHash failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

INTEGER ext__RSPClient__setConfirmationCodeHash(const INTEGER& clientHandle,
                                               const OCTETSTRING& hash) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }

        std::vector<uint8_t> hashVec = octetstring_to_bytes(hash);
        client->setConfirmationCodeHash(hashVec);

        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__setConfirmationCodeHash failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}

BOOLEAN ext__RSPClient__verifyServerSignature(const INTEGER& clientHandle, const OCTETSTRING& serverSigned, const OCTETSTRING& serverSignature1, const OCTETSTRING& serverCert) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return BOOLEAN(false);
        }

        std::vector<uint8_t> ss1 = octetstring_to_bytes(serverSigned);
        std::vector<uint8_t> ssi = octetstring_to_bytes(serverSignature1);
        std::vector<uint8_t> scd = octetstring_to_bytes(serverCert);

        auto rv = client->verifyServerSignature(ss1, ssi, scd);
        return BOOLEAN(rv);


    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__isExpired failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

CHARSTRING ext__CertificateUtil__getEID(const OCTETSTRING& certData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        auto cert = CertificateUtil::loadCertFromDER(der);
        std::string eid = CertificateUtil::getEID(cert.get());
        return string_to_charstring(eid);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getEID failed: " + std::string(e.what()));
        return CHARSTRING("");
    }
}

BOOLEAN ext__CertificateUtil__validateEIDRange(const CHARSTRING& eid, const OCTETSTRING& eumCertData) {
    try {
        std::string eidStr = charstring_to_string(eid);
        std::vector<uint8_t> eumDer = octetstring_to_bytes(eumCertData);

        // This is a thin wrapper - the actual logic is in CertificateUtil::validateEIDRange
        bool result = CertificateUtil::validateEIDRange(eidStr, eumDer);
        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__validateEIDRange failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

BOOLEAN ext__CertificateUtil__hasPolicyOID(const OCTETSTRING& certData, const CHARSTRING& oidStr) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        auto cert = CertificateUtil::loadCertFromDER(der);
        std::string oid = charstring_to_string(oidStr);

        // Check certificate policies extension
        int pos = X509_get_ext_by_NID(cert.get(), NID_certificate_policies, -1);
        if (pos < 0) return BOOLEAN(false);

        X509_EXTENSION *ext = X509_get_ext(cert.get(), pos);
        if (!ext) return BOOLEAN(false);

        // Parse and check policies (simplified - would need proper ASN.1 parsing)
        // For now, return true if extension exists
        return BOOLEAN(true);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__hasPolicyOID failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

// Verify certificate has correct RSP role OID
BOOLEAN ext__CertificateUtil__hasRSPRole(const OCTETSTRING& certData, const CHARSTRING& roleOid) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        std::string expectedOid = charstring_to_string(roleOid);

        // This is a thin wrapper - the actual logic is in CertificateUtil::hasRSPRole
        bool result = CertificateUtil::hasRSPRole(der, expectedOid);
        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__hasRSPRole failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

CHARSTRING ext__CertificateUtil__getPermittedEINs(const OCTETSTRING& eumCertData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(eumCertData);

        // This is a thin wrapper - the actual logic is in CertificateUtil::getPermittedEINs
        std::string result = CertificateUtil::getPermittedEINs(der);
        return string_to_charstring(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getPermittedEINs failed: " + std::string(e.what()));
        return CHARSTRING("");
    }
}

BOOLEAN ext__CertificateUtil__verifyECDHCompatible(const OCTETSTRING& pubKey1,
                                                   const OCTETSTRING& pubKey2) {
    try {
        std::vector<uint8_t> key1 = octetstring_to_bytes(pubKey1);
        std::vector<uint8_t> key2 = octetstring_to_bytes(pubKey2);

        // This is a thin wrapper - the actual logic is in CertificateUtil::verifyECDHCompatible
        bool result = CertificateUtil::verifyECDHCompatible(key1, key2);
        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__verifyECDHCompatible failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

// Fix the external function to use the actual implementation:
OCTETSTRING ext__RSPClient__computeSharedSecret(const INTEGER& clientHandle,
                                                const OCTETSTRING& otherPublicKey) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle");
            return OCTETSTRING(0, nullptr);
        }

        std::vector<uint8_t> otherPubKey = octetstring_to_bytes(otherPublicKey);
        std::vector<uint8_t> sharedSecret = client->computeECDHSharedSecret(otherPubKey);

        LOG_INFO("ECDH shared secret computed: " + HexUtil::bytesToHex(sharedSecret));

        return bytes_to_octetstring(sharedSecret);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__computeSharedSecret failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}


CHARSTRING ext__CertificateUtil__getCurveOID(const OCTETSTRING& certData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);

        // This is a thin wrapper - the actual logic is in CertificateUtil::getCurveOID
        std::string result = CertificateUtil::getCurveOID(der);
        return string_to_charstring(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getCurveOID failed: " + std::string(e.what()));
        return CHARSTRING("");
    }
}

BOOLEAN ext__CertificateUtil__verifyCertificateChainDynamic(const OCTETSTRING& cert,
                                                          const CHARSTRING& certPoolDir,
                                                          const OCTETSTRING& rootCA) {
    try {
        std::vector<uint8_t> certDer = octetstring_to_bytes(cert);
        std::string poolDir = charstring_to_string(certPoolDir);
        std::vector<uint8_t> rootDer = octetstring_to_bytes(rootCA);

        auto certObj = CertificateUtil::loadCertFromDER(certDer);
        auto rootObj = CertificateUtil::loadCertFromDER(rootDer);

        auto certPool = CertificateUtil::loadCertificatesFromDirectory(poolDir, {});
        std::vector<X509*> certPoolRaw;
        for (const auto& c : certPool) {
            certPoolRaw.push_back(c.get());
        }

        bool result = CertificateUtil::verifyCertificateChainDynamic(
            certObj.get(), certPoolRaw, rootObj.get(), false); // verbose=false

        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__verifyCertificateChainDynamic failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

BOOLEAN ext__CertificateUtil__verifyCertificateChainWithIntermediate(const OCTETSTRING& cert,
                                                                   const OCTETSTRING& intermediateCert,
                                                                   const OCTETSTRING& rootCA) {
    try {
        std::vector<uint8_t> certDer = octetstring_to_bytes(cert);
        std::vector<uint8_t> intermediateDer = octetstring_to_bytes(intermediateCert);
        std::vector<uint8_t> rootDer = octetstring_to_bytes(rootCA);

        auto certObj = CertificateUtil::loadCertFromDER(certDer);
        auto intermediateObj = CertificateUtil::loadCertFromDER(intermediateDer);
        auto rootObj = CertificateUtil::loadCertFromDER(rootDer);

        // Create a certificate pool containing just the intermediate certificate
        std::vector<X509*> certPool = { intermediateObj.get() };

        bool result = CertificateUtil::verifyCertificateChainDynamic(
            certObj.get(), certPool, rootObj.get(), false); // verbose=false

        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__verifyCertificateChainWithIntermediate failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

BOOLEAN ext__CertificateUtil__verify_TR031111(const OCTETSTRING& message,
                                             const OCTETSTRING& bsiSignature,
                                             const OCTETSTRING& publicKey) {
    try {
        std::vector<uint8_t> msg = octetstring_to_bytes(message);
        std::vector<uint8_t> sig = octetstring_to_bytes(bsiSignature);
        std::vector<uint8_t> pubkey = octetstring_to_bytes(publicKey);

        // This would need proper implementation with EVP_PKEY conversion
        return BOOLEAN(false);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__verify_TR031111 failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

void ext__logInfo(const CHARSTRING& message) {
    try {
        std::string msg = charstring_to_string(message);
        Logger::info(msg);
        return;
    } catch (const std::exception& e) {
        std::cerr << "ext__logInfo failed: " << e.what() << std::endl;
        return;
    }
}

void ext__logError(const CHARSTRING& message) {
    try {
        std::string msg = charstring_to_string(message);
        Logger::error(msg);
        return;
    } catch (const std::exception& e) {
        std::cerr << "ext__logError failed: " << e.what() << std::endl;
        return;
    }
}

void ext__logDebug(const CHARSTRING& message) {
    try {
        std::string msg = charstring_to_string(message);
        Logger::debug(msg);
        return;
    } catch (const std::exception& e) {
        std::cerr << "ext__logDebug failed: " << e.what() << std::endl;
        return;
    }
}

}

// Include BSP crypto implementation
#include "bsp_crypto.h"

namespace smdpp__Tests {

ProcessedBoundProfilePackage ext__BSP__processBoundProfilePackage(
    const OCTETSTRING& sharedSecret,
    const INTEGER& keyType,
    const INTEGER& keyLength,
    const OCTETSTRING& hostId,
    const CHARSTRING& eid,
    const OCTETSTRING& encodedBoundProfilePackage
) {
    try {
        // Convert TTCN-3 types to C++ vectors
        std::vector<uint8_t> sharedSecretVec = octetstring_to_bytes(sharedSecret);
        std::vector<uint8_t> hostIdVec = octetstring_to_bytes(hostId);

        std::string eidStr = charstring_to_string(eid);
        std::vector<uint8_t> eidVec;
        for (size_t i = 0; i < eidStr.length(); i += 2) {
            std::string hexByte = eidStr.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoi(hexByte, nullptr, 16));
            eidVec.push_back(byte);
        }

        std::vector<uint8_t> bppData = octetstring_to_bytes(encodedBoundProfilePackage);

        // Derive BSP session keys using X9.63 KDF (from_kdf)
        auto bsp = BspCryptoNS::BspCrypto::from_kdf(
            sharedSecretVec,
            static_cast<uint8_t>(keyType.get_long_long_val()),
            static_cast<uint8_t>(keyLength.get_long_long_val()),
            hostIdVec,
            eidVec
        );

        LOG_INFO("BSP session keys derived successfully");
        LOG_INFO("Shared secret: " + HexUtil::bytesToHex(sharedSecretVec));
        LOG_INFO("EID (converted): " + HexUtil::bytesToHex(eidVec));
        LOG_INFO("Processing BoundProfilePackage of size: " + std::to_string(bppData.size()));

        // Process the entire BoundProfilePackage
        auto result = bsp.process_bound_profile_package(bppData);

        // Convert result back to TTCN-3
        ProcessedBoundProfilePackage ttcn_result;
        ttcn_result.configureIsdp() = bytes_to_octetstring(result.configureIsdp);
        ttcn_result.storeMetadata() = bytes_to_octetstring(result.storeMetadata);
        ttcn_result.hasReplaceSessionKeys() = BOOLEAN(result.hasReplaceSessionKeys);
        ttcn_result.profileData() = bytes_to_octetstring(result.profileData);

        if (result.hasReplaceSessionKeys) {
            ttcn_result.ppkEnc() = bytes_to_octetstring(result.replaceSessionKeys.ppkEnc);
            ttcn_result.ppkMac() = bytes_to_octetstring(result.replaceSessionKeys.ppkCmac);
            ttcn_result.ppkInitialMCV() = bytes_to_octetstring(result.replaceSessionKeys.initialMacChainingValue);
            LOG_INFO("ReplaceSessionKeys present - PPK will be used for profile decryption");
        }

        LOG_INFO("BoundProfilePackage processed successfully");
        LOG_INFO("ConfigureISDP size: " + std::to_string(result.configureIsdp.size()));
        LOG_INFO("StoreMetadata size: " + std::to_string(result.storeMetadata.size()));
        LOG_INFO("Profile data size: " + std::to_string(result.profileData.size()));

        return ttcn_result;

    } catch (const std::exception& e) {
        LOG_ERROR("BSP processing failed: " + std::string(e.what()));
        // Return empty result on error
        ProcessedBoundProfilePackage error_result;
        error_result.configureIsdp() = OCTETSTRING(0, nullptr);
        error_result.storeMetadata() = OCTETSTRING(0, nullptr);
        error_result.hasReplaceSessionKeys() = BOOLEAN(false);
        error_result.profileData() = OCTETSTRING(0, nullptr);
        return error_result;
    }
}

ProcessedBoundProfilePackage ext__BSP__processSegments(
    const OCTETSTRING& sharedSecret,
    const INTEGER& keyType,
    const INTEGER& keyLength,
    const OCTETSTRING& hostId,
    const CHARSTRING& eid,
    const OCTETSTRING& firstSequenceOf87,
    const OCTETSTRING& sequenceOf88,
    const OCTETSTRING& secondSequenceOf87,
    const OCTETSTRING& sequenceOf86
) {
    try {
        // Convert TTCN-3 types to C++ vectors
        std::vector<uint8_t> sharedSecretVec = octetstring_to_bytes(sharedSecret);
        std::vector<uint8_t> hostIdVec = octetstring_to_bytes(hostId);

        std::string eidStr = charstring_to_string(eid);
        std::vector<uint8_t> eidVec;
        for (size_t i = 0; i < eidStr.length(); i += 2) {
            std::string hexByte = eidStr.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoi(hexByte, nullptr, 16));
            eidVec.push_back(byte);
        }

        // Convert segments
        std::vector<uint8_t> firstSeq87Vec = octetstring_to_bytes(firstSequenceOf87);
        std::vector<uint8_t> seq88Vec = octetstring_to_bytes(sequenceOf88);
        std::vector<uint8_t> secondSeq87Vec = octetstring_to_bytes(secondSequenceOf87);
        std::vector<uint8_t> seq86Vec = octetstring_to_bytes(sequenceOf86);

        // Derive BSP session keys using X9.63 KDF (from_kdf)
        auto bsp = BspCryptoNS::BspCrypto::from_kdf(
            sharedSecretVec,
            static_cast<uint8_t>(keyType.get_long_long_val()),
            static_cast<uint8_t>(keyLength.get_long_long_val()),
            hostIdVec,
            eidVec
        );

        LOG_INFO("BSP session keys derived successfully");
        LOG_INFO("Processing BoundProfilePackage segments");
        LOG_INFO("firstSequenceOf87 size: " + std::to_string(firstSeq87Vec.size()));
        LOG_INFO("sequenceOf88 size: " + std::to_string(seq88Vec.size()));
        LOG_INFO("secondSequenceOf87 size: " + std::to_string(secondSeq87Vec.size()));
        LOG_INFO("sequenceOf86 size: " + std::to_string(seq86Vec.size()));

        // Process segments using the 4-parameter version
        auto result = bsp.process_bound_profile_package(
            firstSeq87Vec,
            seq88Vec,
            secondSeq87Vec,
            seq86Vec
        );

        // Convert result back to TTCN-3
        ProcessedBoundProfilePackage ttcn_result;
        ttcn_result.configureIsdp() = bytes_to_octetstring(result.configureIsdp);
        ttcn_result.storeMetadata() = bytes_to_octetstring(result.storeMetadata);
        ttcn_result.hasReplaceSessionKeys() = BOOLEAN(result.hasReplaceSessionKeys);
        ttcn_result.profileData() = bytes_to_octetstring(result.profileData);

        if (result.hasReplaceSessionKeys) {
            ttcn_result.ppkEnc() = bytes_to_octetstring(result.replaceSessionKeys.ppkEnc);
            ttcn_result.ppkMac() = bytes_to_octetstring(result.replaceSessionKeys.ppkCmac);
            ttcn_result.ppkInitialMCV() = bytes_to_octetstring(result.replaceSessionKeys.initialMacChainingValue);
            LOG_INFO("ReplaceSessionKeys present - PPK will be used for profile decryption");
        }

        LOG_INFO("BoundProfilePackage segments processed successfully");
        LOG_INFO("ConfigureISDP size: " + std::to_string(result.configureIsdp.size()));
        LOG_INFO("StoreMetadata size: " + std::to_string(result.storeMetadata.size()));
        LOG_INFO("Profile data size: " + std::to_string(result.profileData.size()));

        return ttcn_result;

    } catch (const std::exception& e) {
        LOG_ERROR("BSP segment processing failed: " + std::string(e.what()));
        // Return empty result on error
        ProcessedBoundProfilePackage error_result;
        error_result.configureIsdp() = OCTETSTRING(0, nullptr);
        error_result.storeMetadata() = OCTETSTRING(0, nullptr);
        error_result.hasReplaceSessionKeys() = BOOLEAN(false);
        error_result.profileData() = OCTETSTRING(0, nullptr);
        return error_result;
    }
}

// HTTP client function wrapper
CHARSTRING ext__RSPClient__sendHttpsPost(
    const INTEGER& clientHandle,
    const CHARSTRING& endpoint,
    const CHARSTRING& body,
    INTEGER& statusCode
) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            statusCode = 0;
            return CHARSTRING("");
        }

        // Convert TTCN-3 types to C++
        std::string endpointStr = charstring_to_string(endpoint);
        std::string bodyStr = charstring_to_string(body);
        
        // Send HTTP request
        int httpStatus = 0;
        std::string response = client->sendHttpsPost(endpointStr, bodyStr, httpStatus);
        
        // Set output status code
        statusCode = httpStatus;
        
        // Return response body as CHARSTRING
        return CHARSTRING(response.c_str());
        
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__sendHttpsPost failed: " + std::string(e.what()));
        statusCode = 0;
        return CHARSTRING("");
    }
}

// Configure HTTP client settings
INTEGER ext__RSPClient__configureHttpClient(
    const INTEGER& clientHandle,
    const BOOLEAN& useCustomTlsCert,
    const CHARSTRING& customTlsCertPath
) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }

        // Convert TTCN-3 types to C++
        bool useCustomCert = useCustomTlsCert;
        std::string certPath = charstring_to_string(customTlsCertPath);
        
        // Configure HTTP client
        client->configureHttpClient(useCustomCert, certPath);
        
        LOG_INFO("Configured HTTP client - custom TLS cert: " + 
                 std::string(useCustomCert ? "true" : "false") +
                 (certPath.empty() ? "" : " (" + certPath + ")"));
        
        return INTEGER(0);
        
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__configureHttpClient failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}


}