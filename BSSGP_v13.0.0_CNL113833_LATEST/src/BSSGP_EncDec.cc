///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright Test Competence Center (TCC) ETH 2016                           //
//                                                                           //
// The copyright to the computer  program(s) herein  is the property of TCC. //
// The program(s) may be used and/or copied only with the written permission //
// of TCC or in accordance with  the terms and conditions  stipulated in the //
// agreement/contract under which the program(s) has been supplied.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
//  File:               BSSGP_EncDec.cc
//  Rev:                R1A
//  Prodnr:             CNL 113 833
//  Contact:            http://ttcn.ericsson.se
//  Reference:          3GPP TS 48.018 13.0.0

#include "BSSGP_Types.hh"

//static TTCN_Module BSSGP_EndDec("BSSGP_EncDec", __DATE__, __TIME__);

namespace BSSGP__Types {


OCTETSTRING enc__PDU__BSSGP(const PDU__BSSGP& pdu)
{
 TTCN_Buffer bb;

 // CALCULATE WHICH LENGTH FORMAT SHOULD BE USED AND CHANGE SHORT LENGTH FORM
 // TO LONG LENGTH FORM IF NECESSARY
 if (pdu.get_selection() == PDU__BSSGP::ALT_pDU__BSSGP__DL__UNITDATA)
 {
  if( pdu.pDU__BSSGP__DL__UNITDATA().lLC__PDU().lLC__PDU().lengthof() > 127)
    {
      PDU__BSSGP pdu2(pdu);
      pdu2.pDU__BSSGP__DL__UNITDATA().lLC__PDU().lengthIndicator().length2() =
        pdu2.pDU__BSSGP__DL__UNITDATA().lLC__PDU().lLC__PDU().lengthof();
      pdu2.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);

    }
    else
      pdu.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);
  }
  else if(pdu.get_selection() == PDU__BSSGP::ALT_pDU__BSSGP__UL__UNITDATA)
  {
   if( pdu.pDU__BSSGP__UL__UNITDATA().lLC__PDU().lLC__PDU().lengthof() > 127)
    {
      PDU__BSSGP pdu2(pdu);
      pdu2.pDU__BSSGP__UL__UNITDATA().lLC__PDU().lengthIndicator().length2() =
        pdu2.pDU__BSSGP__UL__UNITDATA().lLC__PDU().lLC__PDU().lengthof();
      pdu2.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);
    }
    else
      pdu.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);
  }

  else if(pdu.get_selection() == PDU__BSSGP::ALT_pDU__BSSGP__DL__MBMS__UNITDATA)
   {
    if( pdu.pDU__BSSGP__DL__MBMS__UNITDATA().lLC__PDU().lLC__PDU().lengthof() > 127)
     {
       PDU__BSSGP pdu2(pdu);
       pdu2.pDU__BSSGP__DL__MBMS__UNITDATA().lLC__PDU().lengthIndicator().length2() =
         pdu2.pDU__BSSGP__DL__MBMS__UNITDATA().lLC__PDU().lLC__PDU().lengthof();
       pdu2.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);
     }
     else
       pdu.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);
   }

  else if(pdu.get_selection() == PDU__BSSGP::ALT_pDU__BSSGP__UL__MBMS__UNITDATA)
   {
    if( pdu.pDU__BSSGP__UL__MBMS__UNITDATA().lLC__PDU().lLC__PDU().lengthof() > 127)
     {
       PDU__BSSGP pdu2(pdu);
       pdu2.pDU__BSSGP__UL__MBMS__UNITDATA().lLC__PDU().lengthIndicator().length2() =
         pdu2.pDU__BSSGP__UL__MBMS__UNITDATA().lLC__PDU().lLC__PDU().lengthof();
       pdu2.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);
     }
     else
       pdu.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);
   }

  else
    pdu.encode(PDU__BSSGP_descr_ ,bb, TTCN_EncDec::CT_RAW);

  return OCTETSTRING (bb.get_len(), bb.get_data());
}

/*PDU__BSSGP dec__PDU__BSSGP(const OCTETSTRING& stream)
{
        PDU__BSSGP retv;
        TTCN_Buffer bb;

        bb.put_os(stream);

        retv.decode(PDU__BSSGP_descr_, bb, TTCN_EncDec::CT_RAW);
        return retv;
}*/

PDU__BSSGP dec__PDU__BSSGP(const OCTETSTRING& stream)
{
	if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC)) {
		TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
		TTCN_Logger::log_event_str("dec_PDU_BSSGP(): Stream before decoding: ");
		stream.log();
		TTCN_Logger::end_event();
	}
	TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_DEFAULT);
	TTCN_EncDec::clear_error();
	TTCN_Buffer ttcn_buffer(stream);
	PDU__BSSGP ret_val;
	ret_val.decode(PDU__BSSGP_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);

	if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC)) {
		TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
		TTCN_Logger::log_event_str("dec_PDU_BSSGP(): Decoded @BSSGP_Types.PDU_BSSGP: ");
		ret_val.log();
		TTCN_Logger::end_event();
	}
if (TTCN_EncDec::get_last_error_type() == TTCN_EncDec::ET_NONE) {
	if (ttcn_buffer.get_pos() < ttcn_buffer.get_len() && TTCN_Logger::log_this_event(TTCN_WARNING)) {
		ttcn_buffer.cut();
		OCTETSTRING remaining_stream;
		ttcn_buffer.get_string(remaining_stream);
		TTCN_Logger::begin_event(TTCN_WARNING);
		TTCN_Logger::log_event_str("dec_PDU_BSSGP(): Warning: Data remained at the end of the stream after successful decoding: ");
		remaining_stream.log();
		TTCN_Logger::end_event();
	}
	}
if(ret_val.get_selection()== PDU__BSSGP::ALT_pDU__BSSGP__PS__HANDOVER__REQUEST)
{
	if(!ret_val.pDU__BSSGP__PS__HANDOVER__REQUEST().target__Cell__Identifier().ispresent()){
		Cell__Identifier cell;
		cell = ret_val.pDU__BSSGP__PS__HANDOVER__REQUEST().source__Cell__Identifier();
		ret_val.pDU__BSSGP__PS__HANDOVER__REQUEST().target__Cell__Identifier() = cell;
		//ret_val.pDU__BSSGP__PS__HANDOVER__REQUEST().source__Cell__Identifier().clean_up();
		ret_val.pDU__BSSGP__PS__HANDOVER__REQUEST().source__Cell__Identifier()= OMIT_VALUE;

	} }
return ret_val;
}


}//namespace
