#include "RSPDefinitions.hh"

namespace RSPDefinitions__Types {

using namespace RSPDefinitions;

TTCN_Module RSPDefinitions__EncDec("RSPDefinitions_EncDec", __DATE__, __TIME__);

OCTETSTRING enc__GetEuiccChallengeResponse(const GetEuiccChallengeResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(GetEuiccChallengeResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

GetEuiccChallengeResponse dec__GetEuiccChallengeResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	GetEuiccChallengeResponse msg;
	buf.put_os(stream);

	msg.decode(GetEuiccChallengeResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__EUICCInfo1(const EUICCInfo1 &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(EUICCInfo1_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

EUICCInfo1 dec__EUICCInfo1(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	EUICCInfo1 msg;
	buf.put_os(stream);

	msg.decode(EUICCInfo1_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__EUICCInfo2(const EUICCInfo2 &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(EUICCInfo2_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

EUICCInfo2 dec__EUICCInfo2(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	EUICCInfo2 msg;
	buf.put_os(stream);

	msg.decode(EUICCInfo2_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__AuthenticateServerResponse(const AuthenticateServerResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(AuthenticateServerResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

AuthenticateServerResponse dec__AuthenticateServerResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	AuthenticateServerResponse msg;
	buf.put_os(stream);

	msg.decode(AuthenticateServerResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__CancelSessionResponse(const CancelSessionResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(CancelSessionResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

CancelSessionResponse dec__CancelSessionResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	CancelSessionResponse msg;
	buf.put_os(stream);

	msg.decode(CancelSessionResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__PrepareDownloadResponse(const PrepareDownloadResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(PrepareDownloadResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

PrepareDownloadResponse dec__PrepareDownloadResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	PrepareDownloadResponse msg;
	buf.put_os(stream);

	msg.decode(PrepareDownloadResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__ProfileInstallationResult(const ProfileInstallationResult &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(ProfileInstallationResult_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

ProfileInstallationResult dec__ProfileInstallationResult(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	ProfileInstallationResult msg;
	buf.put_os(stream);

	msg.decode(ProfileInstallationResult_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__NotificationSentResponse(const NotificationSentResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(NotificationSentResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

NotificationSentResponse dec__NotificationSentResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	NotificationSentResponse msg;
	buf.put_os(stream);

	msg.decode(NotificationSentResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__RetrieveNotificationsListResponse(const RetrieveNotificationsListResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(RetrieveNotificationsListResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

RetrieveNotificationsListResponse dec__RetrieveNotificationsListResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	RetrieveNotificationsListResponse msg;
	buf.put_os(stream);

	msg.decode(RetrieveNotificationsListResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__GetEuiccDataResponse(const GetEuiccDataResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(GetEuiccDataResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

GetEuiccDataResponse dec__GetEuiccDataResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	GetEuiccDataResponse msg;
	buf.put_os(stream);

	msg.decode(GetEuiccDataResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__EuiccConfiguredAddressesResponse(const EuiccConfiguredAddressesResponse &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(EuiccConfiguredAddressesResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

EuiccConfiguredAddressesResponse dec__EuiccConfiguredAddressesResponse(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	EuiccConfiguredAddressesResponse msg;
	buf.put_os(stream);

	msg.decode(EuiccConfiguredAddressesResponse_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__PendingNotification(const PendingNotification &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(PendingNotification_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

PendingNotification dec__PendingNotification(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	PendingNotification msg;
	buf.put_os(stream);

	msg.decode(PendingNotification_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__ServerSigned1(const ServerSigned1 &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(ServerSigned1_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

ServerSigned1 dec__ServerSigned1(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	ServerSigned1 msg;
	buf.put_os(stream);

	msg.decode(ServerSigned1_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__BoundProfilePackage(const BoundProfilePackage &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(BoundProfilePackage_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

BoundProfilePackage dec__BoundProfilePackage(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	BoundProfilePackage msg;
	buf.put_os(stream);

	msg.decode(BoundProfilePackage_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__StoreMetadataRequest(const StoreMetadataRequest &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(StoreMetadataRequest_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

StoreMetadataRequest dec__StoreMetadataRequest(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	StoreMetadataRequest msg;
	buf.put_os(stream);

	msg.decode(StoreMetadataRequest_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

OCTETSTRING enc__SmdpSigned2(const SmdpSigned2 &msg) {
	TTCN_Buffer buf;

	buf.clear();
	msg.encode(SmdpSigned2_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
	return OCTETSTRING(buf.get_len(), buf.get_data());
}

SmdpSigned2 dec__SmdpSigned2(const OCTETSTRING &stream) {
	TTCN_Buffer buf;
	SmdpSigned2 msg;
	buf.put_os(stream);

	msg.decode(SmdpSigned2_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
	return msg;
}

}
