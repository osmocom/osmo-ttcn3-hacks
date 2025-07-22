
#include <string.h>
#include <stdarg.h>
#include "NGAP_PDU_Descriptions.hh"

namespace NGAP__Types {

TTCN_Module NGAP__EncDec("NGAP_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__NGAP__PDU(const NGAP__PDU__Descriptions::NGAP__PDU &pdu)
{
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	pdu.encode(NGAP__PDU__Descriptions::NGAP__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(TTCN_buf.get_len(), TTCN_buf.get_data());
}

NGAP__PDU__Descriptions::NGAP__PDU dec__NGAP__PDU(const OCTETSTRING &stream)
{
	NGAP__PDU__Descriptions::NGAP__PDU pdu;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	pdu.decode(NGAP__PDU__Descriptions::NGAP__PDU_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return pdu;
}

}
