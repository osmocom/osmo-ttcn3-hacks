/* BSP Crypto Header File
 *
 * Author: Eric Wild <ewild@sysmocom.de> / sysmocom - s.f.m.c. GmbH
 *
 * Released under the terms of GNU General Public License, Version 2 or
 * (at your option) any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BSP_CRYPTO_H
#define BSP_CRYPTO_H

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <cstring>
#include <arpa/inet.h>

extern "C" {
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/params.h>
#include <openssl/core_names.h>
#include <openssl/kdf.h>
#include <openssl/sha.h>
#include <openssl/opensslv.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/err.h>
}

namespace BspCryptoNS {

// ASN.1 structures for ReplaceSessionKeysRequest
typedef struct ReplaceSessionKeysRequest_ASN1_st {
	ASN1_OCTET_STRING* initialMacChainingValue;
	ASN1_OCTET_STRING* ppkEnc;
	ASN1_OCTET_STRING* ppkCmac;
} ReplaceSessionKeysRequest_ASN1;

DECLARE_ASN1_FUNCTIONS(ReplaceSessionKeysRequest_ASN1)

typedef ReplaceSessionKeysRequest_ASN1 ReplaceSessionKeysRequest_outer;
DECLARE_ASN1_FUNCTIONS(ReplaceSessionKeysRequest_outer)

// Define stack macros for ASN1_OCTET_STRING
DEFINE_STACK_OF(ASN1_OCTET_STRING)

// RemoteOpId ::= [2] INTEGER
typedef ASN1_INTEGER RemoteOpId;
DECLARE_ASN1_FUNCTIONS(RemoteOpId)

// ControlRefTemplate
typedef struct ControlRefTemplate_st {
	ASN1_OCTET_STRING* keyType;
	ASN1_OCTET_STRING* keyLen;
	ASN1_OCTET_STRING* hostId;
} ControlRefTemplate;

DECLARE_ASN1_FUNCTIONS(ControlRefTemplate)

// Custom types for APPLICATION tags > 30
typedef ASN1_OCTET_STRING SmdpOtpk;
DECLARE_ASN1_FUNCTIONS(SmdpOtpk)

typedef ASN1_OCTET_STRING SmdpSign;
DECLARE_ASN1_FUNCTIONS(SmdpSign)

// InitialiseSecureChannelRequest - base structure
typedef struct InitialiseSecureChannelRequest_ASN1_st {
	RemoteOpId* remoteOpId;
	ASN1_OCTET_STRING* transactionId;
	ControlRefTemplate* controlRefTemplate;
	SmdpOtpk* smdpOtpk;
	SmdpSign* smdpSign;
} InitialiseSecureChannelRequest_ASN1;

DECLARE_ASN1_FUNCTIONS(InitialiseSecureChannelRequest_ASN1)

// InitialiseSecureChannelRequest with tag [35]
typedef InitialiseSecureChannelRequest_ASN1 InitialiseSecureChannelRequest_tagged;
DECLARE_ASN1_FUNCTIONS(InitialiseSecureChannelRequest_tagged)

// Define the tagged OCTET STRING types
typedef ASN1_OCTET_STRING ASN1_OCTET_STRING_TAG7;
typedef ASN1_OCTET_STRING ASN1_OCTET_STRING_TAG8;
typedef ASN1_OCTET_STRING ASN1_OCTET_STRING_TAG6;

DECLARE_ASN1_ITEM(ASN1_OCTET_STRING_TAG7)
DECLARE_ASN1_ITEM(ASN1_OCTET_STRING_TAG8)
DECLARE_ASN1_ITEM(ASN1_OCTET_STRING_TAG6)

DECLARE_ASN1_FUNCTIONS(ASN1_OCTET_STRING_TAG7)
DECLARE_ASN1_FUNCTIONS(ASN1_OCTET_STRING_TAG8)
DECLARE_ASN1_FUNCTIONS(ASN1_OCTET_STRING_TAG6)

// BoundProfilePackage
typedef struct BoundProfilePackage_st {
	InitialiseSecureChannelRequest_tagged* initialiseSecureChannelRequest;
	STACK_OF(ASN1_OCTET_STRING) * firstSequenceOf87;
	STACK_OF(ASN1_OCTET_STRING) * sequenceOf88;
	STACK_OF(ASN1_OCTET_STRING) * secondSequenceOf87;
	STACK_OF(ASN1_OCTET_STRING) * sequenceOf86;
} BoundProfilePackage;

DECLARE_ASN1_FUNCTIONS(BoundProfilePackage)

// For BoundProfilePackage ::= [54] SEQUENCE
typedef BoundProfilePackage BoundProfilePackage_tagged;
DECLARE_ASN1_FUNCTIONS(BoundProfilePackage_tagged)

// Custom deleters
struct BoundProfilePackage_tagged_Deleter {
	void operator()(BoundProfilePackage_tagged* ss1) const;
};

struct ReplaceSessionKeysRequest_outer_Deleter {
	void operator()(ReplaceSessionKeysRequest_outer* ss1) const;
};

struct EVP_CIPHER_CTX_Deleter {
	void operator()(EVP_CIPHER_CTX* ctx) const {
		if (ctx)
			EVP_CIPHER_CTX_free(ctx);
	}
};

struct EVP_MAC_CTX_Deleter {
	void operator()(EVP_MAC_CTX* ctx) const {
		if (ctx)
			EVP_MAC_CTX_free(ctx);
	}
};

struct EVP_MAC_Deleter {
	void operator()(EVP_MAC* mac) const {
		if (mac)
			EVP_MAC_free(mac);
	}
};

using BPP_ptr = std::unique_ptr<BoundProfilePackage_tagged, BoundProfilePackage_tagged_Deleter>;
using RPK_ptr = std::unique_ptr<ReplaceSessionKeysRequest_outer, ReplaceSessionKeysRequest_outer_Deleter>;
using EVP_CIPHER_CTX_unique_ptr = std::unique_ptr<EVP_CIPHER_CTX, EVP_CIPHER_CTX_Deleter>;
using EVP_MAC_CTX_unique_ptr = std::unique_ptr<EVP_MAC_CTX, EVP_MAC_CTX_Deleter>;
using EVP_MAC_unique_ptr = std::unique_ptr<EVP_MAC, EVP_MAC_Deleter>;

class BspCrypto {
    private:
	static const size_t MAX_SEGMENT_SIZE = 1020;
	static const size_t MAC_LENGTH = 8;
	static const size_t AES_KEY_SIZE = 16;

	std::vector<uint8_t> s_enc;
	std::vector<uint8_t> s_mac;
	std::vector<uint8_t> mac_chain;
	uint32_t block_number;

	static std::vector<uint8_t> compute_cmac(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data, size_t output_size = AES_BLOCK_SIZE);

	std::vector<uint8_t> aes_cipher_operation(const std::vector<uint8_t>& input, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv,
						  bool encrypt);

	static std::vector<uint8_t> encode_bertlv_length(size_t length);

	static void print_hex(const std::string& label, const std::vector<uint8_t>& data);
	static void print_hex(const char* label, const unsigned char* data, int len);

	static std::pair<size_t, size_t> parse_bertlv_length(const std::vector<uint8_t>& data, size_t offset);

	std::vector<uint8_t> verify_and_decrypt_helper(const std::vector<uint8_t>& segment, const std::vector<uint8_t>& mac_chain_to_use, bool decrypt);

    public:
	BspCrypto(const std::vector<uint8_t>& s_enc_key, const std::vector<uint8_t>& s_mac_key, const std::vector<uint8_t>& initial_mcv);

	static std::vector<uint8_t> x963_kdf_sha256(const std::vector<uint8_t>& shared_secret, const std::vector<uint8_t>& shared_info, size_t output_length);

	static BspCrypto from_kdf(const std::vector<uint8_t>& shared_secret, uint8_t key_type, uint8_t key_length, const std::vector<uint8_t>& host_id,
				  const std::vector<uint8_t>& eid);

	std::vector<uint8_t> generate_icv();
	std::vector<uint8_t> add_padding(const std::vector<uint8_t>& data);
	std::vector<uint8_t> remove_padding(const std::vector<uint8_t>& data);
	std::vector<uint8_t> aes_encrypt(const std::vector<uint8_t>& plaintext);
	std::vector<uint8_t> aes_decrypt_with_icv(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& icv);
	std::vector<uint8_t> generate_icv_for_block(uint32_t block_num);
	std::vector<uint8_t> compute_mac(uint8_t tag, const std::vector<uint8_t>& data);
	std::vector<uint8_t> encrypt_and_mac_one(uint8_t tag, const std::vector<uint8_t>& plaintext);
	std::vector<uint8_t> decrypt_and_verify(const std::vector<uint8_t>& segments, bool decrypt = true);

	struct ReplaceSessionKeysRequest {
		std::vector<uint8_t> ppkEnc;
		std::vector<uint8_t> ppkCmac;
		std::vector<uint8_t> initialMacChainingValue;
	};

	static std::vector<uint8_t> asn1_octet_string_to_vector(const ASN1_OCTET_STRING* asn1_str);
	static ReplaceSessionKeysRequest parse_replace_session_keys(const std::vector<uint8_t>& data);
	static BspCrypto from_replace_session_keys(const ReplaceSessionKeysRequest& rsk);

	struct BppProcessingResult {
		std::vector<uint8_t> configureIsdp;
		std::vector<uint8_t> storeMetadata;
		ReplaceSessionKeysRequest replaceSessionKeys;
		std::vector<uint8_t> profileData;
		bool hasReplaceSessionKeys;
	};

	BppProcessingResult process_bound_profile_package(const std::vector<uint8_t>& allofit);

	BppProcessingResult process_bound_profile_package(const std::vector<uint8_t>& firstSequenceOf87, const std::vector<uint8_t>& sequenceOf88,
							  const std::vector<uint8_t>& secondSequenceOf87, const std::vector<uint8_t>& sequenceOf86);

	void reset(const std::vector<uint8_t>& initial_mcv);

	static std::vector<uint8_t> generate_replace_session_keys_data(const std::vector<uint8_t>& ppk_enc, const std::vector<uint8_t>& ppk_mac,
								       const std::vector<uint8_t>& initial_mcv);

	std::vector<uint8_t> mac_only_one(uint8_t tag, const std::vector<uint8_t>& plaintext);
	std::vector<std::vector<uint8_t>> encrypt_and_mac_seg(uint8_t tag, const std::vector<uint8_t>& data);
	std::vector<std::vector<uint8_t>> mac_only_seg(uint8_t tag, const std::vector<uint8_t>& data);
};

} // namespace BspCryptoNS

#endif // BSP_CRYPTO_H