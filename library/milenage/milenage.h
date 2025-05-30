
#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

int milenage_f1(const u8 *opc, const u8 *k, const u8 *_rand,
		const u8 *sqn, const u8 *amf, u8 *mac_a, u8 *mac_s);
int milenage_f2345(const u8 *opc, const u8 *k, const u8 *_rand,
		   u8 *res, u8 *ck, u8 *ik, u8 *ak, u8 *akstar);
void milenage_generate(const u8 *opc, const u8 *amf, const u8 *k,
		       const u8 *sqn, const u8 *_rand, u8 *autn, u8 *ik,
		       u8 *ck, u8 *res, size_t *res_len);
int milenage_auts(const u8 *opc, const u8 *k, const u8 *_rand, const u8 *auts,
		  u8 *sqn);
int gsm_milenage(const u8 *opc, const u8 *k, const u8 *_rand, u8 *sres, u8 *kc);
int milenage_check(const u8 *opc, const u8 *k, const u8 *sqn, const u8 *_rand,
		   const u8 *autn, u8 *ik, u8 *ck, u8 *res, size_t *res_len,
		   u8 *auts);
