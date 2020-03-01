
#include "Octetstring.hh"
#include "Error.hh"
#include "Logger.hh"

#include "IuUP_Types.hh"

#include <stdint.h>

extern "C" {

typedef uint8_t ubit_t;
typedef uint8_t pbit_t;

static int osmo_pbit2ubit(ubit_t *out, const pbit_t *in, unsigned int num_bits)
{
	unsigned int i;
	ubit_t *cur = out;
	ubit_t *limit = out + num_bits;

	for (i = 0; i < (num_bits/8)+1; i++) {
		pbit_t byte = in[i];
		*cur++ = (byte >> 7) & 1;
		if (cur >= limit)
			break;
		*cur++ = (byte >> 6) & 1;
		if (cur >= limit)
			break;
		*cur++ = (byte >> 5) & 1;
		if (cur >= limit)
			break;
		*cur++ = (byte >> 4) & 1;
		if (cur >= limit)
			break;
		*cur++ = (byte >> 3) & 1;
		if (cur >= limit)
			break;
		*cur++ = (byte >> 2) & 1;
		if (cur >= limit)
			break;
		*cur++ = (byte >> 1) & 1;
		if (cur >= limit)
			break;
		*cur++ = (byte >> 0) & 1;
		if (cur >= limit)
			break;
	}
	return cur - out;
}


struct osmo_crc8gen_code {
	int bits;           /*!< Actual number of bits of the CRC */
	uint8_t poly;      /*!< Polynom (normal representation, MSB omitted */
	uint8_t init;      /*!< Initialization value of the CRC state */
	uint8_t remainder; /*!< Remainder of the CRC (final XOR) */
};

static uint8_t
osmo_crc8gen_compute_bits(const struct osmo_crc8gen_code *code,
                           const ubit_t *in, int len)
{
	const uint8_t poly = code->poly;
	uint8_t crc = code->init;
	int i, n = code->bits-1;

	for (i=0; i<len; i++) {
		uint8_t bit = in[i] & 1;
		crc ^= (bit << n);
		if (crc & ((uint8_t)1 << n)) {
			crc <<= 1;
			crc ^= poly;
		} else {
			crc <<= 1;
		}
		crc &= ((uint8_t)1 << code->bits) - 1;
	}

	crc ^= code->remainder;

	return crc;
}

struct osmo_crc16gen_code {
	int bits;           /*!< Actual number of bits of the CRC */
	uint16_t poly;      /*!< Polynom (normal representation, MSB omitted */
	uint16_t init;      /*!< Initialization value of the CRC state */
	uint16_t remainder; /*!< Remainder of the CRC (final XOR) */
};

static uint16_t
osmo_crc16gen_compute_bits(const struct osmo_crc16gen_code *code,
                           const ubit_t *in, int len)
{
	const uint16_t poly = code->poly;
	uint16_t crc = code->init;
	int i, n = code->bits-1;

	for (i=0; i<len; i++) {
		uint16_t bit = in[i] & 1;
		crc ^= (bit << n);
		if (crc & ((uint16_t)1 << n)) {
			crc <<= 1;
			crc ^= poly;
		} else {
			crc <<= 1;
		}
		crc &= ((uint16_t)1 << code->bits) - 1;
	}

	crc ^= code->remainder;

	return crc;
}

}


static const struct osmo_crc8gen_code iuup_hdr_crc_code = {
	.bits = 6,
	.poly = 47,
	.init = 0,
	.remainder = 0,
};

static const struct osmo_crc16gen_code iuup_data_crc_code = {
	.bits = 10,
	.poly = 563,
	.init = 0,
	.remainder = 0,
};

static int iuup_get_payload_offset(const uint8_t *iuup_pdu)
{
	uint8_t pdu_type = iuup_pdu[0] >> 4;
	switch (pdu_type) {
	case 0:
	case 14:
		return 4;
	case 1:
		return 3;
	default:
		return -1;
	}
}

int osmo_iuup_compute_payload_crc(const uint8_t *iuup_pdu, unsigned int pdu_len)
{
	ubit_t buf[1024*8];
	uint8_t pdu_type;
	int offset, payload_len_bytes;

	if (pdu_len < 1)
		return -1;

	pdu_type = iuup_pdu[0] >> 4;

	/* Type 1 has no CRC */
	if (pdu_type == 1)
		return 0;

	offset = iuup_get_payload_offset(iuup_pdu);
	if (offset < 0)
		return offset;

	if (pdu_len < (unsigned int)offset)
		return -1;

	payload_len_bytes = pdu_len - offset;
	osmo_pbit2ubit(buf, iuup_pdu+offset, payload_len_bytes*8);
	return osmo_crc16gen_compute_bits(&iuup_data_crc_code, buf, payload_len_bytes*8);
}

int osmo_iuup_compute_header_crc(const uint8_t *iuup_pdu, unsigned int pdu_len)
{
	ubit_t buf[2*8];

	if (pdu_len < 2)
		return -1;

	osmo_pbit2ubit(buf, iuup_pdu, 2*8);
	return osmo_crc8gen_compute_bits(&iuup_hdr_crc_code, buf, 2*8);
}
/* IuUP CRC Implementation */

/* (C) 2017 by Harald Welte <laforge@gnumonks.org>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

namespace IuUP__Types {


INTEGER f__IuUP__compute__crc__payload(OCTETSTRING const &in)
{
	int crc_calc;
	const unsigned char *data = (const unsigned char *)in;
	int len = in.lengthof();

	crc_calc = osmo_iuup_compute_payload_crc(data, len);

	return INTEGER(crc_calc);
}

INTEGER f__IuUP__compute__crc__header(OCTETSTRING const &in)
{
	int crc_calc;
	const unsigned char *data = (const unsigned char *)in;
	int len = in.lengthof();

	crc_calc = osmo_iuup_compute_header_crc(data, len);

	return INTEGER(crc_calc);
}

OCTETSTRING f__enc__IuUP__PDU(const IuUP__PDU& pdu)
{
	TTCN_Buffer buf;
	int crc_hdr, crc_payload;
	uint8_t in[2];
	pdu.encode(IuUP__PDU_descr_, buf, TTCN_EncDec::CT_RAW);
	OCTETSTRING ret_val(buf.get_len(), buf.get_data());

	crc_hdr = osmo_iuup_compute_header_crc(buf.get_data(), buf.get_len());
	crc_payload = osmo_iuup_compute_payload_crc(buf.get_data(), buf.get_len());
	in[0] = (crc_hdr & 0x3f) << 2;
	in[0] |= (crc_payload & 0x3ff) >> 8;
	in[1] = crc_payload & 0xff;
	OCTETSTRING CHECKSUM(2, in);

	ret_val = substr(ret_val, 0, 2) + CHECKSUM + substr(ret_val, 4, ret_val.lengthof()-4);

	return ret_val;
}

}
