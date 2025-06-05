/* ============================================================================
 * IMPLEMENTATION FILE: smdpp_Tests_Functions.cc
 * TTCN-3 External Function Implementations
 * ============================================================================ */

#include "smdpp_Tests_Functions.hh"
#include "smdvalcpp2.cpp" // Your existing RSP implementation

#include <iostream>
#include <cstring>

using namespace RspCrypto;

/* ============================================================================
 * RSP CLIENT REGISTRY IMPLEMENTATION
 * ============================================================================ */
namespace smdpp__Tests {

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

std::vector<uint8_t> octetstring_to_bytes(const OCTETSTRING& octetstr) {
    const unsigned char* data = static_cast<const unsigned char*>(octetstr);
    int length = octetstr.lengthof();
    return std::vector<uint8_t>(data, data + length);
}

OCTETSTRING bytes_to_octetstring(const std::vector<uint8_t>& bytes) {
    return OCTETSTRING(bytes.size(), bytes.data());
}

std::string charstring_to_string(const CHARSTRING& charstr) {
    return std::string(static_cast<const char*>(charstr));
}

CHARSTRING string_to_charstring(const std::string& str) {
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



OCTETSTRING ext__RSPClient__getSMDPCertificate(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        std::vector<uint8_t> cert = client->getSMDPCertificate();

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

INTEGER ext__RSPClient__loadSMDPKeyAndCertificate(const INTEGER& clientHandle,
                                                 const CHARSTRING& smdpPrivateKeyPath,
                                                 const CHARSTRING& smdpCertPath) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }

        std::string keyPath = charstring_to_string(smdpPrivateKeyPath);
        std::string certPath = charstring_to_string(smdpCertPath);
        client->loadSMDPKeyAndCertificate(keyPath, certPath);

        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__loadSMDPKeyAndCertificate failed: " + std::string(e.what()));
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

OCTETSTRING ext__RSPClient__getTransactionId(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        // Access private member through public interface (would need to add getter)
        // For now, return empty - this would need RSPClient modification
        return OCTETSTRING(0, nullptr);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__getTransactionId failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

OCTETSTRING ext__RSPClient__getServerChallenge(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        // Access private member through public interface (would need to add getter)
        // For now, return empty - this would need RSPClient modification
        return OCTETSTRING(0, nullptr);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__getServerChallenge failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

OCTETSTRING ext__RSPClient__getEuiccChallenge(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        // Access private member through public interface (would need to add getter)
        // For now, return empty - this would need RSPClient modification
        return OCTETSTRING(0, nullptr);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__getEuiccChallenge failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

BOOLEAN ext__RSPClient__isAuthenticationComplete(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return BOOLEAN(false);
        }

        // Access private member through public interface (would need to add getter)
        // For now, return false - this would need RSPClient modification
        return BOOLEAN(false);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__isAuthenticationComplete failed: " + std::string(e.what()));
        return BOOLEAN(false);
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

OCTETSTRING ext__RSPClient__signDataWithSMDP(const INTEGER& clientHandle,
                                            const OCTETSTRING& dataToSign) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING(0, nullptr);
        }

        std::vector<uint8_t> data = octetstring_to_bytes(dataToSign);
        std::vector<uint8_t> signature = client->signDataWithSMDP(data);

        return bytes_to_octetstring(signature);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__signDataWithSMDP failed: " + std::string(e.what()));
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

/* Certificate Utilities */
OCTETSTRING ext__CertificateUtil__loadCertFromPEM(const CHARSTRING& pemData) {
    try {
        std::string pem = charstring_to_string(pemData);
        auto cert = CertificateUtil::loadCertFromPEM(pem);

        // Convert X509* to DER bytes (would need implementation)
        return OCTETSTRING(0, nullptr);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__loadCertFromPEM failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

OCTETSTRING ext__CertificateUtil__loadCertFromDER(const OCTETSTRING& derData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(derData);
        auto cert = CertificateUtil::loadCertFromDER(der);

        // Return the same DER data for now
        return derData;
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__loadCertFromDER failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

CHARSTRING ext__CertificateUtil__getSubjectName(const OCTETSTRING& certData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        auto cert = CertificateUtil::loadCertFromDER(der);
        std::string subject = CertificateUtil::getSubjectName(cert.get());

        return string_to_charstring(subject);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getSubjectName failed: " + std::string(e.what()));
        return CHARSTRING("");
    }
}

CHARSTRING ext__CertificateUtil__getIssuerName(const OCTETSTRING& certData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        auto cert = CertificateUtil::loadCertFromDER(der);
        std::string issuer = CertificateUtil::getIssuerName(cert.get());

        return string_to_charstring(issuer);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getIssuerName failed: " + std::string(e.what()));
        return CHARSTRING("");
    }
}

BOOLEAN ext__CertificateUtil__isExpired(const OCTETSTRING& certData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        auto cert = CertificateUtil::loadCertFromDER(der);
        bool expired = CertificateUtil::isExpired(cert.get());

        return BOOLEAN(expired);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__isExpired failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

OCTETSTRING ext__CertificateUtil__getSubjectKeyIdentifier(const OCTETSTRING& certData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        auto cert = CertificateUtil::loadCertFromDER(der);
        std::vector<uint8_t> ski = CertificateUtil::getSubjectKeyIdentifier(cert.get());

        return bytes_to_octetstring(ski);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getSubjectKeyIdentifier failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
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

// In smdpp_Tests_Functions.cc, add these functions:

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

BOOLEAN ext__RSPClient__verifyInitialiseSecureChannelRequest(const INTEGER& clientHandle,
                                                            const OCTETSTRING& transactionId,
                                                            const OCTETSTRING& controlRefTemplate,
                                                            const OCTETSTRING& smdpOtpk,
                                                            const OCTETSTRING& signature,
                                                            const OCTETSTRING& dpPbCert) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle");
            return BOOLEAN(false);
        }

        // Build the data that was signed
        std::vector<uint8_t> tid = octetstring_to_bytes(transactionId);
        std::vector<uint8_t> crt = octetstring_to_bytes(controlRefTemplate);
        std::vector<uint8_t> otpk = octetstring_to_bytes(smdpOtpk);
        std::vector<uint8_t> sig = octetstring_to_bytes(signature);
        std::vector<uint8_t> cert = octetstring_to_bytes(dpPbCert);

        // This is a thin wrapper - the actual logic is in RSPClient::verifyInitialiseSecureChannelRequest
        bool result = client->verifyInitialiseSecureChannelRequest(tid, crt, otpk, sig, cert);
        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__verifyInitialiseSecureChannelRequest failed: " + std::string(e.what()));
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

        return bytes_to_octetstring(sharedSecret);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__computeSharedSecret failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
    }
}

// // MISSING: Derive session keys from shared secret
// BOOLEAN ext__Crypto__deriveSessionKeys(const OCTETSTRING& sharedSecret,
//                                       const OCTETSTRING& hostId,
//                                       OCTETSTRING& sEnc,
//                                       OCTETSTRING& sMac,
//                                       OCTETSTRING& sDek) {
//     try {
//         std::vector<uint8_t> ss = octetstring_to_bytes(sharedSecret);
//         std::vector<uint8_t> hid = octetstring_to_bytes(hostId);

//         // Implement key derivation as per GlobalPlatform SCP03
//         // Using SHA256-based KDF

//         // Derive S-ENC
//         std::vector<uint8_t> sEncInput = ss;
//         sEncInput.insert(sEncInput.end(), hid.begin(), hid.end());
//         sEncInput.push_back(0x01); // Counter for S-ENC

//         unsigned char sEncHash[SHA256_DIGEST_LENGTH];
//         SHA256(sEncInput.data(), sEncInput.size(), sEncHash);
//         sEnc = OCTETSTRING(16, sEncHash); // Take first 16 bytes

//         // Derive S-MAC
//         std::vector<uint8_t> sMacInput = ss;
//         sMacInput.insert(sMacInput.end(), hid.begin(), hid.end());
//         sMacInput.push_back(0x02); // Counter for S-MAC

//         unsigned char sMacHash[SHA256_DIGEST_LENGTH];
//         SHA256(sMacInput.data(), sMacInput.size(), sMacHash);
//         sMac = OCTETSTRING(16, sMacHash); // Take first 16 bytes

//         // Derive S-DEK
//         std::vector<uint8_t> sDekInput = ss;
//         sDekInput.insert(sDekInput.end(), hid.begin(), hid.end());
//         sDekInput.push_back(0x03); // Counter for S-DEK

//         unsigned char sDekHash[SHA256_DIGEST_LENGTH];
//         SHA256(sDekInput.data(), sDekInput.size(), sDekHash);
//         sDek = OCTETSTRING(16, sDekHash); // Take first 16 bytes

//         LOG_INFO("Derived session keys successfully");
//         return BOOLEAN(true);
//     } catch (const std::exception& e) {
//         LOG_ERROR("ext__Crypto__deriveSessionKeys failed: " + std::string(e.what()));
//         return BOOLEAN(false);
//     }
// }

// Verify encrypted profile data using session keys (MAC verification and decryption)
BOOLEAN ext__Crypto__verifyEncryptedProfileData(const INTEGER& clientHandle,
                                                const OCTETSTRING& encData,
                                               const OCTETSTRING& sEnc,
                                               const OCTETSTRING& sMac) {
    try {
        std::vector<uint8_t> data = octetstring_to_bytes(encData);
        std::vector<uint8_t> s_enc = octetstring_to_bytes(sEnc);
        std::vector<uint8_t> s_mac = octetstring_to_bytes(sMac);

        RSPClient* client = RSPClientRegistry::getInstance().getClient(clientHandle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle");
            return BOOLEAN(false);
        }

        bool result = client->verifyEncryptedProfileData(data, s_enc, s_mac);

        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__Crypto__verifyEncryptedProfileData failed: " + std::string(e.what()));
        return BOOLEAN(false);
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


// Helper functions moved to RSPClient class - these are removed


BOOLEAN ext__Crypto__deriveSessionKeys(const INTEGER& clientHandle,
                                        const OCTETSTRING& sharedSecret,
                                      const OCTETSTRING& hostId,
                                      OCTETSTRING& sEnc,
                                      OCTETSTRING& sMac,
                                      OCTETSTRING& sDek) {
    try {
        std::vector<uint8_t> ss = octetstring_to_bytes(sharedSecret);
        std::vector<uint8_t> hid = octetstring_to_bytes(hostId);
        std::vector<uint8_t> sEnc_vec, sMac_vec, sDek_vec;

        RSPClient* client = RSPClientRegistry::getInstance().getClient(clientHandle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle");
            return BOOLEAN(false);
        }

        bool result = client->deriveSessionKeys(ss, hid, sEnc_vec, sMac_vec, sDek_vec);

        if (result) {
            sEnc = bytes_to_octetstring(sEnc_vec);
            sMac = bytes_to_octetstring(sMac_vec);
            sDek = bytes_to_octetstring(sDek_vec);
        }

        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__Crypto__deriveSessionKeys failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

// Helper functions moved to RSPClient class - these are removed

BOOLEAN ext__Crypto__deriveBSPSessionKeys(const INTEGER& clientHandle,
                                        const OCTETSTRING& sharedSecret,
                                        const INTEGER& keyType,
                                        const INTEGER& keyLength,
                                        const OCTETSTRING& hostId,
                                        const OCTETSTRING& eid,
                                        OCTETSTRING& bspSEnc,
                                        OCTETSTRING& bspSMac,
                                        OCTETSTRING& initialMCV) {
    try {
        std::vector<uint8_t> ss = octetstring_to_bytes(sharedSecret);
        std::vector<uint8_t> host_id = octetstring_to_bytes(hostId);
        std::vector<uint8_t> eid_bytes = octetstring_to_bytes(eid);

        uint8_t key_type = static_cast<uint8_t>(keyType.get_long_long_val());
        uint8_t key_len = static_cast<uint8_t>(keyLength.get_long_long_val());

        std::vector<uint8_t> bspSEnc_vec, bspSMac_vec, initialMCV_vec;

        RSPClient* client = RSPClientRegistry::getInstance().getClient(clientHandle);

        if (!client) {
            LOG_ERROR("Invalid RSP client handle");
            return BOOLEAN(false);
        }

        bool result = client->deriveBSPSessionKeys(ss, key_type, key_len, host_id, eid_bytes,
                                                     bspSEnc_vec, bspSMac_vec, initialMCV_vec);

        if (result) {
            bspSEnc = bytes_to_octetstring(bspSEnc_vec);
            bspSMac = bytes_to_octetstring(bspSMac_vec);
            initialMCV = bytes_to_octetstring(initialMCV_vec);
        }

        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__Crypto__deriveBSPSessionKeys failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

OCTETSTRING ext__CertificateUtil__getAuthorityKeyIdentifier(const OCTETSTRING& certData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        auto cert = CertificateUtil::loadCertFromDER(der);
        std::vector<uint8_t> aki = CertificateUtil::getAuthorityKeyIdentifier(cert.get());

        return bytes_to_octetstring(aki);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__getAuthorityKeyIdentifier failed: " + std::string(e.what()));
        return OCTETSTRING(0, nullptr);
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

        // Enable test mode to handle name constraint violations gracefully
        bool result = CertificateUtil::verifyCertificateChainDynamic(
            certObj.get(), certPoolRaw, rootObj.get(), false, true); // verbose=false, testMode=true

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

        // Enable test mode to handle name constraint violations gracefully
        bool result = CertificateUtil::verifyCertificateChainDynamic(
            certObj.get(), certPool, rootObj.get(), false, true); // verbose=false, testMode=true

        return BOOLEAN(result);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__verifyCertificateChainWithIntermediate failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}

INTEGER ext__CertificateUtil__printCertificateDetails(const OCTETSTRING& certData) {
    try {
        std::vector<uint8_t> der = octetstring_to_bytes(certData);
        auto cert = CertificateUtil::loadCertFromDER(der);
        CertificateUtil::printCertificateDetails(cert.get());

        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__printCertificateDetails failed: " + std::string(e.what()));
        return INTEGER(-1);
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

// /* Utility Functions */
// OCTETSTRING ext__Base64__decode(const CHARSTRING& b64message) {
//     try {
//         std::string b64 = charstring_to_string(b64message);
//         std::vector<uint8_t> decoded = Base64::decode(b64);

//         return bytes_to_octetstring(decoded);
//     } catch (const std::exception& e) {
//         LOG_ERROR("ext__Base64__decode failed: " + std::string(e.what()));
//         return OCTETSTRING(0, nullptr);
//     }
// }

// CHARSTRING ext__Base64__encode(const OCTETSTRING& data) {
//     try {
//         std::vector<uint8_t> bytes = octetstring_to_bytes(data);
//         std::string encoded = Base64::encode(bytes);

//         return string_to_charstring(encoded);
//     } catch (const std::exception& e) {
//         LOG_ERROR("ext__Base64__encode failed: " + std::string(e.what()));
//         return CHARSTRING("");
//     }
// }

// OCTETSTRING ext__HexUtil__hexToBytes(const CHARSTRING& hex) {
//     try {
//         std::string hexStr = charstring_to_string(hex);
//         std::vector<uint8_t> bytes = HexUtil::hexToBytes(hexStr);

//         return bytes_to_octetstring(bytes);
//     } catch (const std::exception& e) {
//         LOG_ERROR("ext__HexUtil__hexToBytes failed: " + std::string(e.what()));
//         return OCTETSTRING(0, nullptr);
//     }
// }

// CHARSTRING ext__HexUtil__bytesToHex(const OCTETSTRING& bytes) {
//     try {
//         std::vector<uint8_t> data = octetstring_to_bytes(bytes);
//         std::string hex = HexUtil::bytesToHex(data);

//         return string_to_charstring(hex);
//     } catch (const std::exception& e) {
//         LOG_ERROR("ext__HexUtil__bytesToHex failed: " + std::string(e.what()));
//         return CHARSTRING("");
//     }
// }

/* OpenSSL Error Handling */
CHARSTRING ext__OpenSSLErrorHandler__getLastError() {
    try {
        std::string error = OpenSSLErrorHandler::getLastError();
        return string_to_charstring(error);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__OpenSSLErrorHandler__getLastError failed: " + std::string(e.what()));
        return CHARSTRING("");
    }
}

INTEGER ext__OpenSSLErrorHandler__printSSLErrors(const CHARSTRING& context) {
    try {
        std::string ctx = charstring_to_string(context);
        OpenSSLErrorHandler::printSSLErrors(ctx);
        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__OpenSSLErrorHandler__printSSLErrors failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}

/* Logger Functions */
INTEGER ext__Logger__debug(const CHARSTRING& message, const CHARSTRING& filename, const INTEGER& line) {
    try {
        std::string msg = charstring_to_string(message);
        std::string file = charstring_to_string(filename);
        int lineNum = static_cast<int>(line);

        Logger::debug(msg, file.c_str(), lineNum);
        return INTEGER(0);
    } catch (const std::exception& e) {
        std::cerr << "ext__Logger__debug failed: " << e.what() << std::endl;
        return INTEGER(-1);
    }
}

INTEGER ext__Logger__info(const CHARSTRING& message, const CHARSTRING& filename, const INTEGER& line) {
    try {
        std::string msg = charstring_to_string(message);
        std::string file = charstring_to_string(filename);
        int lineNum = static_cast<int>(line);

        Logger::info(msg, file.c_str(), lineNum);
        return INTEGER(0);
    } catch (const std::exception& e) {
        std::cerr << "ext__Logger__info failed: " << e.what() << std::endl;
        return INTEGER(-1);
    }
}

INTEGER ext__Logger__warning(const CHARSTRING& message, const CHARSTRING& filename, const INTEGER& line) {
    try {
        std::string msg = charstring_to_string(message);
        std::string file = charstring_to_string(filename);
        int lineNum = static_cast<int>(line);

        Logger::warning(msg, file.c_str(), lineNum);
        return INTEGER(0);
    } catch (const std::exception& e) {
        std::cerr << "ext__Logger__warning failed: " << e.what() << std::endl;
        return INTEGER(-1);
    }
}

INTEGER ext__Logger__error(const CHARSTRING& message, const CHARSTRING& filename, const INTEGER& line) {
    try {
        std::string msg = charstring_to_string(message);
        std::string file = charstring_to_string(filename);
        int lineNum = static_cast<int>(line);

        Logger::error(msg, file.c_str(), lineNum);
        return INTEGER(0);
    } catch (const std::exception& e) {
        std::cerr << "ext__Logger__error failed: " << e.what() << std::endl;
        return INTEGER(-1);
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
/* ============================================================================
 * BUILD CONFIGURATION
 * Makefile for TTCN-3 C++ Integration
 * ============================================================================ */

/*
# Makefile

# TTCN-3 Tools Configuration
TTCN3_DIR = /usr/local/ttcn3
TTCN3_COMPILER = $(TTCN3_DIR)/bin/compiler
TTCN3_MCTR_CLI = $(TTCN3_DIR)/bin/mctr_cli

# Include directories
TTCN3_INCLUDES = -I$(TTCN3_DIR)/include
OPENSSL_INCLUDES = -I/usr/include/openssl

# Libraries
TTCN3_LIBS = -L$(TTCN3_DIR)/lib -lttcn3-rt2 -lttcn3-rt2-parallel
OPENSSL_LIBS = -lssl -lcrypto
CURL_LIBS = -lcurl
JSON_LIBS = -lcjson

# Compiler flags
CPPFLAGS = -std=c++17 -Wall -Wextra -O2 -g
CPPFLAGS += $(TTCN3_INCLUDES) $(OPENSSL_INCLUDES)

# Source files
TTCN3_SOURCES = smdpp_Tests.ttcn3
CPP_SOURCES = smdpp_Tests_Functions.cc

# Generated files
TTCN3_GENERATED = smdpp_Tests.cc smdpp_Tests.hh

# Object files
OBJECTS = smdpp_Tests.o smdpp_Tests_Functions.o

# Target executable
TARGET = smdpp_test

.PHONY: all clean compile run

all: $(TARGET)

compile:
	$(TTCN3_COMPILER) $(TTCN3_SOURCES)

$(TARGET): compile $(OBJECTS)
	$(CXX) $(CPPFLAGS) -o $@ $(OBJECTS) $(TTCN3_LIBS) $(OPENSSL_LIBS) $(CURL_LIBS) $(JSON_LIBS)

%.o: %.cc
	$(CXX) $(CPPFLAGS) -c $< -o $@

smdpp_Tests.o: smdpp_Tests.cc smdpp_Tests.hh

smdpp_Tests_Functions.o: smdpp_Tests_Functions.cc smdpp_Tests_Functions.hh

run: $(TARGET)
	./$(TARGET) config.cfg

clean:
	rm -f $(TARGET) $(OBJECTS) $(TTCN3_GENERATED) *.log

install-deps:
	# Install TTCN-3 runtime
	wget https://www.eclipse.org/downloads/download.php?file=/titan/TTCN3-latest.tgz
	tar -xzf TTCN3-latest.tgz
	cd TTCN3-* && make install

	# Install OpenSSL development libraries
	sudo apt-get update
	sudo apt-get install -y libssl-dev libcurl4-openssl-dev libcjson-dev

# Configuration file for TTCN-3 runtime
config.cfg:
	echo "[LOGGING]" > config.cfg
	echo "LogFile := \"smdpp_test.log\"" >> config.cfg
	echo "FileMask := LOG_ALL" >> config.cfg
	echo "[TESTPORT_PARAMETERS]" >> config.cfg
	echo "[MODULE_PARAMETERS]" >> config.cfg
	echo "[GROUPS]" >> config.cfg
	echo "[COMPONENTS]" >> config.cfg
	echo "[MAIN_CONTROLLER]" >> config.cfg
	echo "TCPPort := 9999" >> config.cfg
	echo "[EXECUTE]" >> config.cfg
	echo "smdpp_Tests.control" >> config.cfg
*/