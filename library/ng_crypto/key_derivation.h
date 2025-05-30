#pragma once

#include <stdint.h>
#include <unistd.h>

void kdf_xres_star(const uint8_t *serving_network_name,
		   uint16_t serving_network_name_len,
		   const uint8_t *ck,
		   const uint8_t *ik,
		   const uint8_t *rand,
		   const uint8_t *xres, size_t xres_len,
		   uint8_t *out);
