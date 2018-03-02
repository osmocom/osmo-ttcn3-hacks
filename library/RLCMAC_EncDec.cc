#include "RLCMAC_Types.hh"
#include "GSM_Types.hh"
/* Decoding of TS 44.060 GPRS RLC/MAC blocks, portions requiring manual functions
 * beyond what TITAN RAW coder can handle internally.
 *
 * (C) 2017 by Harald Welte <laforge@gnumonks.org>
 */

namespace RLCMAC__Types {

OCTETSTRING enc__RlcmacDlDataBlock(const RlcmacDlDataBlock& si)
{
	RlcmacDlDataBlock in = si;
	OCTETSTRING ret_val;
	TTCN_Buffer ttcn_buffer;
	int i;

	/* Fix 'e' bit of initial header based on following blocks */
	if (!in.blocks().is_bound() ||
	    (in.blocks().size_of() == 1 && !in.blocks()[0].hdr().is_bound()))
		in.mac__hdr().hdr__ext().e() = true;
	else
		in.mac__hdr().hdr__ext().e() = false;

	/* use automatic/generated decoder for header */
	in.mac__hdr().encode(DlMacDataHeader_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	/* Add LI octets, if any */
	if (in.blocks().is_bound() &&
	    (in.blocks().size_of() != 1 || in.blocks()[0].hdr().is_bound())) {
		/* first write LI octets */
		for (i = 0; i < in.blocks().size_of(); i++) {
			/* fix the 'E' bit in case it is not clear */
			if (i < in.blocks().size_of()-1)
				in.blocks()[i].hdr().e() = false;
			else
				in.blocks()[i].hdr().e() = true;
			in.blocks()[i].hdr().encode(LlcBlockHdr_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
		}
	}
	if (in.blocks().is_bound()) {
		for (i = 0; i < in.blocks().size_of(); i++) {
			if (!in.blocks()[i].is_bound())
				continue;
			ttcn_buffer.put_string(in.blocks()[i].payload());
		}
	}

	ttcn_buffer.get_string(ret_val);
	return ret_val;
}

RlcmacDlDataBlock dec__RlcmacDlDataBlock(const OCTETSTRING& stream)
{
	RlcmacDlDataBlock ret_val;
	TTCN_Buffer ttcn_buffer(stream);
	int num_llc_blocks = 0;

	/* use automatic/generated decoder for header */
	ret_val.mac__hdr().decode(DlMacDataHeader_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	/* optional extension octets, containing LI+M+E of Llc blocks */
	if (ret_val.mac__hdr().hdr__ext().e() == false) {
		/* extension octet follows, i.e. optional Llc length octets */
		while (1) {
			/* decode one more extension octet with LlcBlocHdr inside */
			LlcBlock lb;
			lb.hdr().decode(LlcBlockHdr_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
			ret_val.blocks()[num_llc_blocks++] = lb;

			/* if E == '1'B, we can proceed further */
			if (lb.hdr().e() == true)
				break;
		}
	}

	/* RLC blocks at end */
	if (ret_val.mac__hdr().hdr__ext().e() == true) {
		LlcBlock lb;
		unsigned int length = ttcn_buffer.get_read_len();
		/* LI not present: The Upper Layer PDU that starts with the current RLC data block either
		 * fills the current RLC data block precisely or continues in the following in-sequence RLC
		 * data block */
		lb.payload() = OCTETSTRING(length, ttcn_buffer.get_read_data());
		ttcn_buffer.increase_pos(length);
		ret_val.blocks()[0] = lb;
	} else {
		if (ret_val.blocks().is_bound()) {
			for (int i = 0; i < ret_val.blocks().size_of(); i++) {
				unsigned int length = ret_val.blocks()[i].hdr().length__ind();
				if (length > ttcn_buffer.get_read_len())
					length = ttcn_buffer.get_read_len();
				ret_val.blocks()[i].payload() = OCTETSTRING(length, ttcn_buffer.get_read_data());
				ttcn_buffer.increase_pos(length);
			}
		}
	}

	return ret_val;
}


OCTETSTRING enc__RlcmacUlDataBlock(const RlcmacUlDataBlock& si)
{
	RlcmacUlDataBlock in = si;
	OCTETSTRING ret_val;
	TTCN_Buffer ttcn_buffer;
	int i;

	/* Fix 'e' bit of initial header based on following blocks */
	if (!in.blocks().is_bound() ||
	    (in.blocks().size_of() == 1 && !in.blocks()[0].hdr().is_bound()))
		in.mac__hdr().e() = true;
	else
		in.mac__hdr().e() = false;

	/* Fix other presence indications */
	in.mac__hdr().tlli__ind() = in.tlli().is_bound() && in.tlli() != OMIT_VALUE;
	in.mac__hdr().pfi__ind() = in.pfi().is_bound() && in.pfi() != OMIT_VALUE;

	/* use automatic/generated decoder for header */
	in.mac__hdr().encode(UlMacDataHeader_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	/* Add LI octets, if any */
	if (in.blocks().is_bound() &&
	    (in.blocks().size_of() != 1 || in.blocks()[0].hdr().is_bound())) {
		/* first write LI octets */
		for (i = 0; i < in.blocks().size_of(); i++) {
			/* fix the 'E' bit in case it is not clear */
			if (i < in.blocks().size_of()-1)
				in.blocks()[i].hdr().e() = false;
			else
				in.blocks()[i].hdr().e() = true;
			in.blocks()[i].hdr().encode(LlcBlockHdr_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
		}
	}

	if (in.mac__hdr().tlli__ind()) {
		ttcn_buffer.put_string(in.tlli());
	}

	if (in.mac__hdr().pfi__ind()) {
		in.pfi().encode(RlcmacUlDataBlock_pfi_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
	}

	if (in.blocks().is_bound()) {
		for (i = 0; i < in.blocks().size_of(); i++) {
			if (!in.blocks()[i].is_bound())
				continue;
			ttcn_buffer.put_string(in.blocks()[i].payload());
		}
	}

	ttcn_buffer.get_string(ret_val);
	return ret_val;
}

RlcmacUlDataBlock dec__RlcmacUlDataBlock(const OCTETSTRING& stream)
{
	RlcmacUlDataBlock ret_val;
	TTCN_Buffer ttcn_buffer(stream);
	int num_llc_blocks = 0;

	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("==================================\n"
				"dec_RlcmacUlDataBlock(): Stream before decoding: ");
	stream.log();
	TTCN_Logger::end_event();

	/* use automatic/generated decoder for header */
	ret_val.mac__hdr().decode(UlMacDataHeader_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): Stream after decoding hdr: ");
	ttcn_buffer.log();
	TTCN_Logger::end_event();
	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): ret_val after decoding hdr: ");
	ret_val.log();
	TTCN_Logger::end_event();

	/* Manually decoder remainder of ttcn_buffer, containing optional header octets, 
	 * optional tlli, optional pfi and LLC Blocks */

	/* optional extension octets, containing LI+M+E of Llc blocks */
	if (ret_val.mac__hdr().e() == false) {
		/* extension octet follows, i.e. optional Llc length octets */
		while (1) {
			/* decode one more extension octet with LlcBlocHdr inside */
			LlcBlock lb;
			lb.hdr().decode(LlcBlockHdr_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
			ret_val.blocks()[num_llc_blocks++] = lb;

			TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
			TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): Stream after decoding ExtOct: ");
			ttcn_buffer.log();
			TTCN_Logger::end_event();
			TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
			TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): ret_val after decoding ExtOct: ");
			ret_val.log();
			TTCN_Logger::end_event();

			/* if E == '1'B, we can proceed further */
			if (lb.hdr().e() == true)
				break;
		}
	}

	/* parse optional TLLI */
	if (ret_val.mac__hdr().tlli__ind()) {
		ret_val.tlli() = OCTETSTRING(4, ttcn_buffer.get_read_data());
		ttcn_buffer.increase_pos(4);
	}
	/* parse optional PFI */
	if (ret_val.mac__hdr().pfi__ind()) {
		ret_val.pfi().decode(RlcmacUlDataBlock_pfi_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
	}

	/* RLC blocks at end */
	if (ret_val.mac__hdr().e() == true) {
		LlcBlock lb;
		unsigned int length = ttcn_buffer.get_read_len();
		/* LI not present: The Upper Layer PDU that starts with the current RLC data block either
		 * fills the current RLC data block precisely or continues in the following in-sequence RLC
		 * data block */
		lb.payload() = OCTETSTRING(length, ttcn_buffer.get_read_data());
		ttcn_buffer.increase_pos(length);
		ret_val.blocks()[0] = lb;
	} else {
		if (ret_val.blocks().is_bound()) {
			for (int i = 0; i < ret_val.blocks().size_of(); i++) {
				unsigned int length = ret_val.blocks()[i].hdr().length__ind();
				if (length > ttcn_buffer.get_read_len())
					length = ttcn_buffer.get_read_len();
				ret_val.blocks()[i].payload() = OCTETSTRING(length, ttcn_buffer.get_read_data());
				ttcn_buffer.increase_pos(length);
			}
		}
	}

	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): Stream before return: ");
	ttcn_buffer.log();
	TTCN_Logger::end_event();
	TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
	TTCN_Logger::log_event_str("dec_RlcmacUlDataBlock(): ret_val before return: ");
	ret_val.log();
	TTCN_Logger::end_event();

	return ret_val;
}

OCTETSTRING enc__RlcmacUlBlock(const RlcmacUlBlock& si)
{
	if (si.ischosen(RlcmacUlBlock::ALT_data))
		return enc__RlcmacUlDataBlock(si.data());
	else
		return enc__RlcmacUlCtrlBlock(si.ctrl());
}

OCTETSTRING enc__RlcmacDlBlock(const RlcmacDlBlock& si)
{
	if (si.ischosen(RlcmacDlBlock::ALT_data))
		return enc__RlcmacDlDataBlock(si.data());
	else
		return enc__RlcmacDlCtrlBlock(si.ctrl());
}


RlcmacUlBlock dec__RlcmacUlBlock(const OCTETSTRING& stream)
{
	RlcmacUlBlock ret_val;
	unsigned char pt = stream[0].get_octet() >> 6;

	if (pt == MacPayloadType::MAC__PT__RLC__DATA)
		ret_val.data() = dec__RlcmacUlDataBlock(stream);
	else
		ret_val.ctrl() = dec__RlcmacUlCtrlBlock(stream);

	return ret_val;
}

RlcmacDlBlock dec__RlcmacDlBlock(const OCTETSTRING& stream)
{
	RlcmacDlBlock ret_val;
	unsigned char pt = stream[0].get_octet() >> 6;

	if (pt == MacPayloadType::MAC__PT__RLC__DATA)
		ret_val.data() = dec__RlcmacDlDataBlock(stream);
	else
		ret_val.ctrl() = dec__RlcmacDlCtrlBlock(stream);

	return ret_val;
}


} // namespace
