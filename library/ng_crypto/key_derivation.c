#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <gnutls/crypto.h>

#include "key_derivation.h"

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
