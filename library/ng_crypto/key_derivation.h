#pragma once

#include <stdint.h>
#include <unistd.h>

void kdf_kausf(const uint8_t *ck, const uint8_t *ik,
	       const uint8_t *serving_network_name, uint16_t serving_network_name_len,
	       const uint8_t *autn,
	       uint8_t *out);

void kdf_kseaf(const uint8_t *kausf,
	       const uint8_t *serving_network_name, uint16_t serving_network_name_len,
	       uint8_t *out);

void kdf_kamf(const uint8_t *kseaf,
	      const uint8_t *supi, uint16_t supi_len,
	      const uint8_t *abba,
	      uint8_t *out);

void kdf_xres_star(const uint8_t *serving_network_name,
		   uint16_t serving_network_name_len,
		   const uint8_t *ck,
		   const uint8_t *ik,
		   const uint8_t *rand,
		   const uint8_t *xres, size_t xres_len,
		   uint8_t *out);

void kdf_ng_nas_algo(const uint8_t *kamf,
		     uint8_t algo_type,
		     uint8_t algo_id,
		     uint8_t *out);
