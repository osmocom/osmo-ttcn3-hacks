#include "PKIX1Implicit88.hh"

namespace PKIX1Implicit88__Types {

using namespace PKIX1Implicit88;

TTCN_Module PKIX1Implicit88__EncDec("PKIX1Implicit88_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__SubjectKeyIdentifier(const SubjectKeyIdentifier &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(SubjectKeyIdentifier_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

SubjectKeyIdentifier dec__SubjectKeyIdentifier(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	SubjectKeyIdentifier msg;
	buf.put_os(stream);

	msg.decode(SubjectKeyIdentifier_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

}
