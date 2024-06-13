#include "RUA_PDU_Descriptions.hh"

/* 3GPP TS 25.468, section 9.4 Message transfer syntax:
 * RUA shall use the ASN.1 Basic Packed Encoding Rules (BASIC-PER) Aligned Variant
 * as transfer syntax as specified in ref. ITU-T Rec. X.691 */

namespace RUA__Types {

TTCN_Module RUA__EncDec("RUA_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__RUA__PDU(const RUA__PDU__Descriptions::RUA__PDU &pdu)
{
	TTCN_Buffer buf;

	buf.clear();
	pdu.encode(RUA__PDU__Descriptions::RUA__PDU_descr_, buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

RUA__PDU__Descriptions::RUA__PDU dec__RUA__PDU(const OCTETSTRING &stream)
{
	RUA__PDU__Descriptions::RUA__PDU pdu;
	TTCN_Buffer TTCN_buf;

	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	pdu.decode(RUA__PDU__Descriptions::RUA__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return pdu;
}

}
