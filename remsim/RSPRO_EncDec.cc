#include "RSPRO.hh"

namespace RSPRO__Types {

using namespace RSPRO;

TTCN_Module RSPRO__EncDec("RSPRO_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__RsproPDU(const RsproPDU& pdu) {
	TTCN_Buffer buf;

	buf.clear();
	pdu.encode(RsproPDU_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

RsproPDU dec__RsproPDU(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	RsproPDU pdu;
	buf.put_os(stream);

	pdu.decode(RsproPDU_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return pdu;
}

}
