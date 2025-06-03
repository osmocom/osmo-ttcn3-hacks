#pragma once

#include <stdint.h>

#define HSS_SQN_LEN 6
#define HSS_AK_LEN 6

void hss_auc_kasme(const uint8_t *ck, const uint8_t *ik,
        const uint8_t plmn_id[3], const uint8_t *sqn,  const uint8_t *ak,
        uint8_t *kasme);

/* Algorithm Type Distinguishers */
#define MME_KDF_NAS_ENC_ALG 0x01
#define MME_KDF_NAS_INT_ALG 0x02

void mme_kdf_nas(uint8_t algorithm_type_distinguishers,
    uint8_t algorithm_identity, const uint8_t *kasme, uint8_t *knas);

void mme_kdf_enb(const uint8_t *kasme, uint32_t ul_count, uint8_t *kenb);

void mme_kdf_nh(const uint8_t *kasme, const uint8_t *sync_input, uint8_t *kenb);

void mme_kdf_nas_token(const uint8_t *kasme, uint32_t ul_count, uint8_t *nas_token);
