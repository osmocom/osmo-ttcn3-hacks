#include "HNBAP_PDU_Descriptions.hh"

/* 3GPP TS 25.469, section 9.4 Message transfer syntax:
 * HNBAP shall use the ASN.1 Basic Packed Encoding Rules (BASIC-PER) Aligned Variant
 * as transfer syntax as specified in ref. ITU-T Rec. X.691 */

namespace HNBAP__Types {

TTCN_Module HNBAP__EncDec("HNBAP_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__HNBAP__PDU(const HNBAP__PDU__Descriptions::HNBAP__PDU &pdu)
{
	TTCN_Buffer buf;

	buf.clear();
	pdu.encode(HNBAP__PDU__Descriptions::HNBAP__PDU_descr_, buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

HNBAP__PDU__Descriptions::HNBAP__PDU dec__HNBAP__PDU(const OCTETSTRING &stream)
{
	HNBAP__PDU__Descriptions::HNBAP__PDU pdu;
	TTCN_Buffer TTCN_buf;

	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	pdu.decode(HNBAP__PDU__Descriptions::HNBAP__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return pdu;
}

}
