
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

}
