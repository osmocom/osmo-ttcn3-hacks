#include <stdint.h>
#include <endian.h>

#include "RLCMAC_Types.hh"
#include "GSM_Types.hh"
/* Decoding of TS 44.060 GPRS RLC/MAC blocks, portions requiring manual functions
 * beyond what TITAN RAW coder can handle internally.
 *
 * (C) 2017 by Harald Welte <laforge@gnumonks.org>
 * All rights reserved.
 *
 * Released under the terms of GNU General Public License, Version 2 or
 * (at your option) any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

namespace RLCMAC__Types {

/////////////////////
// INTENRAL HELPERS
/////////////////////

/* TS 04.60  10.3a.4.1.1 */
struct gprs_rlc_ul_header_egprs_1 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t r:1,
		 si:1,
		 cv:4,
		 tfi_hi:2;
	uint8_t tfi_lo:3,
		 bsn1_hi:5;
	uint8_t bsn1_lo:6,
		 bsn2_hi:2;
	uint8_t bsn2_lo:8;
	uint8_t cps:5,
		 rsb:1,
		 pi:1,
		 spare_hi:1;
	uint8_t spare_lo:6,
		 dummy:2;
#else
/* auto-generated from the little endian part above (libosmocore/contrib/struct_endianess.py) */
	uint8_t tfi_hi:2, cv:4, si:1, r:1;
	uint8_t bsn1_hi:5, tfi_lo:3;
	uint8_t bsn2_hi:2, bsn1_lo:6;
	uint8_t bsn2_lo:8;
	uint8_t spare_hi:1, pi:1, rsb:1, cps:5;
	uint8_t dummy:2, spare_lo:6;
#endif
} __attribute__ ((packed));

/* TS 04.60  10.3a.4.2.1 */
struct gprs_rlc_ul_header_egprs_2 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t r:1,
		 si:1,
		 cv:4,
		 tfi_hi:2;
	uint8_t tfi_lo:3,
		 bsn1_hi:5;
	uint8_t bsn1_lo:6,
		 cps_hi:2;
	uint8_t cps_lo:1,
		 rsb:1,
		 pi:1,
		 spare_hi:5;
	uint8_t spare_lo:5,
		 dummy:3;
#else
/* auto-generated from the little endian part above (libosmocore/contrib/struct_endianess.py) */
	uint8_t tfi_hi:2, cv:4, si:1, r:1;
	uint8_t bsn1_hi:5, tfi_lo:3;
	uint8_t cps_hi:2, bsn1_lo:6;
	uint8_t spare_hi:5, pi:1, rsb:1, cps_lo:1;
	uint8_t dummy:3, spare_lo:5;
#endif
} __attribute__ ((packed));

/* TS 04.60  10.3a.4.3.1 */
struct gprs_rlc_ul_header_egprs_3 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t r:1,
		 si:1,
		 cv:4,
		 tfi_hi:2;
	uint8_t tfi_lo:3,
		 bsn1_hi:5;
	uint8_t bsn1_lo:6,
		 cps_hi:2;
	uint8_t cps_lo:2,
		 spb:2,
		 rsb:1,
		 pi:1,
		 spare:1,
		 dummy:1;
#else
/* auto-generated from the little endian part above (libosmocore/contrib/struct_endianess.py) */
	uint8_t tfi_hi:2, cv:4, si:1, r:1;
	uint8_t bsn1_hi:5, tfi_lo:3;
	uint8_t cps_hi:2, bsn1_lo:6;
	uint8_t dummy:1, spare:1, pi:1, rsb:1, spb:2, cps_lo:2;
#endif
} __attribute__ ((packed));

struct gprs_rlc_dl_header_egprs_1 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t usf:3,
		 es_p:2,
		 rrbp:2,
		 tfi_hi:1;
	uint8_t tfi_lo:4,
		 pr:2,
		 bsn1_hi:2;
	uint8_t bsn1_mid:8;
	uint8_t bsn1_lo:1,
		 bsn2_hi:7;
	uint8_t bsn2_lo:3,
		 cps:5;
#else
/* auto-generated from the little endian part above (libosmocore/contrib/struct_endianess.py) */
	uint8_t tfi_hi:1, rrbp:2, es_p:2, usf:3;
	uint8_t bsn1_hi:2, pr:2, tfi_lo:4;
	uint8_t bsn1_mid:8;
	uint8_t bsn2_hi:7, bsn1_lo:1;
	uint8_t cps:5, bsn2_lo:3;
#endif
} __attribute__ ((packed));

struct gprs_rlc_dl_header_egprs_2 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t usf:3,
		 es_p:2,
		 rrbp:2,
		 tfi_hi:1;
	uint8_t tfi_lo:4,
		 pr:2,
		 bsn1_hi:2;
	uint8_t bsn1_mid:8;
	uint8_t bsn1_lo:1,
		 cps:3,
		 dummy:4;
#else
/* auto-generated from the little endian part above (libosmocore/contrib/struct_endianess.py) */
	uint8_t tfi_hi:1, rrbp:2, es_p:2, usf:3;
	uint8_t bsn1_hi:2, pr:2, tfi_lo:4;
	uint8_t bsn1_mid:8;
	uint8_t dummy:4, cps:3, bsn1_lo:1;
#endif
} __attribute__ ((packed));

struct gprs_rlc_dl_header_egprs_3 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t usf:3,
		 es_p:2,
		 rrbp:2,
		 tfi_hi:1;
	uint8_t tfi_lo:4,
		 pr:2,
		 bsn1_hi:2;
	uint8_t bsn1_mid:8;
	uint8_t bsn1_lo:1,
		 cps:4,
		 spb:2,
		 dummy:1;
#else
/* auto-generated from the little endian part above (libosmocore/contrib/struct_endianess.py) */
	uint8_t tfi_hi:1, rrbp:2, es_p:2, usf:3;
	uint8_t bsn1_hi:2, pr:2, tfi_lo:4;
	uint8_t bsn1_mid:8;
	uint8_t dummy:1, spb:2, cps:4, bsn1_lo:1;
#endif
} __attribute__ ((packed));

static CodingScheme::enum_type payload_len_2_coding_scheme(size_t payload_len) {
	switch (payload_len) {
	case 23:
		return CodingScheme::CS__1;
	case 34:
		return CodingScheme::CS__2;
	case 40:
		return CodingScheme::CS__3;
	case 54:
		return CodingScheme::CS__4;
	case 27:
		return CodingScheme::MCS__1;
	case 33:
		return CodingScheme::MCS__2;
	case 42:
		return CodingScheme::MCS__3;
	case 49:
		return CodingScheme::MCS__4;
	case 61:
		return CodingScheme::MCS__5;
	case 79:
		return CodingScheme::MCS__6;
	case 119:
		return CodingScheme::MCS__7;
	case 142:
		return CodingScheme::MCS__8;
	case 155:
		return CodingScheme::MCS__9;
	default:
		return CodingScheme::CS__1;
	}
}

static unsigned int coding_scheme_2_data_block_len(CodingScheme::enum_type mcs) {
	switch (mcs) {
	case CodingScheme::MCS__0:
		return 0;
	case CodingScheme::MCS__1:
		return 22;
	case CodingScheme::MCS__2:
		return 28;
	case CodingScheme::MCS__3:
		return 37;
	case CodingScheme::MCS__4:
		return 44;
	case CodingScheme::MCS__5:
		return 56;
	case CodingScheme::MCS__6:
		return 74;
	case CodingScheme::MCS__7:
		return 56;
	case CodingScheme::MCS__8:
		return 68;
	case CodingScheme::MCS__9:
		return 74;
	default:
		return 22; /* MCS1*/
	}
}

static uint8_t bs2uint8(const BITSTRING& bs)
{
	int len = bs.lengthof();
	int i;
	uint8_t res = 0;
	for (i = 0; i < len; i++) {
		res = res << 1;
		res |= (bs[i].get_bit() ? 1 : 0);
	}
	return res;
}

/* determine the number of rlc data blocks and their size / offsets */
static void
setup_rlc_mac_priv(CodingScheme::enum_type mcs, EgprsHeaderType::enum_type hdrtype, boolean is_uplink,
	unsigned int *n_calls, unsigned int *data_block_bits, unsigned int *data_block_offsets)
{
	unsigned int nc, dbl = 0, dbo[2] = {0,0};

	dbl = coding_scheme_2_data_block_len(mcs);

	switch (hdrtype) {
	case EgprsHeaderType::RLCMAC__HDR__TYPE__1:
		nc = 3;
		dbo[0] = is_uplink ? 5*8 + 6 : 5*8 + 0;
		dbo[1] = dbo[0] + dbl * 8 + 2;
		break;
	case EgprsHeaderType::RLCMAC__HDR__TYPE__2:
		nc = 2;
		dbo[0] = is_uplink ? 4*8 + 5 : 3*8 + 4;
		break;
	case EgprsHeaderType::RLCMAC__HDR__TYPE__3:
		nc = 2;
		dbo[0] = 3*8 + 7;
		break;
	default:
		nc = 1;
		break;
	}

	*n_calls = nc;
	*data_block_bits = dbl * 8 + 2;
	data_block_offsets[0] = dbo[0];
	data_block_offsets[1] = dbo[1];
}

/* bit-shift the entire 'src' of length 'length_bytes' by 'offset_bits'
 * and store the result to caller-allocated 'buffer'.  The shifting is
 * done lsb-first. */
static void clone_aligned_buffer_lsbf(unsigned int offset_bits, unsigned int length_bytes,
	const uint8_t *src, uint8_t *buffer)
{
	unsigned int hdr_bytes;
	unsigned int extra_bits;
	unsigned int i;

	uint8_t c, last_c;
	uint8_t *dst;

	hdr_bytes = offset_bits / 8;
	extra_bits = offset_bits % 8;

	fprintf(stderr, "RLMAC: clone: hdr_bytes=%u extra_bits=%u (length_bytes=%u)\n", hdr_bytes, extra_bits, length_bytes);

	if (extra_bits == 0) {
		/* It is aligned already */
		memcpy(buffer, src + hdr_bytes, length_bytes);
		return;
	}

	dst = buffer;
	src = src + hdr_bytes;
	last_c = *(src++);

	for (i = 0; i < length_bytes; i++) {
		c = src[i];
		*(dst++) = (last_c >> extra_bits) | (c << (8 - extra_bits));
		last_c = c;
	}
}

/* obtain an (aligned) EGPRS data block with given bit-offset and
 * bit-length from the parent buffer */
static void get_egprs_data_block(const TTCN_Buffer& orig_ttcn_buffer, unsigned int offset_bits,
	unsigned int length_bits, TTCN_Buffer& dst_ttcn_buffer)
{
	const unsigned int initial_spare_bits = 6;
	unsigned char *aligned_buf = NULL;
	size_t min_src_length_bytes = (offset_bits + length_bits + 7) / 8;
	size_t length_bytes = (initial_spare_bits + length_bits + 7) / 8;
	size_t accepted_len = length_bytes;

	fprintf(stderr, "RLMAC: trying to allocate %u bytes (orig is %zu bytes long with read pos %zu)\n", length_bytes, orig_ttcn_buffer.get_len(), orig_ttcn_buffer.get_pos());
	dst_ttcn_buffer.get_end(aligned_buf, accepted_len);
	fprintf(stderr, "RLMAC: For dst ptr=%p with length=%zu\n", aligned_buf, accepted_len);
	if (accepted_len < length_bytes) {
		fprintf(stderr, "RLMAC: ERROR! asked for %zu bytes but got %zu\n", length_bytes, accepted_len);
	}

	/* Copy the data out of the tvb to an aligned buffer */
	clone_aligned_buffer_lsbf(
		offset_bits - initial_spare_bits, length_bytes,
		orig_ttcn_buffer.get_data(),
		aligned_buf);

	fprintf(stderr, "RLMAC: clone_aligned_buffer_lsbf success\n");

	/* clear spare bits and move block header bits to the right */
	aligned_buf[0] = aligned_buf[0] >> initial_spare_bits;

	dst_ttcn_buffer.increase_length(length_bytes);
}


/////////////////////
// DECODE
/////////////////////

/* DECODE DOWNLINK */

RlcmacDlDataBlock dec__RlcmacDlDataBlock(const OCTETSTRING& stream)
{
	RlcmacDlDataBlock ret_val;
	TTCN_Buffer ttcn_buffer(stream);
	int num_llc_blocks = 0;

	/* use automatic/generated decoder for header */
	ret_val.mac__hdr().decode(DlMacDataHeader_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	/* optional extension octets, containing LI+M+E of Llc blocks */
	if (ret_val.mac__hdr().hdr__ext().e() == false) {
		/* extension octet follows, i.e. optional Llc length octets */
		while (1) {
			/* decode one more extension octet with LlcBlocHdr inside */
			LlcBlock lb;
			lb.hdr()().decode(LlcBlockHdr_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
			ret_val.blocks()[num_llc_blocks++] = lb;

			/* if E == '1'B, we can proceed further */
			if (lb.hdr()().e() == true)
				break;
		}
	}

	/* RLC blocks at end */
	if (ret_val.mac__hdr().hdr__ext().e() == true) {
		LlcBlock lb;
		unsigned int length = ttcn_buffer.get_read_len();
		/* LI not present: The Upper Layer PDU that starts with the current RLC data block either
		 * fills the current RLC data block precisely or continues in the following in-sequence RLC
		 * data block */
		lb.payload() = OCTETSTRING(length, ttcn_buffer.get_read_data());
		ttcn_buffer.increase_pos(length);
		ret_val.blocks()[0] = lb;
	} else {
		if (ret_val.blocks().is_bound()) {
			for (int i = 0; i < ret_val.blocks().size_of(); i++) {
				unsigned int length = ret_val.blocks()[i].hdr()().length__ind();
				if (length > ttcn_buffer.get_read_len())
					length = ttcn_buffer.get_read_len();
				ret_val.blocks()[i].payload() = OCTETSTRING(length, ttcn_buffer.get_read_data());
				ttcn_buffer.increase_pos(length);
			}
		}
	}

	return ret_val;
}

static
EgprsDlMacDataHeader dec__EgprsDlMacDataHeader_type1(const OCTETSTRING& stream)
{
	EgprsDlMacDataHeader ret_val;

	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);

	return ret_val;
}

static
EgprsDlMacDataHeader dec__EgprsDlMacDataHeader_type2(const OCTETSTRING& stream)
{
	EgprsDlMacDataHeader ret_val;

	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);

	return ret_val;
}

static
EgprsDlMacDataHeader dec__EgprsDlMacDataHeader_type3(const OCTETSTRING& stream)
{
	TTCN_Buffer ttcn_buffer(stream);
	EgprsDlMacDataHeader ret_val;
	const struct gprs_rlc_dl_header_egprs_3 *egprs3;
	uint8_t tmp;

	egprs3 = static_cast<const struct gprs_rlc_dl_header_egprs_3 *>
		((const void *)ttcn_buffer.get_data());

	ret_val.header__type() = EgprsHeaderType::RLCMAC__HDR__TYPE__3;
	ret_val.tfi() = egprs3->tfi_lo << 1 | egprs3->tfi_hi << 0;
	ret_val.rrbp() = egprs3->rrbp;
	tmp = egprs3->es_p;
	ret_val.esp() = BITSTRING(2, &tmp);
	ret_val.usf() = egprs3->usf;
	ret_val.bsn1() = egprs3->bsn1_lo << 10 | egprs3->bsn1_mid << 2 | egprs3->bsn1_hi;
	ret_val.bsn2__offset() = 0; /*TODO: mark optional and not set ? */
	ret_val.pr() = egprs3->pr;
	ret_val.spb() = egprs3->spb;
	ret_val.cps() = egprs3->cps;

	ttcn_buffer.increase_pos(sizeof(*egprs3));
	return ret_val;
}

static
RlcmacDlEgprsDataBlock dec__RlcmacDlEgprsDataBlock(const OCTETSTRING& stream, CodingScheme::enum_type mcs)
{
	RlcmacDlEgprsDataBlock ret_val;
	TTCN_Buffer ttcn_buffer(stream);
	TTCN_Buffer aligned_buffer;
	int num_llc_blocks = 0;
	unsigned int data_block_bits, data_block_offsets[2];
	unsigned int num_calls;
	const uint8_t *ti_e;

	switch (mcs) {
	case CodingScheme::MCS__0:
	case CodingScheme::MCS__1:
	case CodingScheme::MCS__2:
	case CodingScheme::MCS__3:
	case CodingScheme::MCS__4:
		ret_val.mac__hdr() = dec__EgprsDlMacDataHeader_type3(stream);
		break;
	case CodingScheme::MCS__5:
	case CodingScheme::MCS__6:
		ret_val.mac__hdr() = dec__EgprsDlMacDataHeader_type2(stream);
		break;
	case CodingScheme::MCS__7:
	case CodingScheme::MCS__8:
	case CodingScheme::MCS__9:
		ret_val.mac__hdr() = dec__EgprsDlMacDataHeader_type1(stream);
		break;
	}
	setup_rlc_mac_priv(mcs, ret_val.mac__hdr().header__type(), false,
			   &num_calls, &data_block_bits, data_block_offsets);
	get_egprs_data_block(ttcn_buffer, data_block_offsets[0], data_block_bits, aligned_buffer);

	ti_e = aligned_buffer.get_read_data();
	ret_val.fbi() = *ti_e & 0x02 ? true : false;
	ret_val.e() = *ti_e & 0x01 ? true : false;
	aligned_buffer.increase_pos(1);

	/* optional extension octets, containing LI+E of Llc blocks */
	if (ret_val.e() == false) {
		/* extension octet follows, i.e. optional Llc length octets */
		while (1) {
			/* decode one more extension octet with LlcBlocHdr inside */
			EgprsLlcBlock lb;
			lb.hdr()().decode(EgprsLlcBlockHdr_descr_, aligned_buffer, TTCN_EncDec::CT_RAW);
			ret_val.blocks()[num_llc_blocks++] = lb;

			/* if E == '1'B, we can proceed further */
			if (lb.hdr()().e() == true)
				break;
		}
	}

	/* RLC blocks at end */
	if (ret_val.blocks().is_bound()) {
		for (int i = 0; i < ret_val.blocks().size_of(); i++) {
			unsigned int length = ret_val.blocks()[i].hdr()().length__ind();
			if (length > aligned_buffer.get_read_len())
				length = aligned_buffer.get_read_len();
			ret_val.blocks()[i].payload() = OCTETSTRING(length, aligned_buffer.get_read_data());
			aligned_buffer.increase_pos(length);
		}
	}

	return ret_val;
}

RlcmacDlBlock dec__RlcmacDlBlock(const OCTETSTRING& stream)
{
	RlcmacDlBlock ret_val;
	size_t stream_len = stream.lengthof();
	CodingScheme::enum_type mcs = payload_len_2_coding_scheme(stream_len);
	unsigned char pt;

	switch (mcs) {
	case CodingScheme::CS__1:
	case CodingScheme::CS__2:
	case CodingScheme::CS__3:
	case CodingScheme::CS__4:
		pt = stream[0].get_octet() >> 6;
		if (pt == MacPayloadType::MAC__PT__RLC__DATA)
			ret_val.data() = dec__RlcmacDlDataBlock(stream);
		else
			ret_val.ctrl() = dec__RlcmacDlCtrlBlock(stream);
		break;
	case CodingScheme::MCS__0:
	case CodingScheme::MCS__1:
	case CodingScheme::MCS__2:
	case CodingScheme::MCS__3:
	case CodingScheme::MCS__4:
	case CodingScheme::MCS__5:
	case CodingScheme::MCS__6:
	case CodingScheme::MCS__7:
	case CodingScheme::MCS__8:
	case CodingScheme::MCS__9:
		ret_val.data__egprs() = dec__RlcmacDlEgprsDataBlock(stream, mcs);
		break;
	}
	return ret_val;
}

/* DECODE UPLINK */

RlcmacUlDataBlock dec__RlcmacUlDataBlock(const OCTETSTRING& stream)
{
	RlcmacUlDataBlock ret_val;
	TTCN_Buffer ttcn_buffer(stream);
	int num_llc_blocks = 0;

	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("==================================\n"
				"dec_RlcmacUlDataBlock(): Stream before decoding: ");
	stream.log();
	TTCN_Logger::end_event();

	/* use automatic/generated decoder for header */
	ret_val.mac__hdr().decode(UlMacDataHeader_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): Stream after decoding hdr: ");
	ttcn_buffer.log();
	TTCN_Logger::end_event();
	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): ret_val after decoding hdr: ");
	ret_val.log();
	TTCN_Logger::end_event();

	/* Manually decoder remainder of ttcn_buffer, containing optional header octets,
	 * optional tlli, optional pfi and LLC Blocks */

	/* optional extension octets, containing LI+M+E of Llc blocks */
	if (ret_val.mac__hdr().e() == false) {
		/* extension octet follows, i.e. optional Llc length octets */
		while (1) {
			/* decode one more extension octet with LlcBlocHdr inside */
			LlcBlock lb;
			lb.hdr()().decode(LlcBlockHdr_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
			ret_val.blocks()[num_llc_blocks++] = lb;

			TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
			TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): Stream after decoding ExtOct: ");
			ttcn_buffer.log();
			TTCN_Logger::end_event();
			TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
			TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): ret_val after decoding ExtOct: ");
			ret_val.log();
			TTCN_Logger::end_event();

			/* if E == '1'B, we can proceed further */
			if (lb.hdr()().e() == true)
				break;
		}
	}

	/* parse optional TLLI */
	if (ret_val.mac__hdr().tlli__ind()) {
		ret_val.tlli() = OCTETSTRING(4, ttcn_buffer.get_read_data());
		ttcn_buffer.increase_pos(4);
	} else {
		ret_val.tlli() = OMIT_VALUE;
	}
	/* parse optional PFI */
	if (ret_val.mac__hdr().pfi__ind()) {
		ret_val.pfi().decode(RlcmacUlDataBlock_pfi_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
	} else {
		ret_val.pfi() = OMIT_VALUE;
	}

	/* RLC blocks at end */
	if (ret_val.mac__hdr().e() == true) {
		LlcBlock lb;
		unsigned int length = ttcn_buffer.get_read_len();
		/* LI not present: The Upper Layer PDU that starts with the current RLC data block either
		 * fills the current RLC data block precisely or continues in the following in-sequence RLC
		 * data block */
		lb.payload() = OCTETSTRING(length, ttcn_buffer.get_read_data());
		ttcn_buffer.increase_pos(length);
		ret_val.blocks()[0] = lb;
	} else {
		if (ret_val.blocks().is_bound()) {
			for (int i = 0; i < ret_val.blocks().size_of(); i++) {
				unsigned int length = ret_val.blocks()[i].hdr()().length__ind();
				if (length > ttcn_buffer.get_read_len())
					length = ttcn_buffer.get_read_len();
				ret_val.blocks()[i].payload() = OCTETSTRING(length, ttcn_buffer.get_read_data());
				ttcn_buffer.increase_pos(length);
			}
		}
	}

	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): Stream before return: ");
	ttcn_buffer.log();
	TTCN_Logger::end_event();
	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): ret_val before return: ");
	ret_val.log();
	TTCN_Logger::end_event();

	return ret_val;
}

static
EgprsUlMacDataHeader dec__EgprsUlMacDataHeader_type1(const OCTETSTRING& stream)
{
	EgprsUlMacDataHeader ret_val;

	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);

	return ret_val;
}

static
EgprsUlMacDataHeader dec__EgprsUlMacDataHeader_type2(const OCTETSTRING& stream)
{
	EgprsUlMacDataHeader ret_val;

	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);

	return ret_val;
}

static
EgprsUlMacDataHeader dec__EgprsUlMacDataHeader_type3(const OCTETSTRING& stream)
{
	EgprsUlMacDataHeader ret_val;

	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);

	return ret_val;
}

RlcmacUlEgprsDataBlock dec__RlcmacUlEgprsDataBlock(const OCTETSTRING& stream, CodingScheme::enum_type mcs)
{
	RlcmacUlEgprsDataBlock ret_val;
	TTCN_Buffer ttcn_buffer(stream);
	TTCN_Buffer aligned_buffer;
	int num_llc_blocks = 0;
	unsigned int data_block_bits, data_block_offsets[2];
	unsigned int num_calls;
	const uint8_t *ti_e;

	switch (mcs) {
	case CodingScheme::MCS__1:
	case CodingScheme::MCS__2:
	case CodingScheme::MCS__3:
	case CodingScheme::MCS__4:
		ret_val.mac__hdr() = dec__EgprsUlMacDataHeader_type3(stream);
		break;
	case CodingScheme::MCS__5:
	case CodingScheme::MCS__6:
		ret_val.mac__hdr() = dec__EgprsUlMacDataHeader_type2(stream);
		break;
	case CodingScheme::MCS__7:
	case CodingScheme::MCS__8:
	case CodingScheme::MCS__9:
		ret_val.mac__hdr() = dec__EgprsUlMacDataHeader_type1(stream);
		break;
	}
	setup_rlc_mac_priv(mcs, ret_val.mac__hdr().header__type(), true,
			   &num_calls, &data_block_bits, data_block_offsets);
	get_egprs_data_block(ttcn_buffer, data_block_offsets[0], data_block_bits, aligned_buffer);

	ti_e = aligned_buffer.get_read_data();
	ret_val.tlli__ind() = *ti_e & 0x02 ? true : false;
	ret_val.e() = *ti_e & 0x01 ? true : false;
	aligned_buffer.increase_pos(1);

	/* Manually decoder remainder of aligned_buffer, containing optional header octets,
	 * optional tlli, optional pfi and LLC Blocks */

	/* optional extension octets, containing LI+M+E of Llc blocks */
	if (ret_val.e() == false) {
		/* extension octet follows, i.e. optional Llc length octets */
		while (1) {
			/* decode one more extension octet with LlcBlocHdr inside */
			EgprsLlcBlock lb;
			lb.hdr()().decode(EgprsLlcBlockHdr_descr_, aligned_buffer, TTCN_EncDec::CT_RAW);
			ret_val.blocks()[num_llc_blocks++] = lb;

			/* if E == '1'B, we can proceed further */
			if (lb.hdr()().e() == true)
				break;
		}
	}

	/* parse optional TLLI */
	if (ret_val.tlli__ind()) {
		ret_val.tlli() = OCTETSTRING(4, aligned_buffer.get_read_data());
		aligned_buffer.increase_pos(4);
	} else {
		ret_val.tlli() = OMIT_VALUE;
	}
	/* parse optional PFI */
	if (ret_val.mac__hdr().pfi__ind()) {
		ret_val.pfi().decode(RlcmacUlEgprsDataBlock_pfi_descr_, aligned_buffer, TTCN_EncDec::CT_RAW);
	} else {
		ret_val.pfi() = OMIT_VALUE;
	}

	/* RLC blocks at end */
	if (ret_val.e() == true) {
		EgprsLlcBlock lb;
		unsigned int length = aligned_buffer.get_read_len();
		/* LI not present: The Upper Layer PDU that starts with the current RLC data block either
		 * fills the current RLC data block precisely or continues in the following in-sequence RLC
		 * data block */
		lb.payload() = OCTETSTRING(length, aligned_buffer.get_read_data());
		aligned_buffer.increase_pos(length);
		ret_val.blocks()[0] = lb;
	} else {
		if (ret_val.blocks().is_bound()) {
			for (int i = 0; i < ret_val.blocks().size_of(); i++) {
				unsigned int length = ret_val.blocks()[i].hdr()().length__ind();
				if (length > aligned_buffer.get_read_len())
					length = aligned_buffer.get_read_len();
				ret_val.blocks()[i].payload() = OCTETSTRING(length, aligned_buffer.get_read_data());
				aligned_buffer.increase_pos(length);
			}
		}
	}

	return ret_val;
}

RlcmacUlBlock dec__RlcmacUlBlock(const OCTETSTRING& stream)
{
	RlcmacUlBlock ret_val;
	size_t stream_len = stream.lengthof();
	CodingScheme::enum_type mcs = payload_len_2_coding_scheme(stream_len);
	unsigned char pt;

	switch (mcs) {
	case CodingScheme::CS__1:
	case CodingScheme::CS__2:
	case CodingScheme::CS__3:
	case CodingScheme::CS__4:
		pt = stream[0].get_octet() >> 6;
		if (pt == MacPayloadType::MAC__PT__RLC__DATA)
			ret_val.data() = dec__RlcmacUlDataBlock(stream);
		else
			ret_val.ctrl() = dec__RlcmacUlCtrlBlock(stream);
		break;
	case CodingScheme::MCS__1:
	case CodingScheme::MCS__2:
	case CodingScheme::MCS__3:
	case CodingScheme::MCS__4:
	case CodingScheme::MCS__5:
	case CodingScheme::MCS__6:
	case CodingScheme::MCS__7:
	case CodingScheme::MCS__8:
	case CodingScheme::MCS__9:
		ret_val.data__egprs() = dec__RlcmacUlEgprsDataBlock(stream, mcs);
		break;
	}

	return ret_val;
}


/////////////////////
// ENCODE
/////////////////////

/* ENCODE DOWNLINK */

OCTETSTRING enc__RlcmacDlDataBlock(const RlcmacDlDataBlock& si)
{
	RlcmacDlDataBlock in = si;
	OCTETSTRING ret_val;
	TTCN_Buffer ttcn_buffer;
	int i;

	/* Fix 'e' bit of initial header based on following blocks */
	if (!in.blocks().is_bound() ||
	    (in.blocks().size_of() == 1 && !in.blocks()[0].hdr().is_bound()))
		in.mac__hdr().hdr__ext().e() = true;
	else
		in.mac__hdr().hdr__ext().e() = false;

	/* use automatic/generated decoder for header */
	in.mac__hdr().encode(DlMacDataHeader_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	/* Add LI octets, if any */
	if (in.blocks().is_bound() &&
	    (in.blocks().size_of() != 1 || in.blocks()[0].hdr().is_bound())) {
		/* first write LI octets */
		for (i = 0; i < in.blocks().size_of(); i++) {
			/* fix the 'E' bit in case it is not clear */
			if (i < in.blocks().size_of()-1)
				in.blocks()[i].hdr()().e() = false;
			else
				in.blocks()[i].hdr()().e() = true;
			in.blocks()[i].hdr()().encode(LlcBlockHdr_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
		}
	}
	if (in.blocks().is_bound()) {
		for (i = 0; i < in.blocks().size_of(); i++) {
			if (!in.blocks()[i].is_bound())
				continue;
			ttcn_buffer.put_string(in.blocks()[i].payload());
		}
	}

	ttcn_buffer.get_string(ret_val);
	return ret_val;
}

static
void enc__RlcmacDlEgprsDataHeader_type1(const EgprsDlMacDataHeader& si, TTCN_Buffer& ttcn_buffer)
{
	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);
}

static
void enc__RlcmacDlEgprsDataHeader_type2(const EgprsDlMacDataHeader& si, TTCN_Buffer& ttcn_buffer)
{
	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);
}

static
void enc__RlcmacDlEgprsDataHeader_type3(const EgprsDlMacDataHeader& si, TTCN_Buffer& ttcn_buffer)
{
	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);
}

OCTETSTRING enc__RlcmacDlEgprsDataBlock(const RlcmacDlEgprsDataBlock& si)
{
	RlcmacDlEgprsDataBlock in = si;
	OCTETSTRING ret_val;
	TTCN_Buffer ttcn_buffer;
	int i;

	/* Fix 'e' bit of initial header based on following blocks */
	if (!in.blocks().is_bound() ||
	    (in.blocks().size_of() == 1 && !in.blocks()[0].hdr().is_bound()))
		in.e() = true;
	else
		in.e() = false;

	switch (in.mac__hdr().header__type()) {
	case EgprsHeaderType::RLCMAC__HDR__TYPE__1:
		enc__RlcmacDlEgprsDataHeader_type1(si.mac__hdr(), ttcn_buffer);
		break;
	case EgprsHeaderType::RLCMAC__HDR__TYPE__2:
		enc__RlcmacDlEgprsDataHeader_type2(si.mac__hdr(), ttcn_buffer);
		break;
	case EgprsHeaderType::RLCMAC__HDR__TYPE__3:
		enc__RlcmacDlEgprsDataHeader_type3(si.mac__hdr(), ttcn_buffer);
	default:
		break; /* TODO: error */
	}

	/* Add LI octets, if any */
	if (in.blocks().is_bound() &&
	    (in.blocks().size_of() != 1 || in.blocks()[0].hdr().is_bound())) {
		/* first write LI octets */
		for (i = 0; i < in.blocks().size_of(); i++) {
			/* fix the 'E' bit in case it is not clear */
			if (i < in.blocks().size_of()-1)
				in.blocks()[i].hdr()().e() = false;
			else
				in.blocks()[i].hdr()().e() = true;
			in.blocks()[i].hdr()().encode(EgprsLlcBlockHdr_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
		}
	}
	if (in.blocks().is_bound()) {
		for (i = 0; i < in.blocks().size_of(); i++) {
			if (!in.blocks()[i].is_bound())
				continue;
			ttcn_buffer.put_string(in.blocks()[i].payload());
		}
	}

	ttcn_buffer.get_string(ret_val);
	return ret_val;
}

OCTETSTRING enc__RlcmacDlBlock(const RlcmacDlBlock& si)
{
	if (si.ischosen(RlcmacDlBlock::ALT_data__egprs))
		return enc__RlcmacDlEgprsDataBlock(si.data__egprs());
	else if (si.ischosen(RlcmacDlBlock::ALT_data))
		return enc__RlcmacDlDataBlock(si.data());
	else
		return enc__RlcmacDlCtrlBlock(si.ctrl());
}

/* ENCODE UPLINK */

OCTETSTRING enc__RlcmacUlDataBlock(const RlcmacUlDataBlock& si)
{
	RlcmacUlDataBlock in = si;
	OCTETSTRING ret_val;
	TTCN_Buffer ttcn_buffer;
	int i;

	if (!in.blocks().is_bound()) {
		/* we don't have any blocks: Add length value (zero) */
		in.mac__hdr().e() = false; /* E=0: extension octet follows */
	} else if (in.blocks().size_of() == 1 && in.blocks()[0].hdr() == OMIT_VALUE) {
		/* If there's only a single block, and that block has no HDR value defined, */
		in.mac__hdr().e() = true; /* E=0: extension octet follows */
	} else {
		/* Length value */
		in.mac__hdr().e() = false;
	}

	/* Fix other presence indications */
	in.mac__hdr().tlli__ind() = in.tlli().is_bound() && in.tlli() != OMIT_VALUE;
	in.mac__hdr().pfi__ind() = in.pfi().is_bound() && in.pfi() != OMIT_VALUE;

	/* use automatic/generated encoder for header */
	in.mac__hdr().encode(UlMacDataHeader_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	if (in.mac__hdr().e() == false) {
		/* Add LI octets, if any */
		if (!in.blocks().is_bound()) {
			ttcn_buffer.put_c(0x01); /* M=0, E=1 LEN=0 */
		} else {
			for (i = 0; i < in.blocks().size_of(); i++) {
#if 0
				/* check for penultimate block */
				if (i == in.blocks().size_of()-2) {
					/* if last block has no header, no more LI */
					if (in.blocks()[i+1].hdr() == OMIT_VALUE) {
						in.blocks()[i].hdr()().more() = true;
					} else {
						/* header present, we have to encode LI */
						in.blocks()[i].hdr()().more() = false;
						in.blocks()[i].hdr()().length__ind() =
								in.blocks()[i+1].payload().lengthof();
					}
				} else if (i < in.blocks().size_of()-2) {
					/* one of the first blocks, before the penultimate or last */
					in.blocks()[i].hdr()().e() = false; /* LI present */
					/* re-compute length */
					in.blocks()[i].hdr()().length__ind() =
								in.blocks()[i+1].payload().lengthof();
				}
				/* Encode LI octet if E=0 */
				}
#endif
				if (in.blocks()[i].hdr() != OMIT_VALUE) {
					in.blocks()[i].hdr()().encode(LlcBlockHdr_descr_, ttcn_buffer,
									TTCN_EncDec::CT_RAW);
				}
			}
		}
	}

	if (in.mac__hdr().tlli__ind()) {
		ttcn_buffer.put_string(in.tlli());
	}

	if (in.mac__hdr().pfi__ind()) {
		in.pfi().encode(RlcmacUlDataBlock_pfi_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
	}

	if (in.blocks().is_bound()) {
		for (i = 0; i < in.blocks().size_of(); i++) {
			if (!in.blocks()[i].is_bound())
				continue;
			ttcn_buffer.put_string(in.blocks()[i].payload());
		}
	}

	ttcn_buffer.get_string(ret_val);
	return ret_val;
}

static
void enc__RlcmacUlEgprsDataHeader_type1(const EgprsUlMacDataHeader& si, TTCN_Buffer& ttcn_buffer)
{
	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);
}

static
void enc__RlcmacUlEgprsDataHeader_type2(const EgprsUlMacDataHeader& si, TTCN_Buffer& ttcn_buffer)
{
	fprintf(stderr, "FIXME: Not implemented! %s (%s:%u)\n", __func__, __FILE__, __LINE__);
}

static
void enc__RlcmacUlEgprsDataHeader_type3(const EgprsUlMacDataHeader& si, TTCN_Buffer& ttcn_buffer)
{
	struct gprs_rlc_ul_header_egprs_3 egprs3;

	egprs3.r = bs2uint8(si.r__ri());
	egprs3.si = bs2uint8(si.foi__si());
	egprs3.cv = si.countdown();
	egprs3.tfi_hi = si.tfi() >> 0;
	egprs3.tfi_lo = si.tfi() >> 1;
	egprs3.bsn1_hi = si.bsn1() >> 0;
	egprs3.bsn1_lo = si.bsn1() >> 5;
	egprs3.cps_hi = si.cps() >> 0;
	egprs3.cps_lo = si.cps() >> 2;
	egprs3.spb = bs2uint8(si.spb());
	egprs3.rsb = bs2uint8(si.spb());
	egprs3.pi = si.pfi__ind();
	egprs3.spare = 0;
	egprs3.dummy = 0;

	ttcn_buffer.put_s(sizeof(egprs3), (const unsigned char *)&egprs3);
}

OCTETSTRING enc__RlcmacUlEgprsDataBlock(const RlcmacUlEgprsDataBlock& si)
{
	RlcmacUlEgprsDataBlock in = si;
	OCTETSTRING ret_val;
	TTCN_Buffer ttcn_buffer;
	int i;

	if (!in.blocks().is_bound()) {
		/* we don't have nay blocks: Add length value (zero) */
		in.e() = false; /* E=0: extension octet follows */
	} else if (in.blocks().size_of() == 1 && in.blocks()[0].hdr() == OMIT_VALUE) {
		/* If there's only a single block, and that block has no HDR value defined, */
		in.e() = true; /* E=0: extension octet follows */
	} else {
		/* Length value */
		in.e() = false;
	}

	/* Fix other presence indications */
	in.tlli__ind() = in.tlli().is_bound() && in.tlli() != OMIT_VALUE;
	in.mac__hdr().pfi__ind() = in.pfi().is_bound() && in.pfi() != OMIT_VALUE;

	switch (in.mac__hdr().header__type()) {
	case EgprsHeaderType::RLCMAC__HDR__TYPE__1:
		enc__RlcmacUlEgprsDataHeader_type1(si.mac__hdr(), ttcn_buffer);
		break;
	case EgprsHeaderType::RLCMAC__HDR__TYPE__2:
		enc__RlcmacUlEgprsDataHeader_type2(si.mac__hdr(), ttcn_buffer);
		break;
	case EgprsHeaderType::RLCMAC__HDR__TYPE__3:
		enc__RlcmacUlEgprsDataHeader_type3(si.mac__hdr(), ttcn_buffer);
	default:
		break; /* TODO: error */
	}

	if (in.e() == false) {
		/* Add LI octets, if any */
		if (!in.blocks().is_bound()) {
			ttcn_buffer.put_c(0x01); /* M=0, E=1 LEN=0 */
		} else {
			for (i = 0; i < in.blocks().size_of(); i++) {
#if 0
				/* check for penultimate block */
				if (i == in.blocks().size_of()-2) {
					/* if last block has no header, no more LI */
					if (in.blocks()[i+1].hdr() == OMIT_VALUE) {
						in.blocks()[i].hdr()().more() = true;
					} else {
						/* header present, we have to encode LI */
						in.blocks()[i].hdr()().more() = false;
						in.blocks()[i].hdr()().length__ind() =
								in.blocks()[i+1].payload().lengthof();
					}
				} else if (i < in.blocks().size_of()-2) {
					/* one of the first blocks, before the penultimate or last */
					in.blocks()[i].hdr()().e() = false; /* LI present */
					/* re-compute length */
					in.blocks()[i].hdr()().length__ind() =
								in.blocks()[i+1].payload().lengthof();
				}
				/* Encode LI octet if E=0 */
				}
#endif
				if (in.blocks()[i].hdr() != OMIT_VALUE) {
					in.blocks()[i].hdr()().encode(EgprsLlcBlockHdr_descr_, ttcn_buffer,
									TTCN_EncDec::CT_RAW);
				}
			}
		}
	}

	if (in.tlli__ind()) {
		ttcn_buffer.put_string(in.tlli());
	}

	if (in.mac__hdr().pfi__ind()) {
		in.pfi().encode(RlcmacUlEgprsDataBlock_pfi_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
	}

	if (in.blocks().is_bound()) {
		for (i = 0; i < in.blocks().size_of(); i++) {
			if (!in.blocks()[i].is_bound())
				continue;
			ttcn_buffer.put_string(in.blocks()[i].payload());
		}
	}

	ttcn_buffer.get_string(ret_val);
	return ret_val;
}

OCTETSTRING enc__RlcmacUlBlock(const RlcmacUlBlock& si)
{
	if (si.ischosen(RlcmacUlBlock::ALT_data__egprs))
		return enc__RlcmacUlEgprsDataBlock(si.data__egprs());
	else if (si.ischosen(RlcmacUlBlock::ALT_data))
		return enc__RlcmacUlDataBlock(si.data());
	else
		return enc__RlcmacUlCtrlBlock(si.ctrl());
}

} // namespace
