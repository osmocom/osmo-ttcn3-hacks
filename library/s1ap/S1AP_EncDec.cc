#include "S1AP_PDU_Descriptions.hh"

/* 3GPP TS 36.413, section 9.4 Message transfer syntax:
 * S1AP shall use the ASN.1 Basic Packed Encoding Rules (BASIC-PER) Aligned Variant
 * as transfer syntax as specified in ref. ITU-T Rec. X.691 */

namespace S1AP__Types {

TTCN_Module S1AP__EncDec("S1AP_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__S1AP__PDU(const S1AP__PDU__Descriptions::S1AP__PDU &pdu)
{
	TTCN_Buffer buf;

	buf.clear();
	pdu.encode(S1AP__PDU__Descriptions::S1AP__PDU_descr_, buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

S1AP__PDU__Descriptions::S1AP__PDU dec__S1AP__PDU(const OCTETSTRING &stream)
{
	S1AP__PDU__Descriptions::S1AP__PDU pdu;
	TTCN_Buffer TTCN_buf;

	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	pdu.decode(S1AP__PDU__Descriptions::S1AP__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return pdu;
}

}
