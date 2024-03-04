#include "SGP32Definitions.hh"

namespace SGP32Definitions__Types {

using namespace SGP32Definitions;
using namespace PKIX1Explicit88;

TTCN_Module SGP32Definitions__EncDec("SGP32Definitions_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__EsipaMessageFromIpaToEim(const EsipaMessageFromIpaToEim &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(EsipaMessageFromIpaToEim_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

EsipaMessageFromIpaToEim dec__EsipaMessageFromIpaToEim(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	EsipaMessageFromIpaToEim msg;
	buf.put_os(stream);

	msg.decode(EsipaMessageFromIpaToEim_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__EsipaMessageFromEimToIpa(const EsipaMessageFromEimToIpa &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(EsipaMessageFromEimToIpa_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

EsipaMessageFromEimToIpa dec__EsipaMessageFromEimToIpa(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	EsipaMessageFromEimToIpa msg;
	buf.put_os(stream);

	msg.decode(EsipaMessageFromEimToIpa_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__EuiccPackageResult(const EuiccPackageResult &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(EuiccPackageResult_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

EuiccPackageResult dec__EuiccPackageResult(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	EuiccPackageResult msg;
	buf.put_os(stream);

	msg.decode(EuiccPackageResult_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__GetCertsResponse(const GetCertsResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(GetCertsResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

GetCertsResponse dec__GetCertsResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	GetCertsResponse msg;
	buf.put_os(stream);

	msg.decode(GetCertsResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__GetEimConfigurationDataResponse(const GetEimConfigurationDataResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(GetEimConfigurationDataResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

GetEimConfigurationDataResponse dec__GetEimConfigurationDataResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	GetEimConfigurationDataResponse msg;
	buf.put_os(stream);

	msg.decode(GetEimConfigurationDataResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__AddInitialEimResponse(const AddInitialEimResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(AddInitialEimResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

AddInitialEimResponse dec__AddInitialEimResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	AddInitialEimResponse msg;
	buf.put_os(stream);

	msg.decode(AddInitialEimResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}


}
