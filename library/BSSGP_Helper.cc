
#include "Octetstring.hh"
#include "Error.hh"
#include "Logger.hh"

#include <stdint.h>

namespace BSSGP__Helper__Functions {

/* convert a buffer filled with TLVs that have variable-length "length" fields (Osmocom TvLV) into a
 * buffer filled with TLVs that have fixed 16-bit length values (TL16V format) */
static OCTETSTRING expand_tlv_part(OCTETSTRING const &in)
{
	const unsigned char *in_ptr = (const unsigned char *)in;
	int in_len = in.lengthof();
	int ofs = 0;
	uint16_t data_len;
	OCTETSTRING out(0, (const unsigned char *)"");

	while (ofs < in_len) {
		int remain_len = in_len - ofs;
		int tl_length;

		if (remain_len < 2) {
			TTCN_error("Remaining input length (%d) insufficient for Tag+Length", remain_len);
			break;
		}

		/* copy over tag */
		if (in_ptr[ofs+1] & 0x80) {
			/* E bit is set, 7-bit length field */
			data_len = in_ptr[ofs+1] & 0x7F;
			tl_length = 2;
		} else {
			/* E bit is not set, 15 bit length field */
			if (remain_len < 3) {
				TTCN_error("Remaining input length insufficient for 2-octet length");
				break;
			}
			data_len = in_ptr[ofs+1] << 8 | in_ptr[ofs+2];
			tl_length = 3;
		}
		if (in_len < tl_length + data_len) {
			TTCN_error("Remaining input length insufficient for TLV value length");
			break;
		}

		/* Tag + 16bit length */
		uint8_t hdr_buf[3];
		hdr_buf[0] = in_ptr[ofs+0];
		hdr_buf[1] = data_len >> 8;
		hdr_buf[2] = data_len & 0xff;

		OCTETSTRING tlv_hdr(3, hdr_buf);
		out += tlv_hdr;

		if (data_len) {
			/* append octet string of current TLV to output octetstring */
			OCTETSTRING tlv_val(data_len, in_ptr + ofs + tl_length);
			out += tlv_val;
		}

		/* advance input offset*/
		ofs += data_len + tl_length;
	}

	return out;
}

/* convert a buffer filled with TLVs that have fixed-length "length" fields (Osmocom TL16V) into a
 * buffer filled with TLVs that have variable-length values (TvLV format) */
static OCTETSTRING compact_tlv_part(OCTETSTRING const &in)
{
	const unsigned char *in_ptr = (const unsigned char *)in;
	int in_len = in.lengthof();
	int ofs = 0;
	uint16_t data_len;
	OCTETSTRING out(0, (const unsigned char *)"");

	while (ofs < in_len) {
		int remain_len = in_len - ofs;
		int tl_length;

		if (remain_len < 3) {
			TTCN_error("Remaining input length (%d) insufficient for Tag+Length", remain_len);
			break;
		}

		data_len = (in_ptr[ofs+1] << 8) | in_ptr[ofs+2];

		/* Tag + 16bit length */
		uint8_t hdr_buf[3];
		hdr_buf[0] = in_ptr[ofs+0];

		if (data_len <= 0x7f) {
			/* E bit is set, 7-bit length field */
			hdr_buf[1] = 0x80 | data_len;
			tl_length = 2;
		} else {
			/* E bit is not set, 15 bit length field */
			hdr_buf[1] = data_len >> 8;
			hdr_buf[2] = data_len & 0xff;
			tl_length = 3;
		}
		if (in_len < 3 + data_len) {
			TTCN_error("Remaining input length insufficient for TLV value length");
			break;
		}

		OCTETSTRING tlv_hdr(tl_length, hdr_buf);
		out += tlv_hdr;

		if (data_len) {
			/* append octet string of current TLV to output octetstring */
			OCTETSTRING tlv_val(data_len, in_ptr + ofs + 3);
			out += tlv_val;
		}

		/* advance input offset*/
		ofs += data_len + 3;
	}

	return out;
}

#define BSSGP_PDUT_DL_UNITDATA	0x00
#define BSSGP_PDUT_UL_UNITDATA	0x01

/* expand all the variable-length "length" fields of a BSSGP message (Osmocom TvLV) into statlc TL16V format */
OCTETSTRING f__BSSGP__expand__len(OCTETSTRING const &in)
{
	const unsigned char *in_ptr = (const unsigned char *)in;
	int in_len = in.lengthof();
	uint8_t pdu_type = in_ptr[0];
	uint8_t static_hdr_len = 1;

	if (pdu_type == BSSGP_PDUT_DL_UNITDATA || pdu_type == BSSGP_PDUT_UL_UNITDATA)
		static_hdr_len = 8;

	if (in_len < static_hdr_len)
		TTCN_error("BSSGP message is shorter (%u bytes) than minimum header length (%u bytes) for msg_type 0x%02x",
				in_len, static_hdr_len, pdu_type);

	/* prefix = non-TLV section of header */
	OCTETSTRING prefix(static_hdr_len, in_ptr);
	OCTETSTRING tlv_part_in(in_len - static_hdr_len, in_ptr + static_hdr_len);

	return prefix + expand_tlv_part(tlv_part_in);
}

OCTETSTRING f__BSSGP__compact__len(OCTETSTRING const &in)
{
	const unsigned char *in_ptr = (const unsigned char *)in;
	int in_len = in.lengthof();
	uint8_t pdu_type = in_ptr[0];
	uint8_t static_hdr_len = 1;

	if (pdu_type == BSSGP_PDUT_DL_UNITDATA || pdu_type == BSSGP_PDUT_UL_UNITDATA)
		static_hdr_len = 8;

	if (in_len < static_hdr_len)
		TTCN_error("BSSGP message is shorter (%u bytes) than minimum header length (%u bytes) for msg_type 0x%02x",
				in_len, static_hdr_len, pdu_type);

	/* prefix = non-TLV section of header */
	OCTETSTRING prefix(static_hdr_len, in_ptr);
	OCTETSTRING tlv_part_in(in_len - static_hdr_len, in_ptr + static_hdr_len);

	return prefix + compact_tlv_part(tlv_part_in);
}

#define NS_PDUT_NS_UNITDATA	0x00

/* expand all the variable-length "length" fields of a NS message (Osmocom TvLV) into statlc TL16V format */
OCTETSTRING f__NS__expand__len(OCTETSTRING const &in)
{
	const unsigned char *in_ptr = (const unsigned char *)in;
	int in_len = in.lengthof();
	uint8_t pdu_type = in_ptr[0];
	uint8_t static_hdr_len = 1;

	if (pdu_type == NS_PDUT_NS_UNITDATA)
		return in;

	if (in_len < static_hdr_len)
		TTCN_error("NS message is shorter (%u bytes) than minimum header length (%u bytes) for msg_type 0x%02x",
				in_len, static_hdr_len, pdu_type);

	/* prefix = non-TLV section of header */
	OCTETSTRING prefix(static_hdr_len, in_ptr);
	OCTETSTRING tlv_part_in(in_len - static_hdr_len, in_ptr + static_hdr_len);

	return prefix + expand_tlv_part(tlv_part_in);
}

OCTETSTRING f__NS__compact__len(OCTETSTRING const &in)
{
	const unsigned char *in_ptr = (const unsigned char *)in;
	int in_len = in.lengthof();
	uint8_t pdu_type = in_ptr[0];
	uint8_t static_hdr_len = 1;

	if (pdu_type == NS_PDUT_NS_UNITDATA)
		return in;

	if (in_len < static_hdr_len)
		TTCN_error("NS message is shorter (%u bytes) than minimum header length (%u bytes) for msg_type 0x%02x",
				in_len, static_hdr_len, pdu_type);

	/* prefix = non-TLV section of header */
	OCTETSTRING prefix(static_hdr_len, in_ptr);
	OCTETSTRING tlv_part_in(in_len - static_hdr_len, in_ptr + static_hdr_len);

	return prefix + compact_tlv_part(tlv_part_in);
}



/* GPRS LLC CRC-24 Implementation */

/* (C) 2008-2009 by Harald Welte <laforge@gnumonks.org>
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

/* CRC24 table - FCS */
static const uint32_t tbl_crc24[256] = {
	0x00000000, 0x00d6a776, 0x00f64557, 0x0020e221, 0x00b78115, 0x00612663, 0x0041c442, 0x00976334,
	0x00340991, 0x00e2aee7, 0x00c24cc6, 0x0014ebb0, 0x00838884, 0x00552ff2, 0x0075cdd3, 0x00a36aa5,
	0x00681322, 0x00beb454, 0x009e5675, 0x0048f103, 0x00df9237, 0x00093541, 0x0029d760, 0x00ff7016,
	0x005c1ab3, 0x008abdc5, 0x00aa5fe4, 0x007cf892, 0x00eb9ba6, 0x003d3cd0, 0x001ddef1, 0x00cb7987,
	0x00d02644, 0x00068132, 0x00266313, 0x00f0c465, 0x0067a751, 0x00b10027, 0x0091e206, 0x00474570,
	0x00e42fd5, 0x003288a3, 0x00126a82, 0x00c4cdf4, 0x0053aec0, 0x008509b6, 0x00a5eb97, 0x00734ce1,
	0x00b83566, 0x006e9210, 0x004e7031, 0x0098d747, 0x000fb473, 0x00d91305, 0x00f9f124, 0x002f5652,
	0x008c3cf7, 0x005a9b81, 0x007a79a0, 0x00acded6, 0x003bbde2, 0x00ed1a94, 0x00cdf8b5, 0x001b5fc3,
	0x00fb4733, 0x002de045, 0x000d0264, 0x00dba512, 0x004cc626, 0x009a6150, 0x00ba8371, 0x006c2407,
	0x00cf4ea2, 0x0019e9d4, 0x00390bf5, 0x00efac83, 0x0078cfb7, 0x00ae68c1, 0x008e8ae0, 0x00582d96,
	0x00935411, 0x0045f367, 0x00651146, 0x00b3b630, 0x0024d504, 0x00f27272, 0x00d29053, 0x00043725,
	0x00a75d80, 0x0071faf6, 0x005118d7, 0x0087bfa1, 0x0010dc95, 0x00c67be3, 0x00e699c2, 0x00303eb4,
	0x002b6177, 0x00fdc601, 0x00dd2420, 0x000b8356, 0x009ce062, 0x004a4714, 0x006aa535, 0x00bc0243,
	0x001f68e6, 0x00c9cf90, 0x00e92db1, 0x003f8ac7, 0x00a8e9f3, 0x007e4e85, 0x005eaca4, 0x00880bd2,
	0x00437255, 0x0095d523, 0x00b53702, 0x00639074, 0x00f4f340, 0x00225436, 0x0002b617, 0x00d41161,
	0x00777bc4, 0x00a1dcb2, 0x00813e93, 0x005799e5, 0x00c0fad1, 0x00165da7, 0x0036bf86, 0x00e018f0,
	0x00ad85dd, 0x007b22ab, 0x005bc08a, 0x008d67fc, 0x001a04c8, 0x00cca3be, 0x00ec419f, 0x003ae6e9,
	0x00998c4c, 0x004f2b3a, 0x006fc91b, 0x00b96e6d, 0x002e0d59, 0x00f8aa2f, 0x00d8480e, 0x000eef78,
	0x00c596ff, 0x00133189, 0x0033d3a8, 0x00e574de, 0x007217ea, 0x00a4b09c, 0x008452bd, 0x0052f5cb,
	0x00f19f6e, 0x00273818, 0x0007da39, 0x00d17d4f, 0x00461e7b, 0x0090b90d, 0x00b05b2c, 0x0066fc5a,
	0x007da399, 0x00ab04ef, 0x008be6ce, 0x005d41b8, 0x00ca228c, 0x001c85fa, 0x003c67db, 0x00eac0ad,
	0x0049aa08, 0x009f0d7e, 0x00bfef5f, 0x00694829, 0x00fe2b1d, 0x00288c6b, 0x00086e4a, 0x00dec93c,
	0x0015b0bb, 0x00c317cd, 0x00e3f5ec, 0x0035529a, 0x00a231ae, 0x007496d8, 0x005474f9, 0x0082d38f,
	0x0021b92a, 0x00f71e5c, 0x00d7fc7d, 0x00015b0b, 0x0096383f, 0x00409f49, 0x00607d68, 0x00b6da1e,
	0x0056c2ee, 0x00806598, 0x00a087b9, 0x007620cf, 0x00e143fb, 0x0037e48d, 0x001706ac, 0x00c1a1da,
	0x0062cb7f, 0x00b46c09, 0x00948e28, 0x0042295e, 0x00d54a6a, 0x0003ed1c, 0x00230f3d, 0x00f5a84b,
	0x003ed1cc, 0x00e876ba, 0x00c8949b, 0x001e33ed, 0x008950d9, 0x005ff7af, 0x007f158e, 0x00a9b2f8,
	0x000ad85d, 0x00dc7f2b, 0x00fc9d0a, 0x002a3a7c, 0x00bd5948, 0x006bfe3e, 0x004b1c1f, 0x009dbb69,
	0x0086e4aa, 0x005043dc, 0x0070a1fd, 0x00a6068b, 0x003165bf, 0x00e7c2c9, 0x00c720e8, 0x0011879e,
	0x00b2ed3b, 0x00644a4d, 0x0044a86c, 0x00920f1a, 0x00056c2e, 0x00d3cb58, 0x00f32979, 0x00258e0f,
	0x00eef788, 0x003850fe, 0x0018b2df, 0x00ce15a9, 0x0059769d, 0x008fd1eb, 0x00af33ca, 0x007994bc,
	0x00dafe19, 0x000c596f, 0x002cbb4e, 0x00fa1c38, 0x006d7f0c, 0x00bbd87a, 0x009b3a5b, 0x004d9d2d
};

#define INIT_CRC24	0xffffff

static uint32_t crc24_calc(uint32_t fcs, const unsigned char *cp, int len)
{
	while (len--)
		fcs = (fcs >> 8) ^ tbl_crc24[(fcs ^ *cp++) & 0xff];
	return fcs;
}

OCTETSTRING f__LLC__compute__fcs(OCTETSTRING const &in)
{
	uint32_t fcs_calc;
	unsigned char fcs_buf[3];
	const unsigned char *data = (const unsigned char *)in;
	int len = in.lengthof();

	fcs_calc = crc24_calc(INIT_CRC24, data, len);
	fcs_calc = ~fcs_calc;
	fcs_calc &= 0xffffff;

	fcs_buf[0] = fcs_calc & 0xff;
	fcs_buf[1] = (fcs_calc >> 8) & 0xff;
	fcs_buf[2] = (fcs_calc >> 16) & 0xff;

	return OCTETSTRING(3, fcs_buf);
}

}
