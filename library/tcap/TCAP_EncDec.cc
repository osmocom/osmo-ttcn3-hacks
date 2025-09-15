
#include <string.h>
#include <stdarg.h>
#include "TCAPMessages.hh"
namespace TCAP__Types {

TTCN_Module TCAP__EncDec("TCAP_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__TCAP__TCMessage(const TCAPMessages::TCMessage &pdu)
{
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	pdu.encode(TCAPMessages::TCMessage_descr_, TTCN_buf,
		   TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(TTCN_buf.get_len(), TTCN_buf.get_data());
}

TCAPMessages::TCMessage dec__TCAP__TCMessage(const OCTETSTRING &stream)
{
	TCAPMessages::TCMessage pdu;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	pdu.decode(TCAPMessages::TCMessage_descr_, TTCN_buf,
		   TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return pdu;
}


}
