#include "RANAP_PDU_Descriptions.hh"

/* 3GPP TS 25.413, section 9.4 Message transfer syntax:
 * RANAP shall use the ASN.1 Basic Packed Encoding Rules (BASIC-PER) Aligned Variant
 * as transfer syntax as specified in ref. ITU-T Rec. X.691 */

namespace RANAP__Types {

TTCN_Module RANAP__EncDec("RANAP_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__RANAP__PDU(const RANAP__PDU__Descriptions::RANAP__PDU &pdu)
{
	TTCN_Buffer buf;

	buf.clear();
	pdu.encode(RANAP__PDU__Descriptions::RANAP__PDU_descr_, buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

RANAP__PDU__Descriptions::RANAP__PDU dec__RANAP__PDU(const OCTETSTRING &stream)
{
	RANAP__PDU__Descriptions::RANAP__PDU pdu;
	TTCN_Buffer TTCN_buf;

	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	pdu.decode(RANAP__PDU__Descriptions::RANAP__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return pdu;
}

}
