/*
 * 3GPP AKA - Milenage algorithm (3GPP TS 35.205, .206, .207, .208)
 * Copyright (c) 2006-2007 <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This file implements an example authentication algorithm defined for 3GPP
 * AKA. This can be used to implement a simple HLR/AuC into hlr_auc_gw to allow
 * EAP-AKA to be tested properly with real USIM cards.
 *
 * This implementations assumes that the r1..r5 and c1..c5 constants defined in
 * TS 35.206 are used, i.e., r1=64, r2=0, r3=32, r4=64, r5=96, c1=00..00,
 * c2=00..01, c3=00..02, c4=00..04, c5=00..08. The block cipher is assumed to
 * be AES (Rijndael).
 */


#include "milenage.h"

#include <openssl/err.h>            /* for ERR_print_errors_fp */
#include <openssl/ssl.h>            /* for NID_sha1, RSA */
#include <openssl/evp.h>            /* for EVP_PKEY, EVP_sha1(), ... */
#include <openssl/md5.h>            /* for MD5_DIGEST_LENGTH */
#include <openssl/sha.h>            /* for SHA_DIGEST_LENGTH */

#define AST_CRYPTO_RSA_KEY_BITS         1024
#define AST_CRYPTO_AES_BLOCKSIZE        128

struct ast_aes {
	unsigned char raw[AST_CRYPTO_AES_BLOCKSIZE / 8];
};

int ast_aes_set_encrypt_key(const unsigned char *key, struct ast_aes *ctx)
{
	if (key == NULL || ctx == NULL)
		return -1;

	memcpy(ctx->raw, key, AST_CRYPTO_AES_BLOCKSIZE / 8);
	return 0;
}

static int evp_cipher_aes_encrypt(const unsigned char *in, unsigned char *out, unsigned inlen, const struct ast_aes *key)
{
	EVP_CIPHER_CTX *ctx = NULL;
	int res, outlen, finallen;
	unsigned char final[AST_CRYPTO_AES_BLOCKSIZE / 8];

	if ((ctx = EVP_CIPHER_CTX_new()) == NULL)
		return -1;

	do {
		if ((res = EVP_CipherInit(ctx, EVP_aes_128_ecb(), key->raw, NULL, 1)) <= 0)
			break;
		EVP_CIPHER_CTX_set_padding(ctx, 0);
		if ((res = EVP_CipherUpdate(ctx, out, &outlen, in, inlen)) <= 0)
			break;
		/* for ECB, this is a no-op */
		if ((res = EVP_CipherFinal(ctx, final, &finallen)) <= 0)
			break;
		res = outlen;
	} while (0);

	EVP_CIPHER_CTX_free(ctx);

	return res;
}

int ast_aes_encrypt(const unsigned char *in, unsigned char *out, const struct ast_aes *key)
{
	int res;

	if ((res = evp_cipher_aes_encrypt(in, out, AST_CRYPTO_AES_BLOCKSIZE / 8, key)) <= 0) {
		printf("AES encryption failed\n");
		exit(0);
	}
	return res;
}

static int aes_128_encrypt_block(const u8 *key, const u8 *plain, u8 *encr)
{
	struct ast_aes aes_key;

	ast_aes_set_encrypt_key(key, &aes_key);
	if (ast_aes_encrypt(plain, encr, &aes_key) <= 0) {
		printf("Failed to ecrypt AES 128.");
		return -1;
	}

	return 0;
}

//#define DEBUG
#ifdef DEBUG
void hexdump(const char *text, const uint8_t *data, int len)
{
	char s[3 * len + 2], *p;
	int f;

	for (p = s, f = 0; f < len; f++, p += 3)
		sprintf(p, "%02hhX ", (unsigned char)data[f]);

	printf("%s: %s\n", text, s);
}
#endif


/**
 * milenage_f1 - Milenage f1 and f1* algorithms
 * @opc: OPc = 128-bit value derived from OP and K
 * @k: K = 128-bit subscriber key
 * @_rand: RAND = 128-bit random challenge
 * @sqn: SQN = 48-bit sequence number
 * @amf: AMF = 16-bit authentication management field
 * @mac_a: Buffer for MAC-A = 64-bit network authentication code, or %NULL
 * @mac_s: Buffer for MAC-S = 64-bit resync authentication code, or %NULL
 * Returns: 0 on success, -1 on failure
 */
int milenage_f1(const u8 *opc, const u8 *k, const u8 *_rand,
		const u8 *sqn, const u8 *amf, u8 *mac_a, u8 *mac_s)
{
	u8 tmp1[16], tmp2[16], tmp3[16];
	int i;

	/* tmp1 = TEMP = E_K(RAND XOR OP_C) */
	for (i = 0; i < 16; i++)
		tmp1[i] = _rand[i] ^ opc[i];
	if (aes_128_encrypt_block(k, tmp1, tmp1))
		return -1;

	/* tmp2 = IN1 = SQN || AMF || SQN || AMF */
	memcpy(tmp2, sqn, 6);
	memcpy(tmp2 + 6, amf, 2);
	memcpy(tmp2 + 8, tmp2, 8);

	/* OUT1 = E_K(TEMP XOR rot(IN1 XOR OP_C, r1) XOR c1) XOR OP_C */

	/* rotate (tmp2 XOR OP_C) by r1 (= 0x40 = 8 bytes) */
	for (i = 0; i < 16; i++)
		tmp3[(i + 8) % 16] = tmp2[i] ^ opc[i];
	/* XOR with TEMP = E_K(RAND XOR OP_C) */
	for (i = 0; i < 16; i++)
		tmp3[i] ^= tmp1[i];
	/* XOR with c1 (= ..00, i.e., NOP) */

	/* f1 || f1* = E_K(tmp3) XOR OP_c */
	if (aes_128_encrypt_block(k, tmp3, tmp1))
		return -1;
	for (i = 0; i < 16; i++)
		tmp1[i] ^= opc[i];
	if (mac_a)
		memcpy(mac_a, tmp1, 8); /* f1 */
	if (mac_s)
		memcpy(mac_s, tmp1 + 8, 8); /* f1* */
	return 0;
}


/**
 * milenage_f2345 - Milenage f2, f3, f4, f5, f5* algorithms
 * @opc: OPc = 128-bit value derived from OP and K
 * @k: K = 128-bit subscriber key
 * @_rand: RAND = 128-bit random challenge
 * @res: Buffer for RES = 64-bit signed response (f2), or %NULL
 * @ck: Buffer for CK = 128-bit confidentiality key (f3), or %NULL
 * @ik: Buffer for IK = 128-bit integrity key (f4), or %NULL
 * @ak: Buffer for AK = 48-bit anonymity key (f5), or %NULL
 * @akstar: Buffer for AK = 48-bit anonymity key (f5*), or %NULL
 * Returns: 0 on success, -1 on failure
 */
int milenage_f2345(const u8 *opc, const u8 *k, const u8 *_rand,
		   u8 *res, u8 *ck, u8 *ik, u8 *ak, u8 *akstar)
{
	u8 tmp1[16], tmp2[16], tmp3[16];
	int i;

	/* tmp2 = TEMP = E_K(RAND XOR OP_C) */
	for (i = 0; i < 16; i++)
		tmp1[i] = _rand[i] ^ opc[i];
	if (aes_128_encrypt_block(k, tmp1, tmp2))
		return -1;

	/* OUT2 = E_K(rot(TEMP XOR OP_C, r2) XOR c2) XOR OP_C */
	/* OUT3 = E_K(rot(TEMP XOR OP_C, r3) XOR c3) XOR OP_C */
	/* OUT4 = E_K(rot(TEMP XOR OP_C, r4) XOR c4) XOR OP_C */
	/* OUT5 = E_K(rot(TEMP XOR OP_C, r5) XOR c5) XOR OP_C */

	/* f2 and f5 */
	/* rotate by r2 (= 0, i.e., NOP) */
	for (i = 0; i < 16; i++)
		tmp1[i] = tmp2[i] ^ opc[i];
	tmp1[15] ^= 1; /* XOR c2 (= ..01) */
	/* f5 || f2 = E_K(tmp1) XOR OP_c */
	if (aes_128_encrypt_block(k, tmp1, tmp3))
		return -1;
	for (i = 0; i < 16; i++)
		tmp3[i] ^= opc[i];
	if (res)
		memcpy(res, tmp3 + 8, 8); /* f2 */
	if (ak)
		memcpy(ak, tmp3, 6); /* f5 */

	/* f3 */
	if (ck) {
		/* rotate by r3 = 0x20 = 4 bytes */
		for (i = 0; i < 16; i++)
			tmp1[(i + 12) % 16] = tmp2[i] ^ opc[i];
		tmp1[15] ^= 2; /* XOR c3 (= ..02) */
		if (aes_128_encrypt_block(k, tmp1, ck))
			return -1;
		for (i = 0; i < 16; i++)
			ck[i] ^= opc[i];
	}

	/* f4 */
	if (ik) {
		/* rotate by r4 = 0x40 = 8 bytes */
		for (i = 0; i < 16; i++)
			tmp1[(i + 8) % 16] = tmp2[i] ^ opc[i];
		tmp1[15] ^= 4; /* XOR c4 (= ..04) */
		if (aes_128_encrypt_block(k, tmp1, ik))
			return -1;
		for (i = 0; i < 16; i++)
			ik[i] ^= opc[i];
	}

	/* f5* */
	if (akstar) {
		/* rotate by r5 = 0x60 = 12 bytes */
		for (i = 0; i < 16; i++)
			tmp1[(i + 4) % 16] = tmp2[i] ^ opc[i];
		tmp1[15] ^= 8; /* XOR c5 (= ..08) */
		if (aes_128_encrypt_block(k, tmp1, tmp1))
			return -1;
		for (i = 0; i < 6; i++)
			akstar[i] = tmp1[i] ^ opc[i];
	}

	return 0;
}


/**
 * milenage_generate - Generate AKA AUTN,IK,CK,RES
 * @opc: OPc = 128-bit operator variant algorithm configuration field (encr.)
 * @amf: AMF = 16-bit authentication management field
 * @k: K = 128-bit subscriber key
 * @sqn: SQN = 48-bit sequence number
 * @_rand: RAND = 128-bit random challenge
 * @autn: Buffer for AUTN = 128-bit authentication token
 * @ik: Buffer for IK = 128-bit integrity key (f4), or %NULL
 * @ck: Buffer for CK = 128-bit confidentiality key (f3), or %NULL
 * @res: Buffer for RES = 64-bit signed response (f2), or %NULL
 * @res_len: Max length for res; set to used length or 0 on failure
 */
void milenage_generate(const u8 *opc, const u8 *amf, const u8 *k,
		       const u8 *sqn, const u8 *_rand, u8 *autn, u8 *ik,
		       u8 *ck, u8 *res, size_t *res_len)
{
	int i;
	u8 mac_a[8], ak[6];

	if (*res_len < 8) {
		*res_len = 0;
		return;
	}
	if (milenage_f1(opc, k, _rand, sqn, amf, mac_a, NULL) ||
	    milenage_f2345(opc, k, _rand, res, ck, ik, ak, NULL)) {
		*res_len = 0;
		return;
	}
	*res_len = 8;

	/* AUTN = (SQN ^ AK) || AMF || MAC */
	for (i = 0; i < 6; i++)
		autn[i] = sqn[i] ^ ak[i];
	memcpy(autn + 6, amf, 2);
	memcpy(autn + 8, mac_a, 8);
}


/**
 * milenage_auts - Milenage AUTS validation
 * @opc: OPc = 128-bit operator variant algorithm configuration field (encr.)
 * @k: K = 128-bit subscriber key
 * @_rand: RAND = 128-bit random challenge
 * @auts: AUTS = 112-bit authentication token from client
 * @sqn: Buffer for SQN = 48-bit sequence number
 * Returns: 0 = success (sqn filled), -1 on failure
 */
int milenage_auts(const u8 *opc, const u8 *k, const u8 *_rand, const u8 *auts,
		  u8 *sqn)
{
	u8 amf[2] = { 0x00, 0x00 }; /* TS 33.102 v7.0.0, 6.3.3 */
	u8 ak[6], mac_s[8];
	int i;

	if (milenage_f2345(opc, k, _rand, NULL, NULL, NULL, NULL, ak))
		return -1;
	for (i = 0; i < 6; i++)
		sqn[i] = auts[i] ^ ak[i];
	if (milenage_f1(opc, k, _rand, sqn, amf, NULL, mac_s) ||
	    memcmp(mac_s, auts + 6, 8) != 0)
		return -1;
	return 0;
}


/**
 * gsm_milenage - Generate GSM-Milenage (3GPP TS 55.205) authentication triplet
 * @opc: OPc = 128-bit operator variant algorithm configuration field (encr.)
 * @k: K = 128-bit subscriber key
 * @_rand: RAND = 128-bit random challenge
 * @sres: Buffer for SRES = 32-bit SRES
 * @kc: Buffer for Kc = 64-bit Kc
 * Returns: 0 on success, -1 on failure
 */
int gsm_milenage(const u8 *opc, const u8 *k, const u8 *_rand, u8 *sres, u8 *kc)
{
	u8 res[8], ck[16], ik[16];
	int i;

	if (milenage_f2345(opc, k, _rand, res, ck, ik, NULL, NULL))
		return -1;

	for (i = 0; i < 8; i++)
		kc[i] = ck[i] ^ ck[i + 8] ^ ik[i] ^ ik[i + 8];

#ifdef GSM_MILENAGE_ALT_SRES
	memcpy(sres, res, 4);
#else /* GSM_MILENAGE_ALT_SRES */
	for (i = 0; i < 4; i++)
		sres[i] = res[i] ^ res[i + 4];
#endif /* GSM_MILENAGE_ALT_SRES */
	return 0;
}


/**
 * milenage_generate - Generate AKA AUTN,IK,CK,RES
 * @opc: OPc = 128-bit operator variant algorithm configuration field (encr.)
 * @k: K = 128-bit subscriber key
 * @sqn: SQN = 48-bit sequence number
 * @_rand: RAND = 128-bit random challenge
 * @autn: AUTN = 128-bit authentication token
 * @ik: Buffer for IK = 128-bit integrity key (f4), or %NULL
 * @ck: Buffer for CK = 128-bit confidentiality key (f3), or %NULL
 * @res: Buffer for RES = 64-bit signed response (f2), or %NULL
 * @res_len: Variable that will be set to RES length
 * @auts: 112-bit buffer for AUTS
 * Returns: 0 on success, -1 on failure, or -2 on synchronization failure
 */
int milenage_check(const u8 *opc, const u8 *k, const u8 *sqn, const u8 *_rand,
		   const u8 *autn, u8 *ik, u8 *ck, u8 *res, size_t *res_len,
		   u8 *auts)
{
	int i;
	u8 mac_a[8], ak[6], rx_sqn[6];
	const u8 *amf;

#ifdef DEBUG
	hexdump("Milenage: OPC", opc, 16);
	hexdump("Milenage: K", k, 16);
	hexdump("Milenage: AUTN", autn, 16);
	hexdump("Milenage: RAND", _rand, 16);
#endif

	if (milenage_f2345(opc, k, _rand, res, ck, ik, ak, NULL))
		return -1;

	*res_len = 8;
#ifdef DEBUG
	hexdump("Milenage: RES", res, *res_len);
	hexdump("Milenage: CK", ck, 16);
	hexdump("Milenage: IK", ik, 16);
	hexdump("Milenage: AK", ak, 6);
#endif

	/* AUTN = (SQN ^ AK) || AMF || MAC */
	for (i = 0; i < 6; i++)
		rx_sqn[i] = autn[i] ^ ak[i];
#ifdef DEBUG
	hexdump("Milenage: RX SQN", rx_sqn, 6);
	hexdump("Milenage: SQN", sqn, 6);
#endif

	if (memcmp(rx_sqn, sqn, 6) <= 0) {
		u8 auts_amf[2] = { 0x00, 0x00 }; /* TS 33.102 v7.0.0, 6.3.3 */
		if (milenage_f2345(opc, k, _rand, NULL, NULL, NULL, NULL, ak))
			return -1;
#ifdef DEBUG
		hexdump("Milenage: AK*", ak, 6);
#endif
		for (i = 0; i < 6; i++)
			auts[i] = sqn[i] ^ ak[i];
		if (milenage_f1(opc, k, _rand, sqn, auts_amf, NULL, auts + 6))
			return -1;
#ifdef DEBUG
		hexdump("Milenage: AUTS", auts, 14);
#endif
		return -2;
	}

	amf = autn + 6;
#ifdef DEBUG
	hexdump("Milenage: AMF", amf, 2);
#endif
	if (milenage_f1(opc, k, _rand, rx_sqn, amf, mac_a, NULL))
		return -1;

#ifdef DEBUG
	hexdump("Milenage: MAC_A", mac_a, 8);
#endif

	if (memcmp(mac_a, autn + 8, 8) != 0) {
#ifdef DEBUG
		hexdump("Milenage: MAC mismatch, Received MAC_A",
			    autn + 8, 8);
#endif
		return -1;
	}

	return 0;
}
