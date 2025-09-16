/* BSP Crypto Implementation File
 *
 * Author: Eric Wild <ewild@sysmocom.de> / sysmocom - s.f.m.c. GmbH
 *
 * Released under the terms of GNU General Public License, Version 2 or
 * (at your option) any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "bsp_crypto.h"
#include <iostream>
#include <iomanip>
#include <cassert>
#include <stdexcept>

namespace BspCryptoNS {

// ASN.1 structure definitions
ASN1_SEQUENCE(ReplaceSessionKeysRequest_ASN1) = {
    ASN1_IMP(ReplaceSessionKeysRequest_ASN1, initialMacChainingValue, ASN1_OCTET_STRING, 0),
    ASN1_IMP(ReplaceSessionKeysRequest_ASN1, ppkEnc, ASN1_OCTET_STRING, 1),
    ASN1_IMP(ReplaceSessionKeysRequest_ASN1, ppkCmac, ASN1_OCTET_STRING, 2)
} ASN1_SEQUENCE_END(ReplaceSessionKeysRequest_ASN1)
IMPLEMENT_ASN1_FUNCTIONS(ReplaceSessionKeysRequest_ASN1)

ASN1_ITEM_TEMPLATE(ReplaceSessionKeysRequest_outer) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_CONTEXT, 38, ReplaceSessionKeysRequest_outer, ReplaceSessionKeysRequest_ASN1)
    ASN1_ITEM_TEMPLATE_END(ReplaceSessionKeysRequest_outer)
IMPLEMENT_ASN1_FUNCTIONS(ReplaceSessionKeysRequest_outer)

ASN1_ITEM_TEMPLATE(RemoteOpId) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_CONTEXT, 2, RemoteOpId, ASN1_INTEGER)
ASN1_ITEM_TEMPLATE_END(RemoteOpId)
IMPLEMENT_ASN1_FUNCTIONS(RemoteOpId)

ASN1_SEQUENCE(ControlRefTemplate) = {
    ASN1_IMP(ControlRefTemplate, keyType, ASN1_OCTET_STRING, 0),
    ASN1_IMP(ControlRefTemplate, keyLen, ASN1_OCTET_STRING, 1),
    ASN1_IMP(ControlRefTemplate, hostId, ASN1_OCTET_STRING, 4)
} ASN1_SEQUENCE_END(ControlRefTemplate)
IMPLEMENT_ASN1_FUNCTIONS(ControlRefTemplate)

ASN1_ITEM_TEMPLATE(SmdpOtpk) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_APPLICATION, 73, SmdpOtpk, ASN1_OCTET_STRING)
ASN1_ITEM_TEMPLATE_END(SmdpOtpk)
IMPLEMENT_ASN1_FUNCTIONS(SmdpOtpk)

ASN1_ITEM_TEMPLATE(SmdpSign) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_APPLICATION, 55, SmdpSign, ASN1_OCTET_STRING)
ASN1_ITEM_TEMPLATE_END(SmdpSign)
IMPLEMENT_ASN1_FUNCTIONS(SmdpSign)

ASN1_SEQUENCE(InitialiseSecureChannelRequest_ASN1) = {
    ASN1_SIMPLE(InitialiseSecureChannelRequest_ASN1, remoteOpId, RemoteOpId),
    ASN1_IMP(InitialiseSecureChannelRequest_ASN1, transactionId, ASN1_OCTET_STRING, 0),
    ASN1_IMP(InitialiseSecureChannelRequest_ASN1, controlRefTemplate, ControlRefTemplate, 6),
    ASN1_SIMPLE(InitialiseSecureChannelRequest_ASN1, smdpOtpk, SmdpOtpk),
    ASN1_SIMPLE(InitialiseSecureChannelRequest_ASN1, smdpSign, SmdpSign)
} ASN1_SEQUENCE_END(InitialiseSecureChannelRequest_ASN1)
IMPLEMENT_ASN1_FUNCTIONS(InitialiseSecureChannelRequest_ASN1)

ASN1_ITEM_TEMPLATE(InitialiseSecureChannelRequest_tagged) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_CONTEXT, 35, InitialiseSecureChannelRequest_tagged, InitialiseSecureChannelRequest_ASN1)
ASN1_ITEM_TEMPLATE_END(InitialiseSecureChannelRequest_tagged)
IMPLEMENT_ASN1_FUNCTIONS(InitialiseSecureChannelRequest_tagged)

ASN1_ITEM_TEMPLATE(ASN1_OCTET_STRING_TAG7) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_CONTEXT, 7, ASN1_OCTET_STRING_TAG7, ASN1_OCTET_STRING)
ASN1_ITEM_TEMPLATE_END(ASN1_OCTET_STRING_TAG7)
IMPLEMENT_ASN1_FUNCTIONS(ASN1_OCTET_STRING_TAG7)

ASN1_ITEM_TEMPLATE(ASN1_OCTET_STRING_TAG8) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_CONTEXT, 8, ASN1_OCTET_STRING_TAG8, ASN1_OCTET_STRING)
ASN1_ITEM_TEMPLATE_END(ASN1_OCTET_STRING_TAG8)
IMPLEMENT_ASN1_FUNCTIONS(ASN1_OCTET_STRING_TAG8)

ASN1_ITEM_TEMPLATE(ASN1_OCTET_STRING_TAG6) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_CONTEXT, 6, ASN1_OCTET_STRING_TAG6, ASN1_OCTET_STRING)
ASN1_ITEM_TEMPLATE_END(ASN1_OCTET_STRING_TAG6)
IMPLEMENT_ASN1_FUNCTIONS(ASN1_OCTET_STRING_TAG6)

ASN1_SEQUENCE(BoundProfilePackage) = {
    ASN1_SIMPLE(BoundProfilePackage, initialiseSecureChannelRequest, InitialiseSecureChannelRequest_tagged),
    ASN1_IMP_SEQUENCE_OF(BoundProfilePackage, firstSequenceOf87, ASN1_OCTET_STRING_TAG7, 0),
    ASN1_IMP_SEQUENCE_OF(BoundProfilePackage, sequenceOf88, ASN1_OCTET_STRING_TAG8, 1),
    ASN1_IMP_SEQUENCE_OF_OPT(BoundProfilePackage, secondSequenceOf87, ASN1_OCTET_STRING_TAG7, 2),
    ASN1_IMP_SEQUENCE_OF(BoundProfilePackage, sequenceOf86, ASN1_OCTET_STRING_TAG6, 3)
} ASN1_SEQUENCE_END(BoundProfilePackage)
IMPLEMENT_ASN1_FUNCTIONS(BoundProfilePackage)

ASN1_ITEM_TEMPLATE(BoundProfilePackage_tagged) =
    ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_IMPTAG | ASN1_TFLG_CONTEXT,54, BoundProfilePackage_tagged, BoundProfilePackage)
ASN1_ITEM_TEMPLATE_END(BoundProfilePackage_tagged)
IMPLEMENT_ASN1_FUNCTIONS(BoundProfilePackage_tagged)

// Custom deleter implementations
	void BoundProfilePackage_tagged_Deleter::operator()(BoundProfilePackage_tagged * ss1) const {
    BoundProfilePackage_tagged_free(ss1);
}

void ReplaceSessionKeysRequest_outer_Deleter::operator()(ReplaceSessionKeysRequest_outer* ss1) const {
    ReplaceSessionKeysRequest_outer_free(ss1);
}

// reusing a dirty pointer fails
#define ossl_free_reset(add)                                                                                                                                   \
	{                                                                                                                                                      \
		OPENSSL_free(add);                                                                                                                             \
		add = 0;                                                                                                                                       \
}

// BspCrypto implementation
BspCrypto::BspCrypto(const std::vector<uint8_t>& s_enc_key, const std::vector<uint8_t>& s_mac_key, const std::vector<uint8_t>& initial_mcv)
    : s_enc(s_enc_key), s_mac(s_mac_key), mac_chain(initial_mcv), block_number(1) {
    assert(s_enc.size() == AES_KEY_SIZE);
    assert(s_mac.size() == AES_KEY_SIZE);
    assert(mac_chain.size() == AES_BLOCK_SIZE);
}

std::vector<uint8_t> BspCrypto::compute_cmac(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data, size_t output_size) {
	EVP_MAC* mac = EVP_MAC_fetch(nullptr, "CMAC", nullptr);
	if (!mac)
		throw std::runtime_error("Failed to fetch CMAC implementation");

    EVP_MAC_CTX_unique_ptr mac_ctx(EVP_MAC_CTX_new(mac));
    EVP_MAC_free(mac);
	if (!mac_ctx)
		throw std::runtime_error("Failed to create MAC context");

    // Set up parameters for CMAC with AES-128-CBC
	OSSL_PARAM params[] = { OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_CIPHER, (char*)"AES-128-CBC", 0), OSSL_PARAM_construct_end() };

    if (EVP_MAC_init(mac_ctx.get(), key.data(), key.size(), params) != 1) {
        throw std::runtime_error("Failed to init MAC");
    }

    if (EVP_MAC_update(mac_ctx.get(), data.data(), data.size()) != 1) {
        throw std::runtime_error("Failed to update MAC");
    }

    std::vector<uint8_t> output(output_size);
    size_t mac_len = output_size;
    if (EVP_MAC_final(mac_ctx.get(), output.data(), &mac_len, output.size()) != 1) {
        throw std::runtime_error("Failed to finalize MAC");
    }

    output.resize(mac_len);
    return output;
}

std::vector<uint8_t> BspCrypto::aes_cipher_operation(const std::vector<uint8_t>& input, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv,
						     bool encrypt) {
    if (key.size() != AES_KEY_SIZE) {
        throw std::runtime_error("Invalid key size");
    }
    if (iv.size() != AES_BLOCK_SIZE) {
        throw std::runtime_error("Invalid IV size");
    }

    EVP_CIPHER_CTX_unique_ptr ctx(EVP_CIPHER_CTX_new());
	if (!ctx)
		throw std::runtime_error("Failed to create cipher context");

    const EVP_CIPHER* cipher = EVP_aes_128_cbc();

    int result;
    if (encrypt) {
        result = EVP_EncryptInit_ex(ctx.get(), cipher, nullptr, key.data(), iv.data());
    } else {
        result = EVP_DecryptInit_ex(ctx.get(), cipher, nullptr, key.data(), iv.data());
    }

    if (result != 1) {
        throw std::runtime_error(encrypt ? "Failed to init AES encryption" : "Failed to init AES decryption");
    }

    // Disable padding since we handle custom padding ourselves
    if (EVP_CIPHER_CTX_set_padding(ctx.get(), 0) != 1) {
        throw std::runtime_error("Failed to disable padding");
    }

    // Allocate output buffer (may need extra block for padding)
    std::vector<uint8_t> output(input.size() + AES_BLOCK_SIZE);
    int len = 0;

    if (encrypt) {
        result = EVP_EncryptUpdate(ctx.get(), output.data(), &len, input.data(), input.size());
    } else {
        result = EVP_DecryptUpdate(ctx.get(), output.data(), &len, input.data(), input.size());
    }

    if (result != 1) {
        throw std::runtime_error(encrypt ? "Failed to encrypt data" : "Failed to decrypt data");
    }

    // Verify output length matches input (since padding is disabled)
    if (len != static_cast<int>(input.size())) {
		throw std::runtime_error("Unexpected " + std::string(encrypt ? "encryption" : "decryption") + " output length: " + std::to_string(len) +
                               " expected: " + std::to_string(input.size()));
    }

    int final_len = 0;
    if (encrypt) {
        result = EVP_EncryptFinal_ex(ctx.get(), output.data() + len, &final_len);
    } else {
        result = EVP_DecryptFinal_ex(ctx.get(), output.data() + len, &final_len);
    }

    if (result != 1) {
        throw std::runtime_error(encrypt ? "Failed to finalize encryption" : "Failed to finalize decryption");
    }

    // With disabled padding, final_len should be 0
    if (final_len != 0) {
		throw std::runtime_error("Unexpected final " + std::string(encrypt ? "encryption" : "decryption") + " length: " + std::to_string(final_len));
    }

    output.resize(len);
    return output;
}

std::vector<uint8_t> BspCrypto::encode_bertlv_length(size_t length) {
    if (length < 0x80) {
		return { static_cast<uint8_t>(length) };
    } else if (length < 0x100) {
		return { 0x81, static_cast<uint8_t>(length) };
    } else if (length < 0x10000) {
		return { 0x82, static_cast<uint8_t>(length >> 8), static_cast<uint8_t>(length & 0xFF) };
    } else {
        throw std::runtime_error("Length too large for BERTLV encoding");
    }
}

void BspCrypto::print_hex(const std::string& label, const std::vector<uint8_t>& data) {
    std::cout << label << ": ";
    for (uint8_t b : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    std::cout << std::dec << std::endl;
}

void BspCrypto::print_hex(const char* label, const unsigned char* data, int len) {
	const std::vector<uint8_t> t = std::vector<uint8_t>((uint8_t*)data, (uint8_t*)data + len);
    const std::string l = std::string("").assign(label);
    print_hex(l, t);
}

std::pair<size_t, size_t> BspCrypto::parse_bertlv_length(const std::vector<uint8_t>& data, size_t offset) {
	if (offset >= data.size())
		throw std::runtime_error("Invalid length offset");

    uint8_t first_byte = data[offset];
    if (first_byte < 0x80) {
		return { first_byte, offset + 1 };
    } else if (first_byte == 0x81) {
		if (offset + 1 >= data.size())
			throw std::runtime_error("Invalid length encoding");
		return { data[offset + 1], offset + 2 };
    } else if (first_byte == 0x82) {
		if (offset + 2 >= data.size())
			throw std::runtime_error("Invalid length encoding");
        size_t length = (data[offset + 1] << 8) | data[offset + 2];
		return { length, offset + 3 };
    } else {
        throw std::runtime_error("Unsupported length encoding");
    }
}

std::vector<uint8_t> BspCrypto::x963_kdf_sha256(const std::vector<uint8_t>& shared_secret, const std::vector<uint8_t>& shared_info, size_t output_length) {
    std::vector<uint8_t> output;
    output.reserve(output_length);

    const size_t hash_length = 32; // SHA256
    uint32_t counter = 1;

    while (output.size() < output_length) {
        // input: shared_secret || counter || shared_info
        std::vector<uint8_t> hash_input;
        hash_input.insert(hash_input.end(), shared_secret.begin(), shared_secret.end());

        uint32_t counter_be = htonl(counter);
        const uint8_t* counter_bytes = reinterpret_cast<const uint8_t*>(&counter_be);
        hash_input.insert(hash_input.end(), counter_bytes, counter_bytes + 4);

        hash_input.insert(hash_input.end(), shared_info.begin(), shared_info.end());

        std::vector<uint8_t> hash_output(hash_length);
        if (SHA256(hash_input.data(), hash_input.size(), hash_output.data()) == nullptr) {
            throw std::runtime_error("SHA256 computation failed");
        }

        size_t bytes_needed = std::min(hash_length, output_length - output.size());
        output.insert(output.end(), hash_output.begin(), hash_output.begin() + bytes_needed);

        counter++;
    }

    // Truncate to desired length
    output.resize(output_length);
    return output;
}

BspCrypto BspCrypto::from_kdf(const std::vector<uint8_t>& shared_secret, uint8_t key_type, uint8_t key_length, const std::vector<uint8_t>& host_id,
			      const std::vector<uint8_t>& eid) {
    // shared_info: key_type || key_length || len(host_id) || host_id || len(eid) || eid
    std::vector<uint8_t> shared_info;
    shared_info.push_back(key_type);
    shared_info.push_back(key_length);

	if (host_id.size() > 255)
		throw std::runtime_error("Host ID too long");
    shared_info.push_back(static_cast<uint8_t>(host_id.size()));
    shared_info.insert(shared_info.end(), host_id.begin(), host_id.end());

	if (eid.size() > 255)
		throw std::runtime_error("EID too long");
    shared_info.push_back(static_cast<uint8_t>(eid.size()));
    shared_info.insert(shared_info.end(), eid.begin(), eid.end());

    // Derive 48 bytes: 16 for initial_mcv + 16 for s_enc + 16 for s_mac
    auto kdf_output = x963_kdf_sha256(shared_secret, shared_info, 48);

    std::vector<uint8_t> initial_mcv(kdf_output.begin(), kdf_output.begin() + 16);
    std::vector<uint8_t> s_enc(kdf_output.begin() + 16, kdf_output.begin() + 32);
    std::vector<uint8_t> s_mac(kdf_output.begin() + 32, kdf_output.begin() + 48);

    return BspCrypto(s_enc, s_mac, initial_mcv);
}

std::vector<uint8_t> BspCrypto::generate_icv() {
    // Use generate_icv_for_block and increment block_number
    auto icv = generate_icv_for_block(block_number);
    block_number++;
    return icv;
}

std::vector<uint8_t> BspCrypto::add_padding(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> padded = data;
    padded.push_back(0x80);

    while (padded.size() % AES_BLOCK_SIZE != 0) {
        padded.push_back(0x00);
    }

    return padded;
}

std::vector<uint8_t> BspCrypto::remove_padding(const std::vector<uint8_t>& data) {
  if (data.empty())
    return data;

// Remove trailing zeros first
  int last_nonzero = data.size() - 1;
  while (last_nonzero >= 0 && data[last_nonzero] == 0x00) {
    last_nonzero--;
  }
  if (last_nonzero >= 0 && data[last_nonzero] == 0x80) {
    return std::vector<uint8_t>(data.begin(), data.begin() + last_nonzero);
  }
  return data;
}

std::vector<uint8_t> BspCrypto::aes_encrypt(const std::vector<uint8_t>& plaintext) {
    auto padded = add_padding(plaintext);
    auto icv = generate_icv();
    return aes_cipher_operation(padded, s_enc, icv, true);
}

std::vector<uint8_t> BspCrypto::aes_decrypt_with_icv(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& icv) {
    auto decrypted = aes_cipher_operation(ciphertext, s_enc, icv, false);
    return remove_padding(decrypted);
}

std::vector<uint8_t> BspCrypto::generate_icv_for_block(uint32_t block_num) {
    std::vector<uint8_t> block_data(AES_BLOCK_SIZE, 0);

    uint32_t block_num_be = htonl(block_num);
    memcpy(&block_data[12], &block_num_be, 4);

    std::vector<uint8_t> zero_iv(AES_BLOCK_SIZE, 0);
    return aes_cipher_operation(block_data, s_enc, zero_iv, true);
}

std::vector<uint8_t> BspCrypto::verify_and_decrypt_helper(const std::vector<uint8_t>& segment, const std::vector<uint8_t>& mac_chain_to_use, bool decrypt) {
	if (segment.size() < 3)
		throw std::runtime_error("Segment too small");

    uint8_t tag = segment[0];
    auto [length, length_end] = parse_bertlv_length(segment, 1);

    if (length_end + length > segment.size()) {
        throw std::runtime_error("Invalid segment length");
    }

    if (length <= MAC_LENGTH) {
        throw std::runtime_error("Invalid segment length: payload too small");
    }

    size_t payload_length = length - MAC_LENGTH;
    std::vector<uint8_t> payload(segment.begin() + length_end, segment.begin() + length_end + payload_length);
	std::vector<uint8_t> received_mac(segment.begin() + length_end + payload_length, segment.begin() + length_end + length);

    size_t lcc = payload.size() + MAC_LENGTH;
    std::vector<uint8_t> temp_data;
    temp_data.insert(temp_data.end(), mac_chain_to_use.begin(), mac_chain_to_use.end());
    temp_data.push_back(tag);

    auto length_bytes = encode_bertlv_length(lcc);
    temp_data.insert(temp_data.end(), length_bytes.begin(), length_bytes.end());
    temp_data.insert(temp_data.end(), payload.begin(), payload.end());

    std::vector<uint8_t> computed_full_mac = compute_cmac(s_mac, temp_data);
    std::vector<uint8_t> computed_mac(computed_full_mac.begin(), computed_full_mac.begin() + MAC_LENGTH);

    if (received_mac != computed_mac) {
        throw std::runtime_error("MAC verification failed");
    }

    // Update instance state
    mac_chain = computed_full_mac;

    if (decrypt) {
        // generate_icv() increments block_number
        auto icv = generate_icv();
        return aes_decrypt_with_icv(payload, icv);
    } else {
        // MAC-only: return payload as-is and increment block counter
        block_number++;
        return payload;
    }
}

std::vector<uint8_t> BspCrypto::compute_mac(uint8_t tag, const std::vector<uint8_t>& data) {
    size_t lcc = data.size() + MAC_LENGTH;

    // temp_data: mac_chain + tag + length + data
    std::vector<uint8_t> temp_data;
    temp_data.insert(temp_data.end(), mac_chain.begin(), mac_chain.end());
    temp_data.push_back(tag);

    auto length_bytes = encode_bertlv_length(lcc);
    temp_data.insert(temp_data.end(), length_bytes.begin(), length_bytes.end());
    temp_data.insert(temp_data.end(), data.begin(), data.end());

    // Compute CMAC
    std::vector<uint8_t> full_mac = compute_cmac(s_mac, temp_data);

    mac_chain = full_mac;

    // truncated MAC (first 8 bytes)
    return std::vector<uint8_t>(full_mac.begin(), full_mac.begin() + MAC_LENGTH);
}

std::vector<uint8_t> BspCrypto::encrypt_and_mac_one(uint8_t tag, const std::vector<uint8_t>& plaintext) {
    if (plaintext.size() > MAX_SEGMENT_SIZE - 10) { // Account for tag, length, MAC
        throw std::runtime_error("Segment too large");
    }

    auto ciphertext = aes_encrypt(plaintext);
    auto mac = compute_mac(tag, ciphertext);

    // final result: tag + length + ciphertext + mac
    std::vector<uint8_t> result;
    result.push_back(tag);

    auto length_bytes = encode_bertlv_length(ciphertext.size() + MAC_LENGTH);
    result.insert(result.end(), length_bytes.begin(), length_bytes.end());
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());
    result.insert(result.end(), mac.begin(), mac.end());

    return result;
}

std::vector<uint8_t> BspCrypto::decrypt_and_verify(const std::vector<uint8_t>& segments, bool decrypt) {
    return verify_and_decrypt_helper(segments, mac_chain, decrypt);
}

std::vector<uint8_t> BspCrypto::asn1_octet_string_to_vector(const ASN1_OCTET_STRING* asn1_str) {
    if (!asn1_str || !asn1_str->data || asn1_str->length <= 0) {
        throw std::runtime_error("Invalid ASN1_OCTET_STRING");
    }

    return std::vector<uint8_t>(asn1_str->data, asn1_str->data + asn1_str->length);
}

BspCrypto::ReplaceSessionKeysRequest BspCrypto::parse_replace_session_keys(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        throw std::runtime_error("Empty ReplaceSessionKeysRequest data");
    }

	const unsigned char* p = data.data();

    RPK_ptr rsk_asn1(d2i_ReplaceSessionKeysRequest_outer(nullptr, &p, static_cast<int>(data.size())));

    if (!rsk_asn1) {
        unsigned long err = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));

        std::string error_msg = "Failed to parse ReplaceSessionKeysRequest ASN.1: ";
        error_msg += err_buf;
        throw std::runtime_error(error_msg);
    }

    try {
        if (!rsk_asn1->initialMacChainingValue || !rsk_asn1->ppkEnc || !rsk_asn1->ppkCmac) {
            throw std::runtime_error("Missing required fields in ReplaceSessionKeysRequest");
        }

        if (rsk_asn1->initialMacChainingValue->length != 16) {
            throw std::runtime_error("Invalid initialMacChainingValue length: expected 16 bytes");
        }
        if (rsk_asn1->ppkEnc->length != 16) {
            throw std::runtime_error("Invalid ppkEnc length: expected 16 bytes");
        }
        if (rsk_asn1->ppkCmac->length != 16) {
            throw std::runtime_error("Invalid ppkCmac length: expected 16 bytes");
        }

        ReplaceSessionKeysRequest result;
        result.initialMacChainingValue = asn1_octet_string_to_vector(rsk_asn1->initialMacChainingValue);
        result.ppkEnc = asn1_octet_string_to_vector(rsk_asn1->ppkEnc);
        result.ppkCmac = asn1_octet_string_to_vector(rsk_asn1->ppkCmac);

        return result;

    } catch (...) {
        throw;
    }
}

BspCrypto BspCrypto::from_replace_session_keys(const ReplaceSessionKeysRequest& rsk) {
    return BspCrypto(rsk.ppkEnc, rsk.ppkCmac, rsk.initialMacChainingValue);
}

void BspCrypto::reset(const std::vector<uint8_t>& initial_mcv) {
    mac_chain = initial_mcv;
    block_number = 1;
}

std::vector<uint8_t> BspCrypto::generate_replace_session_keys_data(const std::vector<uint8_t>& ppk_enc, const std::vector<uint8_t>& ppk_mac,
								   const std::vector<uint8_t>& initial_mcv) {
    if (ppk_enc.size() != 16 || ppk_mac.size() != 16 || initial_mcv.size() != 16) {
        throw std::runtime_error("All PPK components must be 16 bytes");
    }

    std::vector<uint8_t> rsk_data;
    rsk_data.insert(rsk_data.end(), ppk_enc.begin(), ppk_enc.end());
    rsk_data.insert(rsk_data.end(), ppk_mac.begin(), ppk_mac.end());
    rsk_data.insert(rsk_data.end(), initial_mcv.begin(), initial_mcv.end());

    return rsk_data;
}

std::vector<uint8_t> BspCrypto::mac_only_one(uint8_t tag, const std::vector<uint8_t>& plaintext) {
    if (plaintext.size() > MAX_SEGMENT_SIZE - 10) { // Account for tag, length, MAC
        throw std::runtime_error("Segment too large");
    }

    auto mac = compute_mac(tag, plaintext);

    // final result: tag + length + plaintext + mac
    std::vector<uint8_t> result;
    result.push_back(tag);

    auto length_bytes = encode_bertlv_length(plaintext.size() + MAC_LENGTH);
    result.insert(result.end(), length_bytes.begin(), length_bytes.end());
    result.insert(result.end(), plaintext.begin(), plaintext.end());
    result.insert(result.end(), mac.begin(), mac.end());

    block_number++;

    return result;
}

std::vector<std::vector<uint8_t>> BspCrypto::encrypt_and_mac_seg(uint8_t tag, const std::vector<uint8_t>& data) {
    std::vector<std::vector<uint8_t>> segments;
    size_t max_payload = MAX_SEGMENT_SIZE - 10; // Account for overhead

    for (size_t offset = 0; offset < data.size(); offset += max_payload) {
        size_t segment_size = std::min(max_payload, data.size() - offset);
        std::vector<uint8_t> segment(data.begin() + offset, data.begin() + offset + segment_size);
        segments.push_back(encrypt_and_mac_one(tag, segment));
    }

    return segments;
}

std::vector<std::vector<uint8_t>> BspCrypto::mac_only_seg(uint8_t tag, const std::vector<uint8_t>& data) {
    std::vector<std::vector<uint8_t>> segments;
    size_t max_payload = MAX_SEGMENT_SIZE - 10; // Account for overhead

    for (size_t offset = 0; offset < data.size(); offset += max_payload) {
        size_t segment_size = std::min(max_payload, data.size() - offset);
        std::vector<uint8_t> segment(data.begin() + offset, data.begin() + offset + segment_size);
        segments.push_back(mac_only_one(tag, segment));
    }

    return segments;
}

BspCrypto::BppProcessingResult BspCrypto::process_bound_profile_package(const std::vector<uint8_t>& allofit) {
    BppProcessingResult result;
    auto p = allofit.data();
    BPP_ptr bpp(d2i_BoundProfilePackage_tagged(nullptr, &p, allofit.size()));
    if (!bpp) {
        std::cout << "Failed to decode BoundProfilePackage" << std::endl;
        print_hex("  Data", allofit.data(), (allofit.size() > 32) ? 32 : allofit.size());
        return result;
    }

    // all mandatory
	if (bpp->firstSequenceOf87 == nullptr || bpp->sequenceOf88 == nullptr || bpp->sequenceOf86 == nullptr) {
            std::cout << "Malformed BoundProfilePackage" << std::endl;
            return result;
    }

    result.hasReplaceSessionKeys = bpp->secondSequenceOf87 != nullptr;

	unsigned char* encoded = nullptr;
	ASN1_OCTET_STRING* elem = nullptr;
    int len = 0;

    // Step 1: Decrypt ConfigureISDP with session keys
    std::cout << "Step 1: Decrypting ConfigureISDP with session keys..." << std::endl;
    elem = sk_ASN1_OCTET_STRING_value(bpp->firstSequenceOf87, 0);
    len = i2d_ASN1_OCTET_STRING_TAG7(elem, &encoded);
	result.configureIsdp = decrypt_and_verify({ encoded, encoded + len }, true);
    ossl_free_reset(encoded);

    // Step 2: Verify StoreMetadata with session keys (MAC-only)
    std::cout << "Step 2: Verifying StoreMetadata with session keys (MAC-only)..." << std::endl;
    int num_metadata = sk_ASN1_OCTET_STRING_num(bpp->sequenceOf88);
    for (int i = 0; i < num_metadata; i++) {
        elem = sk_ASN1_OCTET_STRING_value(bpp->sequenceOf88, i);
        len = i2d_ASN1_OCTET_STRING_TAG8(elem, &encoded);
        auto rv = decrypt_and_verify({ encoded, encoded + len }, false);
        result.storeMetadata.insert(result.storeMetadata.end(), rv.begin(), rv.end());
        ossl_free_reset(encoded);
    }
    std::cout << "Step 2: " << num_metadata << " metadata chunks verified" << std::endl;

    // Step 3: If present, decrypt ReplaceSessionKeys with session keys
    if (result.hasReplaceSessionKeys) {
        std::cout << "Step 3: Decrypting ReplaceSessionKeys with session keys..." << std::endl;

        elem = sk_ASN1_OCTET_STRING_value(bpp->secondSequenceOf87, 0);
        len = i2d_ASN1_OCTET_STRING_TAG7(elem, &encoded);
		auto rsk_data = decrypt_and_verify({ encoded, encoded + len }, true);
        result.replaceSessionKeys = parse_replace_session_keys(rsk_data);
        ossl_free_reset(encoded);

        // Step 4: Create NEW BSP instance with PPK and decrypt profile data
        std::cout << "Step 4: Creating new BSP instance with PPK keys..." << std::endl;
        auto ppk_bsp = from_replace_session_keys(result.replaceSessionKeys);

        print_hex("PPK-ENC", result.replaceSessionKeys.ppkEnc);
        print_hex("PPK-MAC", result.replaceSessionKeys.ppkCmac);
        print_hex("PPK Initial MCV", result.replaceSessionKeys.initialMacChainingValue);

		std::cout << "Step 5: Decrypting profile data with PPK keys..." << std::endl;
        int num = sk_ASN1_OCTET_STRING_num(bpp->sequenceOf86);
        for (int i = 0; i < num; i++) {
            elem = sk_ASN1_OCTET_STRING_value(bpp->sequenceOf86, i);
            len = i2d_ASN1_OCTET_STRING_TAG6(elem, &encoded);
			auto rv = ppk_bsp.decrypt_and_verify({ encoded, encoded + len }, true);
            result.profileData.insert(result.profileData.end(), rv.begin(), rv.end());
            ossl_free_reset(encoded);
        }
		std::cout << "Step 5: " << num << " profile chunks verified and decrypted" << std::endl;

    } else {
        // No ReplaceSessionKeys - decrypt profile data with session keys
        std::cout << "Step 3: Decrypting profile data with session keys (no PPK)..." << std::endl;
        int num = sk_ASN1_OCTET_STRING_num(bpp->sequenceOf86);
        for (int i = 0; i < num; i++) {
            elem = sk_ASN1_OCTET_STRING_value(bpp->sequenceOf86, i);
            len = i2d_ASN1_OCTET_STRING_TAG6(elem, &encoded);
			auto rv = decrypt_and_verify({ encoded, encoded + len }, true);
            result.profileData.insert(result.profileData.end(), rv.begin(), rv.end());
            ossl_free_reset(encoded);
        }
		std::cout << "Step 3: " << num << " profile chunks verified and decrypted" << std::endl;
    }

    return result;
}

BspCrypto::BppProcessingResult BspCrypto::process_bound_profile_package(const std::vector<uint8_t>& firstSequenceOf87, // ConfigureISDP
	const std::vector<uint8_t>& sequenceOf88, // StoreMetadata
    const std::vector<uint8_t>& secondSequenceOf87, // ReplaceSessionKeys (optional)
	const std::vector<uint8_t>& sequenceOf86 // Profile data
) {
    BppProcessingResult result;
    result.hasReplaceSessionKeys = !secondSequenceOf87.empty();

    // Step 1: Decrypt ConfigureISDP with session keys
    std::cout << "Step 1: Decrypting ConfigureISDP with session keys..." << std::endl;
    result.configureIsdp = decrypt_and_verify(firstSequenceOf87, true);

    // Step 2: Verify StoreMetadata with session keys (MAC-only)
    std::cout << "Step 2: Verifying StoreMetadata with session keys (MAC-only)..." << std::endl;
    result.storeMetadata = decrypt_and_verify(sequenceOf88, false);

    // Step 3: If present, decrypt ReplaceSessionKeys with session keys
    if (result.hasReplaceSessionKeys) {
        std::cout << "Step 3: Decrypting ReplaceSessionKeys with session keys..." << std::endl;
        auto rsk_data = decrypt_and_verify(secondSequenceOf87, true);
        result.replaceSessionKeys = parse_replace_session_keys(rsk_data);

        // Step 4: Create NEW BSP instance with PPK and decrypt profile data
        std::cout << "Step 4: Creating new BSP instance with PPK keys..." << std::endl;
        auto ppk_bsp = from_replace_session_keys(result.replaceSessionKeys);

        print_hex("PPK-ENC", result.replaceSessionKeys.ppkEnc);
        print_hex("PPK-MAC", result.replaceSessionKeys.ppkCmac);
        print_hex("PPK Initial MCV", result.replaceSessionKeys.initialMacChainingValue);

        std::cout << "Step 5: Decrypting profile data with PPK keys..." << std::endl;
        result.profileData = ppk_bsp.decrypt_and_verify(sequenceOf86, true);

    } else {
        // No ReplaceSessionKeys - decrypt profile data with session keys
        std::cout << "Step 3: Decrypting profile data with session keys (no PPK)..." << std::endl;
        result.profileData = decrypt_and_verify(sequenceOf86, true);
    }

    return result;
}

} // namespace BspCryptoNS