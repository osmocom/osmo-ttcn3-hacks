
#include <string.h>
#include <stdarg.h>
#include "NGAP_IEs.hh"
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

OCTETSTRING enc__NGAP__PDUSessionResourceSetupRequestTransfer(const NGAP__IEs::PDUSessionResourceSetupRequestTransfer &p)
{
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	p.encode(NGAP__IEs::PDUSessionResourceSetupRequestTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(TTCN_buf.get_len(), TTCN_buf.get_data());
}

NGAP__IEs::PDUSessionResourceSetupRequestTransfer dec__NGAP__PDUSessionResourceSetupRequestTransfer(const OCTETSTRING &stream)
{
	NGAP__IEs::PDUSessionResourceSetupRequestTransfer ret;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	ret.decode(NGAP__IEs::PDUSessionResourceSetupRequestTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return ret;

}

OCTETSTRING enc__NGAP__PDUSessionResourceSetupResponseTransfer(const NGAP__IEs::PDUSessionResourceSetupResponseTransfer &p)
{
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	p.encode(NGAP__IEs::PDUSessionResourceSetupResponseTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(TTCN_buf.get_len(), TTCN_buf.get_data());
}

NGAP__IEs::PDUSessionResourceSetupResponseTransfer dec__NGAP__PDUSessionResourceSetupResponseTransfer(const OCTETSTRING &stream)
{
	NGAP__IEs::PDUSessionResourceSetupResponseTransfer ret;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	ret.decode(NGAP__IEs::PDUSessionResourceSetupResponseTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return ret;
}

OCTETSTRING enc__NGAP__PDUSessionResourceModifyRequestTransfer(const NGAP__IEs::PDUSessionResourceModifyRequestTransfer &p)
{
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	p.encode(NGAP__IEs::PDUSessionResourceModifyRequestTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(TTCN_buf.get_len(), TTCN_buf.get_data());
}

NGAP__IEs::PDUSessionResourceModifyRequestTransfer dec__NGAP__PDUSessionResourceModifyRequestTransfer(const OCTETSTRING &stream)
{
	NGAP__IEs::PDUSessionResourceModifyRequestTransfer ret;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	ret.decode(NGAP__IEs::PDUSessionResourceModifyRequestTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return ret;
}

OCTETSTRING enc__NGAP__PDUSessionResourceModifyResponseTransfer(const NGAP__IEs::PDUSessionResourceModifyResponseTransfer &p)
{
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	p.encode(NGAP__IEs::PDUSessionResourceModifyResponseTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(TTCN_buf.get_len(), TTCN_buf.get_data());
}

NGAP__IEs::PDUSessionResourceModifyResponseTransfer dec__NGAP__PDUSessionResourceModifyResponseTransfer(const OCTETSTRING &stream)
{
	NGAP__IEs::PDUSessionResourceModifyResponseTransfer ret;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	ret.decode(NGAP__IEs::PDUSessionResourceModifyResponseTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return ret;
}

OCTETSTRING enc__NGAP__UEContextSuspendRequestTransfer(const NGAP__IEs::UEContextSuspendRequestTransfer &p)
{
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	p.encode(NGAP__IEs::UEContextSuspendRequestTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(TTCN_buf.get_len(), TTCN_buf.get_data());
}

NGAP__IEs::UEContextSuspendRequestTransfer dec__NGAP__UEContextSuspendRequestTransfer(const OCTETSTRING &stream)
{
	NGAP__IEs::UEContextSuspendRequestTransfer ret;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	ret.decode(NGAP__IEs::UEContextSuspendRequestTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return ret;
}

OCTETSTRING enc__NGAP__PathSwitchRequestTransfer(const NGAP__IEs::PathSwitchRequestTransfer &p)
{
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	p.encode(NGAP__IEs::PathSwitchRequestTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return OCTETSTRING(TTCN_buf.get_len(), TTCN_buf.get_data());
}

NGAP__IEs::PathSwitchRequestTransfer dec__NGAP__PathSwitchRequestTransfer(const OCTETSTRING &stream)
{
	NGAP__IEs::PathSwitchRequestTransfer ret;
	TTCN_Buffer TTCN_buf;
	TTCN_buf.clear();
	TTCN_buf.put_os(stream);
	ret.decode(NGAP__IEs::PathSwitchRequestTransfer_descr_, TTCN_buf,
		   TTCN_EncDec::CT_PER, PER_ALIGNED);
	return ret;
}

}
