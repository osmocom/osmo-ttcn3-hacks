#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <gnutls/crypto.h>

#include "key_derivation.h"

/* 3GPP TS 33.501 A.2 KAUSF derivation function */
void kdf_kausf(const uint8_t *ck, const uint8_t *ik,
	       const uint8_t *serving_network_name, uint16_t serving_network_name_len,
	       const uint8_t *autn,
	       uint8_t *out)
{
	uint8_t s[1024];
	uint8_t key[32];
	size_t pos = 0;
	uint16_t lenbe;
	int i;

	memcpy(&key[0], ck, 16);
	memcpy(&key[16], ik, 16);

	s[pos++] = 0x6A; /* FC Value */

	memcpy(&s[pos], serving_network_name, serving_network_name_len);
	pos += serving_network_name_len;
	lenbe = htons(serving_network_name_len);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	memcpy(&s[pos], autn, 6);
	pos += 6;
	lenbe = htons(6);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, key, sizeof(key), s, pos, out);
}

/* 3GPP TS 33.501 A.4 RES* and XRES* derivation function */
void kdf_xres_star(const uint8_t *serving_network_name,
		   uint16_t serving_network_name_len,
		   const uint8_t *ck,
		   const uint8_t *ik,
		   const uint8_t *rand,
		   const uint8_t *xres, size_t xres_len,
		   uint8_t *out)
{
	uint8_t s[1024];
	uint8_t key[16*2];
	uint8_t tmp_out[32];
	size_t pos = 0;
	uint16_t lenbe;

	memcpy(key, ck, 16);
	memcpy(key+16, ik, 16);

	s[pos++] = 0x6B; /* FC Value */

	memcpy(&s[pos], serving_network_name, serving_network_name_len);
	pos += serving_network_name_len;
	lenbe = htons(serving_network_name_len);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	memcpy(&s[pos], rand, 16);
	pos += 16;
	lenbe = htons(16);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	memcpy(&s[pos], xres, xres_len);
	pos += xres_len;
	lenbe = htons(xres_len);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, key, sizeof(key), s, pos, tmp_out);

	memcpy(out, tmp_out+16, 16);
}

/* 3GPP TS 33.501 A.6 KSEAF derivation function */
void kdf_kseaf(const uint8_t *kausf,
	       const uint8_t *serving_network_name, uint16_t serving_network_name_len,
	       uint8_t *out)
{
	uint8_t s[1024];
	size_t pos = 0;
	uint16_t lenbe;
	int i;

	s[pos++] = 0x6C; /* FC Value */

	memcpy(&s[pos], serving_network_name, serving_network_name_len);
	pos += serving_network_name_len;
	lenbe = htons(serving_network_name_len);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, kausf, 32, s, pos, out);
}

/* 3GPP TS 33.501 A.7 KAMF derivation function */
void kdf_kamf(const uint8_t *kseaf,
	      const uint8_t *supi, uint16_t supi_len,
	      const uint8_t *abba,
	      uint8_t *out)
{
	uint8_t s[1024];
	size_t pos = 0;
	uint16_t lenbe;
	int i;

	s[pos++] = 0x6D; /* FC Value */

	memcpy(&s[pos], supi, supi_len);
	pos += supi_len;
	lenbe = htons(supi_len);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	memcpy(&s[pos], abba, 2);
	pos += 2;
	lenbe = htons(2);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, kseaf, 32, s, pos, out);
}

/* 3GPP TS 33.501 A.8 Algorithm key derivation functions */
void kdf_ng_nas_algo(const uint8_t *kamf,
		     uint8_t algo_type,
		     uint8_t algo_id,
		     uint8_t *out)
{
	uint8_t s[1024];
	size_t pos = 0;
	uint16_t lenbe;
	int i;

	s[pos++] = 0x69; /* FC Value */

	s[pos++] = algo_type;
	lenbe = htons(1);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	s[pos++] = algo_id;
	lenbe = htons(1);
	memcpy(&s[pos], &lenbe, 2);
	pos += 2;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, kamf, 32, s, pos, out);
}
