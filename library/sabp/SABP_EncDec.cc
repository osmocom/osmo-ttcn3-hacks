#include "SABP_PDU_Descriptions.hh"

/* 3GPP TS 25.419, section 9.4 Message transfer syntax:
 * SABP shall use the ASN.1 Basic Packed Encoding Rules (BASIC-PER) Aligned Variant
 * as transfer syntax as specified in ref. ITU-T Rec. X.691 */

namespace SABP__Types {

TTCN_Module SABP__EncDec("SABP_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__SABP__PDU(const SABP__PDU__Descriptions::SABP__PDU &pdu)
{
	TTCN_Buffer buf;

	buf.clear();
	pdu.encode(SABP__PDU__Descriptions::SABP__PDU_descr_, buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

SABP__PDU__Descriptions::SABP__PDU dec__SABP__PDU(const OCTETSTRING &stream)
{
	SABP__PDU__Descriptions::SABP__PDU pdu;
	TTCN_Buffer TTCN_buf;

	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	pdu.decode(SABP__PDU__Descriptions::SABP__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return pdu;
}

}
