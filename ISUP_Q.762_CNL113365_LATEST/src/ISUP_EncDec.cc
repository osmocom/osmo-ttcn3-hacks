///////////////////////////////////////////////////////////////////////////////
//
// Copyright Test Competence Center (TCC) ETH 2011
//                                                                           //
// The copyright to the computer  program(s) herein  is the property of TCC. //
// The program(s) may be used and/or copied only with the written permission //
// of TCC or in accordance with  the terms and conditions  stipulated in the //
// agreement/contract under which the program(s) have been supplied          //
///////////////////////////////////////////////////////////////////////////////
//
//  File:		ISUP_EncDec.cc
//  Rev:                R7D
//  Prodnr:             CNL 113 365
//  Updated:            2011-05-10
//  Contact:            http://ttcn.ericsson.se
///////////////////////////////////////////////////////////////////////////////
#include "ISUP_Types.hh"

namespace ISUP__Types {

OCTETSTRING enc__PDU__ISUP(const PDU__ISUP& pdu)
{
    if (TTCN_Logger::log_this_event(TTCN_DEBUG)) {
	TTCN_Logger::begin_event(TTCN_DEBUG);
	TTCN_Logger::log_event("Encoding PDU_ISUP: ");
	pdu.log();
	TTCN_Logger::end_event();
    }
    
    TTCN_Buffer buf;
    pdu.encode(PDU__ISUP_descr_, buf, TTCN_EncDec::CT_RAW);
    OCTETSTRING ret_val(buf.get_len(), buf.get_data());
    
    if (TTCN_Logger::log_this_event(TTCN_DEBUG)) {
	TTCN_Logger::begin_event(TTCN_DEBUG);
	TTCN_Logger::log_event("PDU_ISUP after encoding: ");
	ret_val.log();
	TTCN_Logger::end_event();
    }
    return ret_val;
}

PDU__ISUP dec__PDU__ISUP(const OCTETSTRING& stream)
{
    if (TTCN_Logger::log_this_event(TTCN_DEBUG)) {
	TTCN_Logger::begin_event(TTCN_DEBUG);
	TTCN_Logger::log_event("Decoding PDU_ISUP: ");
	stream.log();
	TTCN_Logger::end_event();
    }

    TTCN_Buffer buf;
    buf.put_os(stream);
    PDU__ISUP ret_val;
    ret_val.decode(PDU__ISUP_descr_, buf, TTCN_EncDec::CT_RAW);

    if (TTCN_Logger::log_this_event(TTCN_DEBUG)) {
	TTCN_Logger::begin_event(TTCN_DEBUG);
	TTCN_Logger::log_event("Decoded PDU_ISUP: ");
	ret_val.log();
	TTCN_Logger::end_event();
    }
    return ret_val;
}

PDU__ISUP dec__PDU__ISUP__noCIC(const OCTETSTRING& stream)
{
    if (TTCN_Logger::log_this_event(TTCN_DEBUG)) {
	TTCN_Logger::begin_event(TTCN_DEBUG);
	TTCN_Logger::log_event("Decoding PDU_ISUP: ");
	stream.log();
	TTCN_Logger::end_event();
    }

    OCTETSTRING mod_stream (int2oct(0,2) + stream);
    TTCN_Buffer buf;
    buf.put_os(mod_stream);
    PDU__ISUP ret_val;
    ret_val.decode(PDU__ISUP_descr_, buf, TTCN_EncDec::CT_RAW);
    PDU__ISUP::union_selection_type msg_type = ret_val.get_selection();
    switch (msg_type)
    {
      case PDU__ISUP::ALT_ISUP__ACM:
      {
        ret_val.ISUP__ACM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__ANM:
      {
        ret_val.ISUP__ANM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__APM:
      {
        ret_val.ISUP__APM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__BLO:
      {
        ret_val.ISUP__BLO().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__BLA:
      {
        ret_val.ISUP__BLA().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CPG:
      {
        ret_val.ISUP__CPG().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CGB:
      {
        ret_val.ISUP__CGB().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CGBA:
      {
        ret_val.ISUP__CGBA().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CQM:
      {
        ret_val.ISUP__CQM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CQR:
      {
        ret_val.ISUP__CQR().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__GRS:
      {
        ret_val.ISUP__GRS().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__GRA:
      {
        ret_val.ISUP__GRA().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CGU:
      {
        ret_val.ISUP__CGU().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CGUA:
      {
        ret_val.ISUP__CGUA().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CRG:
      {
        ret_val.ISUP__CRG().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CFN:
      {
        ret_val.ISUP__CFN().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CON:
      {
        ret_val.ISUP__CON().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__COT:
      {
        ret_val.ISUP__COT().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__CCR:
      {
        ret_val.ISUP__CCR().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__FAC:
      {
        ret_val.ISUP__FAC().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__FAA:
      {
        ret_val.ISUP__FAA().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__FRJ:
      {
        ret_val.ISUP__FRJ().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__FAR:
      {
        ret_val.ISUP__FAR().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__FOT:
      {
        ret_val.ISUP__FOT().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__IDR:
      {
        ret_val.ISUP__IDR().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__IRS:
      {
        ret_val.ISUP__IRS().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__INF:
      {
        ret_val.ISUP__INF().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__INR:
      {
        ret_val.ISUP__INR().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__IAM:
      {
        ret_val.ISUP__IAM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__LPA:
      {
        ret_val.ISUP__LPA().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__LOP:
      {
        ret_val.ISUP__LOP().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__NRM:
      {
        ret_val.ISUP__NRM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__OLM:
      {
        ret_val.ISUP__OLM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__OPR:
      {
        ret_val.ISUP__OPR().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__PAM:
      {
        ret_val.ISUP__PAM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__PRI:
      {
        ret_val.ISUP__PRI().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__REL:
      {
        ret_val.ISUP__REL().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__RLC:
      {
        ret_val.ISUP__RLC().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__RSC:
      {
        ret_val.ISUP__RSC().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__RES:
      {
        ret_val.ISUP__RES().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__SGM:
      {
        ret_val.ISUP__SGM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__SAM:
      {
        ret_val.ISUP__SAM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__SCB:
      {
        ret_val.ISUP__SCB().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__SDM:
      {
        ret_val.ISUP__SDM().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__SUS:
      {
        ret_val.ISUP__SUS().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__UBL:
      {
        ret_val.ISUP__UBL().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__UBA:
      {
        ret_val.ISUP__UBA().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__UCIC:
      {
        ret_val.ISUP__UCIC().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__UPA:
      {
        ret_val.ISUP__UPA().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__UPT:
      {
        ret_val.ISUP__UPT().cic() = OMIT_VALUE; break;
      }
      case PDU__ISUP::ALT_ISUP__USR:
      {
        ret_val.ISUP__USR().cic() = OMIT_VALUE; break;
      }
      default:
        break;
    }
    if (TTCN_Logger::log_this_event(TTCN_DEBUG)) {
	TTCN_Logger::begin_event(TTCN_DEBUG);
	TTCN_Logger::log_event("Decoded PDU_ISUP: ");
	ret_val.log();
	TTCN_Logger::end_event();
    }
    return ret_val;
}

INTEGER dec__PDU__ISUP__backtrack__noCIC(const OCTETSTRING& stream, PDU__ISUP& pdu)
{
	if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC)) {
		TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
		TTCN_Logger::log_event_str("dec_PDU_ISUP_backtrack(): Stream before decoding: ");
		stream.log();
		TTCN_Logger::end_event();
	}
	TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_WARNING);
	TTCN_EncDec::clear_error();
	OCTETSTRING mod_stream (int2oct(0,2) + stream);
    TTCN_Buffer ttcn_buffer;
    ttcn_buffer.put_os(mod_stream);
    pdu.decode(PDU__ISUP_descr_, ttcn_buffer, TTCN_EncDec::CT_RAW);
    	
	
	if (TTCN_EncDec::get_last_error_type() == TTCN_EncDec::ET_NONE) {
		
		PDU__ISUP::union_selection_type msg_type = pdu.get_selection();
	    switch (msg_type)
	    {
	      case PDU__ISUP::ALT_ISUP__ACM:
	      {
	    	  pdu.ISUP__ACM().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__ANM:
	      {
	    	  pdu.ISUP__ANM().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__BLO:
	      {
	    	  pdu.ISUP__BLO().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__BLA:
	      {
	    	  pdu.ISUP__BLA().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CPG:
	      {
	    	  pdu.ISUP__CPG().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CGB:
	      {
	    	  pdu.ISUP__CGB().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CGBA:
	      {
	    	  pdu.ISUP__CGBA().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__GRS:
	      {
	    	  pdu.ISUP__GRS().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__GRA:
	      {
	    	  pdu.ISUP__GRA().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CGU:
	      {
	    	  pdu.ISUP__CGU().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CGUA:
	      {
	    	  pdu.ISUP__CGUA().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CQM:
	      {
	    	  pdu.ISUP__CQM().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CQR:
	      {
	    	  pdu.ISUP__CQR().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CFN:
	      {
	    	  pdu.ISUP__CFN().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__COT:
	      {
	    	  pdu.ISUP__COT().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__CCR:
	      {
	    	  pdu.ISUP__CCR().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__FAC:
	      {
	    	  pdu.ISUP__FAC().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__FOT:
	      {
	    	  pdu.ISUP__FOT().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__INF:
	      {
	    	  pdu.ISUP__INF().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__INR:
	      {
	    	  pdu.ISUP__INR().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__IAM:
	      {
	    	  pdu.ISUP__IAM().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__LPA:
	      {
	    	  pdu.ISUP__LPA().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__REL:
	      {
	    	  pdu.ISUP__REL().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__RLC:
	      {
	    	  pdu.ISUP__RLC().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__RSC:
	      {
	    	  pdu.ISUP__RSC().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__RES:
	      {
	    	  pdu.ISUP__RES().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__SUS:
	      {
	    	  pdu.ISUP__SUS().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__UBL:
	      {
	    	  pdu.ISUP__UBL().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__UBA:
	      {
	    	  pdu.ISUP__UBA().cic() = OMIT_VALUE; break;
	      }
	      case PDU__ISUP::ALT_ISUP__UCIC:
	      {
	    	  pdu.ISUP__UCIC().cic() = OMIT_VALUE; break;
	      }
	      default:
	        break;
	    }
		
		
		if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC)) {
			TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
			TTCN_Logger::log_event_str("dec_PDU_ISUP_backtrack_noCIC(): Decoded @ISUP_Types.PDU_ISUP: ");
			pdu.log();
			TTCN_Logger::end_event();
		}
		
		if (ttcn_buffer.get_pos() < ttcn_buffer.get_len() && TTCN_Logger::log_this_event(TTCN_WARNING)) {
			ttcn_buffer.cut();
			OCTETSTRING remaining_stream;
			ttcn_buffer.get_string(remaining_stream);
			TTCN_Logger::begin_event(TTCN_WARNING);
			TTCN_Logger::log_event_str("dec_PDU_ISUP_backtrack(): Warning: Data remained at the end of the stream after successful decoding: ");
			remaining_stream.log();
			TTCN_Logger::end_event();
		}
		return 0;
	} else return 1;
}

}//namespace
