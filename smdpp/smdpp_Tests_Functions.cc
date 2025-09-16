/* ============================================================================
 * IMPLEMENTATION FILE: smdpp_Tests_Functions.cc
 * TTCN-3 External Function Implementations
 * ============================================================================ */
#include <memory>
#include <map>
#include <mutex>
#include <functional>

#include <iostream>
#include <cstring>
#include <fstream>

#include <TTCN3.hh>
#include "smdpp_Tests.hh"
#include "rsp_client.cpp"
#include "bsp_crypto.cpp"

using namespace RspCrypto;

namespace smdpp__Tests {

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

class RSPClientRegistry {
    public:
	static RSPClientRegistry& getInstance();

	int createClient(const std::string& serverUrl, unsigned int serverPort, const std::string& certPath,
			 const std::string& nameFilter);

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

int RSPClientRegistry::createClient(const std::string& serverUrl, unsigned int serverPort, const std::string& certPath,
				    const std::string& nameFilter) {
	std::lock_guard<std::mutex> lock(m_mutex);

	try {
		// Include CertificateIssuer, EUM, and DPtls directories for complete chain and TLS
		const std::vector<std::string> certPaths = { certPath, "./sgp26/EUM", "./sgp26/DPtls" };
		const std::vector<std::string> nameFilters = { nameFilter, nameFilter, nameFilter };

		auto client = std::make_unique<RSPClient>(serverUrl, serverPort, certPaths, nameFilters);

		// Dynamically select certificate type based on nameFilter
		std::string certType = (nameFilter == "BRP") ? "BRP" : "NIST";

		std::string euiccCertPath = "./sgp26/eUICC/CERT_EUICC_ECDSA_" + certType + ".der";
		std::string euiccprivkeyPath = "./sgp26/eUICC/SK_EUICC_ECDSA_" + certType + ".pem";

		std::string eumCertPath = "./sgp26/EUM/CERT_EUM_ECDSA_" + certType + ".der";
		std::string eumprivkeyPath = "./sgp26/EUM/SK_EUM_ECDSA_" + certType + ".pem";

		std::string caCertPath = "/etc/ssl/certs/ca-certificates.crt"; // Default CA certs on many Linux

		// Load eUICC certificate and key pair
		client->loadEUICCCertificate(euiccCertPath);
		client->loadEUICCKeyPair(euiccprivkeyPath);
		// Load EUM certificate from ./sgp26/EUM directory
		client->loadEUMCertificate(eumCertPath);
		client->loadEUMKeyPair(eumprivkeyPath);
		client->setCACertPath(caCertPath);

		int handle = m_nextHandle++;
		m_clients[handle] = std::move(client);

		LOG_DEBUG("Created RSP client with handle: " + std::to_string(handle));
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
		LOG_DEBUG("Destroying RSP client with handle: " + std::to_string(handle));
		m_clients.erase(it);
		return true;
	}
	return false;
}

void RSPClientRegistry::destroyAllClients() {
	std::lock_guard<std::mutex> lock(m_mutex);
	LOG_DEBUG("Destroying all RSP clients (" + std::to_string(m_clients.size()) + " clients)");
	m_clients.clear();
}

template <void (RspCrypto::RSPClient::*LoaderFunc)(const std::string&)>
INTEGER client_loader(const INTEGER& clientHandle, const char* func_name, const CHARSTRING& path) {
	try {
		int handle = static_cast<int>(clientHandle);
		RspCrypto::RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

		if (!client) {
			LOG_ERROR(std::string("Invalid RSP client handle: ") + std::to_string(handle));
			return INTEGER(-1);
		}

		std::string pathStr = charstring_to_string(path);
		(client->*LoaderFunc)(pathStr);

		return INTEGER(0);
	} catch (const std::exception& e) {
		LOG_ERROR(std::string(func_name) + " failed: " + std::string(e.what()));
		return INTEGER(-1);
	}
}

template <typename ReturnType, typename Func>
ReturnType with_client(const INTEGER& clientHandle, const char* func_name, const ReturnType& error_value, Func&& func) {
	return safe_execute(func_name, error_value, [&]() {
		int handle = static_cast<int>(clientHandle);
		RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

		if (!client) {
			throw std::runtime_error("Invalid RSP client handle: " + std::to_string(handle));
		}

		return func(client);
	});
}

template <typename ReturnType, typename TTCNType, ReturnType (RspCrypto::RSPClient::*GetterFunc)()>
TTCNType client_getter(const INTEGER& clientHandle, const char* func_name, const TTCNType& error_value,
		       std::function<TTCNType(const ReturnType&)> converter) {
	return with_client(clientHandle, func_name, error_value, [&](RSPClient* client) {
		ReturnType result = (client->*GetterFunc)();
		return converter(result);
	});
}

template <typename ReturnType, typename TTCNType, ReturnType (RspCrypto::RSPClient::*GetterFunc)() const>
TTCNType client_getter_const(const INTEGER& clientHandle, const char* func_name, const TTCNType& error_value,
			     std::function<TTCNType(const ReturnType&)> converter) {
	return with_client(clientHandle, func_name, error_value, [&](RSPClient* client) {
		ReturnType result = (client->*GetterFunc)();
		return converter(result);
	});
}

template <typename InputType, typename CppType, void (RspCrypto::RSPClient::*SetterFunc)(const CppType&)>
INTEGER client_setter(const INTEGER& clientHandle, const char* func_name, const InputType& value,
		      std::function<CppType(const InputType&)> converter) {
	return with_client(clientHandle, func_name, INTEGER(-1), [&](RSPClient* client) {
		(client->*SetterFunc)(converter(value));
		return INTEGER(0);
	});
}

template <typename ReturnType, typename Func>
ReturnType safe_execute(const char* func_name, const ReturnType& error_value, Func&& func) {
	try {
		return func();
	} catch (const std::exception& e) {
		LOG_ERROR(std::string(func_name) + " failed: " + std::string(e.what()));
		return error_value;
	}
}

template <typename Func>
OCTETSTRING cert_octetstring_wrapper(const char* func_name, Func&& func) {
	return safe_execute(func_name, OCTETSTRING(0, nullptr), [&]() {
		std::vector<uint8_t> result = func();
		return bytes_to_octetstring(result);
	});
}

template <typename Func>
CHARSTRING cert_string_wrapper(const char* func_name, Func&& func) {
	return safe_execute(func_name, CHARSTRING(""), [&]() {
		std::string result = func();
		return string_to_charstring(result);
	});
}

template <typename Func>
BOOLEAN cert_bool_wrapper(const char* func_name, Func&& func) {
	return safe_execute(func_name, BOOLEAN(false), [&]() {
		bool result = func();
		return BOOLEAN(result);
	});
}

template <void (*LogFunc)(const std::string&, const char*, int)>
void log_wrapper(const char* func_name, const CHARSTRING& message) {
	try {
		LogFunc(charstring_to_string(message), __FILE__, __LINE__);
	} catch (const std::exception& e) {
		std::cerr << func_name << " failed: " << e.what() << std::endl;
	}
}

INTEGER ext__RSPClient__create(const CHARSTRING& serverUrl, const INTEGER& serverPort, const CHARSTRING& certPath,
			       const CHARSTRING& nameFilter) {
	return safe_execute("ext_RSPClient_create", INTEGER(-1), [&]() -> INTEGER {
		std::string url = charstring_to_string(serverUrl);
		unsigned int port = static_cast<unsigned int>(static_cast<int>(serverPort));
		std::string certDir = charstring_to_string(certPath);
		std::string filter = charstring_to_string(nameFilter);

		int handle = RSPClientRegistry::getInstance().createClient(url, port, certDir, filter);
		return INTEGER(handle);
	});
}

INTEGER ext__RSPClient__destroy(const INTEGER& clientHandle) {
	return safe_execute("ext_RSPClient_destroy", INTEGER(-1), [&]() -> INTEGER {
		int handle = static_cast<int>(clientHandle);
		bool success = RSPClientRegistry::getInstance().destroyClient(handle);
		return INTEGER(success ? 0 : -1);
	});
}

INTEGER ext__RSPClient__destroyAll() {
	return safe_execute("ext_RSPClient_destroyAll", INTEGER(-1), [&]() -> INTEGER {
		RSPClientRegistry::getInstance().destroyAllClients();
		return INTEGER(0);
	});
}

OCTETSTRING ext__RSPClient__getEUMCertificate(const INTEGER& clientHandle) {
	return client_getter<std::vector<uint8_t>, OCTETSTRING, &RSPClient::getEUMCertificate>(
		clientHandle, "ext__RSPClient__getEUMCertificate", OCTETSTRING(), bytes_to_octetstring);
}

OCTETSTRING ext__RSPClient__getEUICCCertificate(const INTEGER& clientHandle) {
	return client_getter<std::vector<uint8_t>, OCTETSTRING, &RSPClient::getEUICCCertificate>(
		clientHandle, "ext__RSPClient__getEUICCCertificate", OCTETSTRING(), bytes_to_octetstring);
}

OCTETSTRING ext__RSPClient__getCICertificate(const INTEGER& clientHandle) {
	return client_getter<std::vector<uint8_t>, OCTETSTRING, &RSPClient::getCICertificate>(
		clientHandle, "ext__RSPClient__getCICertificate", OCTETSTRING(), bytes_to_octetstring);
}

INTEGER ext__RSPClient__loadEUICCCertificate(const INTEGER& clientHandle, const CHARSTRING& euiccCertPath) {
	return client_loader<&RSPClient::loadEUICCCertificate>(clientHandle, "ext__RSPClient__loadEUICCCertificate",
							       euiccCertPath);
}

INTEGER ext__RSPClient__loadEUICCKeyPair(const INTEGER& clientHandle, const CHARSTRING& euiccPrivateKeyPath) {
	return safe_execute("ext_RSPClient_loadEUICCKeyPair", INTEGER(-1), [&]() -> INTEGER {
		int handle = static_cast<int>(clientHandle);
		RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

		if (!client) {
			LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
			return INTEGER(-1);
		}

		std::string keyPath = charstring_to_string(euiccPrivateKeyPath);
		client->loadEUICCKeyPair(keyPath);

		return INTEGER(0);
	});
}

INTEGER ext__RSPClient__loadEUMCertificate(const INTEGER& clientHandle, const CHARSTRING& eumCertPath) {
	return client_loader<&RSPClient::loadEUMCertificate>(clientHandle, "ext__RSPClient__loadEUMCertificate",
							     eumCertPath);
}

INTEGER ext__RSPClient__loadEUMKeyPair(const INTEGER& clientHandle, const CHARSTRING& eumPrivateKeyPath) {
	return safe_execute("ext_RSPClient_loadEUMKeyPair", INTEGER(-1), [&]() -> INTEGER {
		int handle = static_cast<int>(clientHandle);
		RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);

		if (!client) {
			LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
			return INTEGER(-1);
		}

		std::string keyPath = charstring_to_string(eumPrivateKeyPath);
		client->loadEUMKeyPair(keyPath);

		return INTEGER(0);
	});
}

INTEGER ext__RSPClient__setCACertPath(const INTEGER& clientHandle, const CHARSTRING& caCertPath) {
	return client_loader<&RSPClient::setCACertPath>(clientHandle, "ext__RSPClient__setCACertPath", caCertPath);
}

OCTETSTRING ext__RSPClient__generateChallenge(const INTEGER& clientHandle) {
	return client_getter<std::vector<uint8_t>, OCTETSTRING, &RSPClient::generateChallenge>(
		clientHandle, "ext__RSPClient__generateChallenge", OCTETSTRING(0, nullptr), bytes_to_octetstring);
}

OCTETSTRING ext__RSPClient__signDataWithEUICC(const INTEGER& clientHandle, const OCTETSTRING& dataToSign) {
	return with_client(clientHandle, "ext__RSPClient__signDataWithEUICC", OCTETSTRING(0, nullptr),
			   [&](RSPClient* client) {
				   std::vector<uint8_t> data = octetstring_to_bytes(dataToSign);
				   std::vector<uint8_t> signature = client->signDataWithEUICC(data);
				   return bytes_to_octetstring(signature);
			   });
}

OCTETSTRING ext__RSPClient__generateEUICCOtpk(const INTEGER& clientHandle) {
	return with_client(clientHandle, "ext__RSPClient__generateEUICCOtpk", OCTETSTRING(0, nullptr),
			   [&](RSPClient* client) {
				   client->generateEUICCOtpk();
				   std::vector<uint8_t> data = client->getEUICCOtpk();
				   return bytes_to_octetstring(data);
			   });
}

OCTETSTRING ext__RSPClient__getEUICCOtpk(const INTEGER& clientHandle) {
	return client_getter<std::vector<uint8_t>, OCTETSTRING, &RSPClient::getEUICCOtpk>(
		clientHandle, "ext__RSPClient__getEUICCOtpk", OCTETSTRING(0, nullptr), bytes_to_octetstring);
}

INTEGER ext__RSPClient__setConfirmationCode(const INTEGER& clientHandle, const CHARSTRING& confirmationCode) {
	return client_setter<CHARSTRING, std::string, &RSPClient::setConfirmationCode>(
		clientHandle, "ext__RSPClient__setConfirmationCode", confirmationCode, charstring_to_string);
}

INTEGER ext__RSPClient__setTransactionId(const INTEGER& clientHandle, const OCTETSTRING& transactionId) {
	return client_setter<OCTETSTRING, std::vector<uint8_t>, &RSPClient::setTransactionId>(
		clientHandle, "ext__RSPClient__setTransactionId", transactionId, octetstring_to_bytes);
}

OCTETSTRING ext__RSPClient__getConfirmationCodeHash(const INTEGER& clientHandle) {
	return client_getter_const<std::vector<uint8_t>, OCTETSTRING, &RSPClient::getConfirmationCodeHash>(
		clientHandle, "ext__RSPClient__getConfirmationCodeHash", OCTETSTRING(), bytes_to_octetstring);
}

INTEGER ext__RSPClient__setConfirmationCodeHash(const INTEGER& clientHandle, const OCTETSTRING& hash) {
	return client_setter<OCTETSTRING, std::vector<uint8_t>, &RSPClient::setConfirmationCodeHash>(
		clientHandle, "ext__RSPClient__setConfirmationCodeHash", hash, octetstring_to_bytes);
}

BOOLEAN ext__RSPClient__verifyServerSignature(const INTEGER& clientHandle, const OCTETSTRING& serverSigned,
					      const OCTETSTRING& serverSignature1, const OCTETSTRING& serverCert) {
	return with_client(clientHandle, "ext__RSPClient__verifyServerSignature", BOOLEAN(false),
			   [&](RSPClient* client) {
				   bool result = client->verifyServerSignature(octetstring_to_bytes(serverSigned),
									       octetstring_to_bytes(serverSignature1),
									       octetstring_to_bytes(serverCert));
				   return BOOLEAN(result);
			   });
}

CHARSTRING ext__CertificateUtil__getEID(const OCTETSTRING& certData) {
	return cert_string_wrapper("ext__CertificateUtil__getEID", [&]() {
		std::vector<uint8_t> der = octetstring_to_bytes(certData);
		auto cert = CertificateUtil::loadCertFromDER(der);
		return CertificateUtil::getEID(cert.get());
	});
}

BOOLEAN ext__CertificateUtil__validateEIDRange(const CHARSTRING& eid, const OCTETSTRING& eumCertData) {
	return cert_bool_wrapper("ext__CertificateUtil__validateEIDRange", [&]() {
		std::string eidStr = charstring_to_string(eid);
		std::vector<uint8_t> eumDer = octetstring_to_bytes(eumCertData);
		return CertificateUtil::validateEIDRange(eidStr, eumDer);
	});
}

BOOLEAN ext__CertificateUtil__hasRSPRole(const OCTETSTRING& certData, const CHARSTRING& roleOid) {
	return cert_bool_wrapper("ext__CertificateUtil__hasRSPRole", [&]() {
		std::vector<uint8_t> der = octetstring_to_bytes(certData);
		std::string expectedOid = charstring_to_string(roleOid);
		return CertificateUtil::hasRSPRole(der, expectedOid);
	});
}

CHARSTRING ext__CertificateUtil__getPermittedEINs(const OCTETSTRING& eumCertData) {
	return cert_string_wrapper("ext__CertificateUtil__getPermittedEINs", [&]() {
		std::vector<uint8_t> der = octetstring_to_bytes(eumCertData);
		return CertificateUtil::getPermittedEINs(der);
	});
}

BOOLEAN ext__CertificateUtil__verifyECDHCompatible(const OCTETSTRING& pubKey1, const OCTETSTRING& pubKey2) {
	return cert_bool_wrapper("ext__CertificateUtil__verifyECDHCompatible", [&]() {
		std::vector<uint8_t> key1 = octetstring_to_bytes(pubKey1);
		std::vector<uint8_t> key2 = octetstring_to_bytes(pubKey2);
		return CertificateUtil::verifyECDHCompatible(key1, key2);
	});
}

OCTETSTRING ext__RSPClient__computeSharedSecret(const INTEGER& clientHandle, const OCTETSTRING& otherPublicKey) {
	return with_client(clientHandle, "ext__RSPClient__computeSharedSecret", OCTETSTRING(0, nullptr),
			   [&](RSPClient* client) {
				   std::vector<uint8_t> otherPubKey = octetstring_to_bytes(otherPublicKey);
				   std::vector<uint8_t> sharedSecret = client->computeECDHSharedSecret(otherPubKey);

				   LOG_DEBUG("ECDH shared secret computed: " + HexUtil::bytesToHex(sharedSecret));

				   return bytes_to_octetstring(sharedSecret);
			   });
}

CHARSTRING ext__CertificateUtil__getCurveOID(const OCTETSTRING& certData) {
	return cert_string_wrapper("ext__CertificateUtil__getCurveOID", [&]() {
		std::vector<uint8_t> der = octetstring_to_bytes(certData);
		return CertificateUtil::getCurveOID(der);
	});
}

BOOLEAN ext__CertificateUtil__verifyCertificateChainDynamic(const OCTETSTRING& cert, const CHARSTRING& certPoolDir,
							    const OCTETSTRING& rootCA) {
	return cert_bool_wrapper("ext__CertificateUtil__verifyCertificateChainDynamic", [&]() {
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

		return CertificateUtil::verifyCertificateChainDynamic(certObj.get(), certPoolRaw, rootObj.get(),
								      false); // verbose=false
	});
}

BOOLEAN ext__CertificateUtil__verifyCertificateChainWithIntermediate(const OCTETSTRING& cert,
								     const OCTETSTRING& intermediateCert,
								     const OCTETSTRING& rootCA) {
	return cert_bool_wrapper("ext__CertificateUtil__verifyCertificateChainWithIntermediate", [&]() {
		std::vector<uint8_t> certDer = octetstring_to_bytes(cert);
		std::vector<uint8_t> intermediateDer = octetstring_to_bytes(intermediateCert);
		std::vector<uint8_t> rootDer = octetstring_to_bytes(rootCA);

		auto certObj = CertificateUtil::loadCertFromDER(certDer);
		auto intermediateObj = CertificateUtil::loadCertFromDER(intermediateDer);
		auto rootObj = CertificateUtil::loadCertFromDER(rootDer);

		// Create a certificate pool containing just the intermediate certificate
		std::vector<X509*> certPool = { intermediateObj.get() };

		return CertificateUtil::verifyCertificateChainDynamic(certObj.get(), certPool, rootObj.get(),
								      false); // verbose=false
	});
}

void ext__logInfo(const CHARSTRING& message) {
	log_wrapper<Logger::info>("ext__logInfo", message);
}

void ext__logError(const CHARSTRING& message) {
	log_wrapper<Logger::error>("ext__logError", message);
}

void ext__logDebug(const CHARSTRING& message) {
	log_wrapper<Logger::debug>("ext__logDebug", message);
}

ProcessedBoundProfilePackage ext__BSP__processBoundProfilePackage(const OCTETSTRING& sharedSecret,
								  const INTEGER& keyType, const INTEGER& keyLength,
								  const OCTETSTRING& hostId, const CHARSTRING& eid,
								  const OCTETSTRING& encodedBoundProfilePackage) {
	auto makeErrorResult = []() {
		ProcessedBoundProfilePackage error_result;
		error_result.configureIsdp() = OCTETSTRING(0, nullptr);
		error_result.storeMetadata() = OCTETSTRING(0, nullptr);
		error_result.hasReplaceSessionKeys() = BOOLEAN(false);
		error_result.profileData() = OCTETSTRING(0, nullptr);
		return error_result;
	};

	return safe_execute("ext__BSP__processBoundProfilePackage", makeErrorResult(), [&]() {
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
		LOG_DEBUG("BSP KDF inputs:");
		LOG_DEBUG("  Shared secret: " + HexUtil::bytesToHex(sharedSecretVec));
		LOG_DEBUG("  Key type: " + std::to_string(keyType.get_long_long_val()) + 
			  " (0x" + HexUtil::bytesToHex({static_cast<uint8_t>(keyType.get_long_long_val())}) + ")");
		LOG_DEBUG("  Key length: " + std::to_string(keyLength.get_long_long_val()));
		LOG_DEBUG("  Host ID: " + HexUtil::bytesToHex(hostIdVec));
		LOG_DEBUG("  EID: " + HexUtil::bytesToHex(eidVec));
		
		auto bsp = BspCryptoNS::BspCrypto::from_kdf(sharedSecretVec,
							    static_cast<uint8_t>(keyType.get_long_long_val()),
							    static_cast<uint8_t>(keyLength.get_long_long_val()),
							    hostIdVec, eidVec);

		LOG_DEBUG("BSP session keys derived successfully");
		LOG_DEBUG("Processing BoundProfilePackage of size: " + std::to_string(bppData.size()));

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
			ttcn_result.ppkInitialMCV() =
				bytes_to_octetstring(result.replaceSessionKeys.initialMacChainingValue);
			LOG_DEBUG("ReplaceSessionKeys present - PPK will be used for profile decryption");
		}

		LOG_DEBUG("BoundProfilePackage processed successfully");
		LOG_DEBUG("ConfigureISDP size: " + std::to_string(result.configureIsdp.size()));
		LOG_DEBUG("StoreMetadata size: " + std::to_string(result.storeMetadata.size()));
		LOG_DEBUG("Profile data size: " + std::to_string(result.profileData.size()));

		return ttcn_result;
	});
}

CHARSTRING ext__RSPClient__sendHttpsPost(const INTEGER& clientHandle, const CHARSTRING& endpoint,
					 const CHARSTRING& body, INTEGER& statusCode) {
	statusCode = INTEGER(0);

	return with_client(clientHandle, "ext__RSPClient__sendHttpsPost", CHARSTRING(""), [&](RSPClient* client) {
		int httpStatus = 0;
		std::string response =
			client->sendHttpsPost(charstring_to_string(endpoint), charstring_to_string(body), httpStatus);
		statusCode = INTEGER(httpStatus);
		return string_to_charstring(response);
	});
}

INTEGER ext__RSPClient__configureHttpClient(const INTEGER& clientHandle, const BOOLEAN& useCustomTlsCert,
					    const CHARSTRING& customTlsCertPath) {
	return with_client(clientHandle, "ext__RSPClient__configureHttpClient", INTEGER(-1), [&](RSPClient* client) {
		bool useCustomCert = static_cast<bool>(useCustomTlsCert);
		std::string certPath = charstring_to_string(customTlsCertPath);
		client->configureHttpClient(useCustomCert, certPath);
		LOG_DEBUG("Configured HTTP client - custom TLS cert: " + std::string(useCustomCert ? "true" : "false") +
			  (certPath.empty() ? "" : " (" + certPath + ")"));
		return INTEGER(0);
	});
}

} // namespace smdpp__Tests