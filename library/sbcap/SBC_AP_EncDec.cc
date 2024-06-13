#include "SBC_AP_PDU_Descriptions.hh"

/* 3GPP TS 29.168, section 9.4 Message transfer syntax:
 * SBC-AP shall use the ASN.1 Basic Packed Encoding Rules (BASIC-PER) Aligned Variant
 * as transfer syntax as specified in ref. ITU-T Rec. X.691 */

namespace SBC__AP__Types {

TTCN_Module SBC__AP__EncDec("SBC_AP_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__SBC__AP__PDU(const SBC__AP__PDU__Descriptions::SBC__AP__PDU &pdu)
{
	TTCN_Buffer buf;

	buf.clear();
	pdu.encode(SBC__AP__PDU__Descriptions::SBC__AP__PDU_descr_, buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

SBC__AP__PDU__Descriptions::SBC__AP__PDU dec__SBC__AP__PDU(const OCTETSTRING &stream)
{
	SBC__AP__PDU__Descriptions::SBC__AP__PDU pdu;
	TTCN_Buffer TTCN_buf;

	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	pdu.decode(SBC__AP__PDU__Descriptions::SBC__AP__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return pdu;
}

}
