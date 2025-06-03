#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <gnutls/crypto.h>

/* From nextepc/src/mme/mme-kdf.c under AGPLv3+ */

void mme_kdf_nas(uint8_t algorithm_type_distinguishers,
		 uint8_t algorithm_identity, const uint8_t *kasme, uint8_t *knas)
{
	uint8_t s[7];
	uint8_t out[32];

	s[0] = 0x15; /* FC Value */

	s[1] = algorithm_type_distinguishers;
	s[2] = 0x00;
	s[3] = 0x01;

	s[4] = algorithm_identity;
	s[5] = 0x00;
	s[6] = 0x01;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, kasme, 32, s, 7, out);
	memcpy(knas, out+16, 16);
}

void mme_kdf_enb(const uint8_t *kasme, uint32_t ul_count, uint8_t *kenb)
{
	uint8_t s[7];

	s[0] = 0x11; /* FC Value */

	ul_count = htonl(ul_count);
	memcpy(s+1, &ul_count, 4);

	s[5] = 0x00;
	s[6] = 0x04;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, kasme, 32, s, 7, kenb);
}

void mme_kdf_nh(const uint8_t *kasme, const uint8_t *sync_input, uint8_t *kenb)
{
	uint8_t s[35];

	s[0] = 0x12; /* FC Value */

	memcpy(s+1, sync_input, 32);

	s[33] = 0x00;
	s[34] = 0x20;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, kasme, 32, s, 35, kenb);
}

/* From nextepc/src/hss/hss-auc.c under AGPLv3+ */

#define FC_VALUE 0x10

void hss_auc_kasme(const uint8_t *ck, const uint8_t *ik, const uint8_t plmn_id[3],
		   const uint8_t *sqn,  const uint8_t *ak, uint8_t *kasme)
{
	uint8_t s[14];
	uint8_t k[32];
	int i;

	memcpy(&k[0], ck, 16);
	memcpy(&k[16], ik, 16);

	s[0] = FC_VALUE;
	memcpy(&s[1], plmn_id, 3);
	s[4] = 0x00;
	s[5] = 0x03;

	for (i = 0; i < 6; i++)
		s[6+i] = sqn[i] ^ ak[i];
	s[12] = 0x00;
	s[13] = 0x06;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, k, 32, s, 14, kasme);
}

/* TS33.401 Annex A.9: NAS token derivation for inter-RAT mobility */
void mme_kdf_nas_token(const uint8_t *kasme, uint32_t ul_count, uint8_t *nas_token)
{
	uint8_t s[7];

	s[0] = 0x17; /* FC Value */

	ul_count = htonl(ul_count);
	memcpy(s+1, &ul_count, 4);

	s[5] = 0x00;
	s[6] = 0x04;

	gnutls_hmac_fast(GNUTLS_MAC_SHA256, kasme, 32, s, 7, nas_token);
}
