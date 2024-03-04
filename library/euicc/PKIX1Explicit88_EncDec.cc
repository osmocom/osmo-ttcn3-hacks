#include "PKIX1Explicit88.hh"

namespace PKIX1Explicit88__Types {

using namespace PKIX1Explicit88;

TTCN_Module PKIX1Explicit88__EncDec("PKIX1Explicit88_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__X520name(const X520name &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520name_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520name dec__X520name(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520name msg;
	buf.put_os(stream);

	msg.decode(X520name_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520CommonName(const X520CommonName &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520CommonName_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520CommonName dec__X520CommonName(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520CommonName msg;
	buf.put_os(stream);

	msg.decode(X520CommonName_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520LocalityName(const X520LocalityName &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520LocalityName_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520LocalityName dec__X520LocalityName(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520LocalityName msg;
	buf.put_os(stream);

	msg.decode(X520LocalityName_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520StateOrProvinceName(const X520StateOrProvinceName &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520StateOrProvinceName_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520StateOrProvinceName dec__X520StateOrProvinceName(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520StateOrProvinceName msg;
	buf.put_os(stream);

	msg.decode(X520StateOrProvinceName_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520OrganizationName(const X520OrganizationName &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520OrganizationName_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520OrganizationName dec__X520OrganizationName(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520OrganizationName msg;
	buf.put_os(stream);

	msg.decode(X520OrganizationName_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520OrganizationalUnitName(const X520OrganizationalUnitName &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520OrganizationalUnitName_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520OrganizationalUnitName dec__X520OrganizationalUnitName(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520OrganizationalUnitName msg;
	buf.put_os(stream);

	msg.decode(X520OrganizationalUnitName_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520Title(const X520Title &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520Title_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520Title dec__X520Title(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520Title msg;
	buf.put_os(stream);

	msg.decode(X520Title_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520dnQualifier(const X520dnQualifier &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520dnQualifier_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520dnQualifier dec__X520dnQualifier(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520dnQualifier msg;
	buf.put_os(stream);

	msg.decode(X520dnQualifier_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520countryName(const X520countryName &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520countryName_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520countryName dec__X520countryName(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520countryName msg;
	buf.put_os(stream);

	msg.decode(X520countryName_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520SerialNumber(const X520SerialNumber &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520SerialNumber_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520SerialNumber dec__X520SerialNumber(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520SerialNumber msg;
	buf.put_os(stream);

	msg.decode(X520SerialNumber_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__X520Pseudonym(const X520Pseudonym &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(X520Pseudonym_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

X520Pseudonym dec__X520Pseudonym(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	X520Pseudonym msg;
	buf.put_os(stream);

	msg.decode(X520Pseudonym_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__Certificate(const Certificate &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(Certificate_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

Certificate dec__Certificate(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	Certificate msg;
	buf.put_os(stream);

	msg.decode(Certificate_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

}
