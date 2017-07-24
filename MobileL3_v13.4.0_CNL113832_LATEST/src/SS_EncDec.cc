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
//  File:               SS_PDU_Defs.asn
//  Rev:                R1B
//  Prodnr:             CNL 113 832
//  Contact:            http://ttcn.ericsson.se
//  Reference:          3GPP TS 24.008 v13.4.0

#include "SS_PDU_Defs.hh"
#include "SS_Types.hh"

namespace SS__Types
{
using namespace SS__PDU__Defs;
TTCN_Module SS__EncDec("SS_EncDec", __DATE__, __TIME__);

//Type definitions to transfer data between coding functions
struct SS_Invoke_for_dec
{
  INTEGER               invokeId;
  OPTIONAL<INTEGER>     linkedId;
  Remote__Operations__Information__Objects::Code opcode;
  OPTIONAL<OCTETSTRING> stream;
};

struct SS_ReturnResult_for_dec
{
  INTEGER               invokeId;
  OPTIONAL<Remote__Operations__Information__Objects::Code> opcode;
  OPTIONAL<OCTETSTRING> stream;
};

struct SS_ReturnError_for_dec
{
  INTEGER               invokeId;
  Remote__Operations__Information__Objects::Code errorCode;
  OPTIONAL<OCTETSTRING> stream;
};

struct SS_Reject_for_dec
{
  OPTIONAL<INTEGER>     invokeId;
  Remote__Operations__Generic__ROS__PDUs::ProblemType problem;
};


//////////////////////////////////
// Encoding function for SS__Invoke
//////////////////////////////////
OCTETSTRING enc_SS_Invoke(const SS__Invoke& pdu)
{
  TTCN_Buffer buf;

  OCTETSTRING ret_val;
  pdu.encode(SS__Invoke_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
  buf.get_string(ret_val);
  ret_val[0] = int2oct(161,1);

  return ret_val;
}
//////////////////////////////////
// Encoding function for SS__ReturnResult
//////////////////////////////////
OCTETSTRING enc_SS_ReturnResult(const SS__ReturnResult& pdu)
{
  TTCN_Buffer buf;

  OCTETSTRING ret_val;
  pdu.encode(SS__ReturnResult_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
  buf.get_string(ret_val);
  ret_val[0] = int2oct(162,1);

  return ret_val;
}
//////////////////////////////////
// Encoding function for SS__ReturnError
//////////////////////////////////
OCTETSTRING enc_SS_ReturnError(const SS__ReturnError& pdu)
{
  TTCN_Buffer buf;

  OCTETSTRING ret_val;
  pdu.encode(SS__ReturnError_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
  buf.get_string(ret_val);
  ret_val[0] = int2oct(163,1);

  return ret_val;
}
//////////////////////////////////
// Encoding function for SS__Reject
//////////////////////////////////
OCTETSTRING enc_SS_Reject(const SS__Reject& pdu)
{
  TTCN_Buffer buf;

  OCTETSTRING ret_val;
  pdu.encode(SS__Reject_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);
  buf.get_string(ret_val);
  ret_val[0] = int2oct(164,1);

  return ret_val;
}

//////////////////////////////////
// Decoding function for SS__Invoke
//////////////////////////////////
int dec_SS_Invoke(const SS_Invoke_for_dec& params, SS__Invoke& pdu)
{  
  SS__Invoke__help help_pdu;
  
  if(params.stream.ispresent())
  {
    help_pdu.invokeId().present__() = params.invokeId;
    if(params.linkedId.ispresent())
      help_pdu.linkedId()().present__().present__() = params.linkedId;
    else
      help_pdu.linkedId() = OMIT_VALUE;

    help_pdu.opcode() = params.opcode;

    OCTETSTRING oct_stream = params.stream;
    help_pdu.argument() = oct_stream;
    TTCN_Buffer buf;
    help_pdu.encode(SS__Invoke__help_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);  
    
	  TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_IGNORE);
	  TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_WARNING);
	  TTCN_EncDec::clear_error();
    
    pdu.decode(SS__Invoke_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
    
    TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_DEFAULT);
    
    if(TTCN_EncDec::get_last_error_type() != 0) return 1;
  }
  else
  {
    pdu.invokeId().present__() = params.invokeId;

    if(params.linkedId.ispresent())
      pdu.linkedId()().present__().present__() = params.linkedId;
    else
      pdu.linkedId() = OMIT_VALUE;

    pdu.opcode() = params.opcode;

    pdu.argument() = OMIT_VALUE;
  }

  return 0;
}

//////////////////////////////////
// Decoding function for SS__ReturnResult
//////////////////////////////////
int dec_SS_ReturnResult(const SS_ReturnResult_for_dec& params, SS__ReturnResult& pdu)
{  
  SS__ReturnResult__help help_pdu;


  if(params.opcode.ispresent() && params.stream.ispresent())
  {
    help_pdu.invokeId().present__() = params.invokeId;
    help_pdu.result()().opcode() = params.opcode;
    OCTETSTRING oct_stream = params.stream;
    help_pdu.result()().result() = oct_stream;
    TTCN_Buffer buf;
    help_pdu.encode(SS__ReturnResult__help_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);  
    
	  TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_IGNORE);
	  TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_WARNING);
	  TTCN_EncDec::clear_error();
	  
	  pdu.decode(SS__ReturnResult_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);

    TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_DEFAULT);
    
    if(TTCN_EncDec::get_last_error_type() != 0) return 1;
  }
  else
  {
    pdu.invokeId().present__() = params.invokeId;
    pdu.result() = OMIT_VALUE;
  }

  return 0;
}

//////////////////////////////////
// Decoding function for SS__ReturnError
//////////////////////////////////
int dec_SS_ReturnError(const SS_ReturnError_for_dec& params, SS__ReturnError& pdu)
{
  
  SS__ReturnError__help help_pdu;

  if(params.stream.ispresent())
  {
    help_pdu.invokeId().present__() = params.invokeId;
    help_pdu.errcode() = params.errorCode;
    OCTETSTRING oct_stream = params.stream;
    help_pdu.parameter()() = oct_stream;

    TTCN_Buffer buf;

    help_pdu.encode(SS__ReturnError__help_descr_, buf, TTCN_EncDec::CT_BER, BER_ENCODE_DER);  

	  TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_IGNORE);
	  TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_WARNING);
	  TTCN_EncDec::clear_error();
	  
    pdu.decode(SS__ReturnError_descr_, buf, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);

    TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_DEFAULT);
    
    if(TTCN_EncDec::get_last_error_type() != 0) return 1;
  }
  else
  {
    pdu.invokeId().present__() = params.invokeId;
    pdu.errcode() = params.errorCode;
    pdu.parameter() = OMIT_VALUE;
  }

  return 0;
}

//////////////////////////////////
// Decoding function for SS__Reject
//////////////////////////////////
int dec_SS_Reject(const SS_Reject_for_dec& params, SS__Reject& pdu)
{
  if(params.invokeId.ispresent())
    pdu.invokeId().present__() = params.invokeId;
  else
    pdu.invokeId().absent() = ASN_NULL_VALUE;

  pdu.problem() = params.problem;
  
  return 0;
}

////////////////////////////////////////////////////////////////////
// Decoding function for internal SS_TCAP_ComponentPortion
////////////////////////////////////////////////////////////////////
int dec__SS__TCAP__ComponentPortion(const OCTETSTRING& stream, SS__TCAP__ComponentPortion& pdu)
{
  if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC))
  {
    TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
    TTCN_Logger::log_event_str("dec_SS_TCAP_ComponentPortion(): Stream before decoding: ");
    stream.log();
    TTCN_Logger::end_event();
  }

	TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_IGNORE);
	TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_WARNING);
	TTCN_EncDec::clear_error();

  TTCN_Buffer ttcn_buffer(stream);
  SS__TCAP__Component    ret_sub_val;
  int i = 0;

  if(!ttcn_buffer.contains_complete_TLV()) {
    if (TTCN_Logger::log_this_event(TTCN_Logger::TTCN_WARNING))
    {
      TTCN_Logger::begin_event(TTCN_Logger::TTCN_WARNING);
      TTCN_Logger::log_event_str("dec_SS_TCAP_ComponentPortion(): there is no complete TLV in the incoming stream!");
      TTCN_Logger::end_event();
    }
    return 1;
  }

  while(ttcn_buffer.contains_complete_TLV())
  {
    ret_sub_val.decode(SS__TCAP__Component_descr_, ttcn_buffer, TTCN_EncDec::CT_BER, BER_ACCEPT_ALL);
    pdu[i] = ret_sub_val;
    i++;
    ttcn_buffer.cut();
  }

	TTCN_EncDec::set_error_behavior(TTCN_EncDec::ET_ALL, TTCN_EncDec::EB_DEFAULT);

  if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC))
  {
    TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
    TTCN_Logger::log_event_str("dec_SS_TCAP_ComponentPortion(): Decoded @SS_Types.SS_TCAP_ComponentPortion: ");
    pdu.log();
    TTCN_Logger::end_event();
  }

	return TTCN_EncDec::get_last_error_type() == TTCN_EncDec::ET_NONE ? 0 : 1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Main encoding function for SS_FacilityInformation. This is called from TTCN-3
//////////////////////////////////////////////////////////////////////////////////////////////////////
OCTETSTRING enc__SS__FacilityInformation(const SS__FacilityInformation& pdu)
{
  if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC))
  {
    TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
    TTCN_Logger::log_event_str("enc_SS_FacilityInformation(): SS_FacilityInformation before encoding: ");
    pdu.log();
    TTCN_Logger::end_event();
  }
  SS__TCAP__ComponentPortion temp_val;
  OCTETSTRING ret_val(0,NULL);

  for (int i = 0; i < pdu.size_of(); i++)
  {
    if (pdu[i].get_selection() == SS__Component::ALT_invoke)
    {
       ret_val = ret_val + enc_SS_Invoke(pdu[i].invoke());
    }
    else if (pdu[i].get_selection() == SS__Component::ALT_returnResult)
    {
       ret_val = ret_val + enc_SS_ReturnResult(pdu[i].returnResult());
    }
    else if (pdu[i].get_selection() == SS__Component::ALT_returnError)
    {
       ret_val = ret_val + enc_SS_ReturnError(pdu[i].returnError());
    }
    else if (pdu[i].get_selection() == SS__Component::ALT_reject)
    {
       ret_val = ret_val + enc_SS_Reject(pdu[i].reject());
    }
  }
  if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC))
  {
    TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
    TTCN_Logger::log_event_str("enc_SS_FacilityInformation(): stream after encoding: ");
    ret_val.log();
    TTCN_Logger::end_event();
  }
  return ret_val;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Main decoding function for SS_FacilityInformation. This is called from TTCN-3
//////////////////////////////////////////////////////////////////////////////////////////////////////
INTEGER dec__SS__FacilityInformation__backtrack(const OCTETSTRING& stream, SS__FacilityInformation& pdu)
{
  if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC))
  {
    TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
    TTCN_Logger::log_event_str("dec_SS_FacilityInformation__backtrack(): Stream before decoding: ");
    stream.log();
    TTCN_Logger::end_event();
  }
  
  SS__TCAP__ComponentPortion temp_val;
  int decode = dec__SS__TCAP__ComponentPortion(stream, temp_val);
  
  if(decode == 0) {

    for (int i = 0; i < temp_val.size_of() && decode == 0; i++)
    {
      if (temp_val[i].get_selection() == SS__TCAP__Component::ALT_invoke)
      {
        SS_Invoke_for_dec par_val;
        par_val.invokeId = temp_val[i].invoke().invokeID();
        par_val.linkedId = temp_val[i].invoke().linkedID();
        par_val.opcode   =   temp_val[i].invoke().operationCode();
        par_val.stream   =   temp_val[i].invoke().parameter();
        decode = dec_SS_Invoke(par_val, pdu[i].invoke());
      }
      else if (temp_val[i].get_selection() == SS__TCAP__Component::ALT_returnResult)
      {
        SS_ReturnResult_for_dec par_val;
        par_val.invokeId = temp_val[i].returnResult().invokeID();

        if(temp_val[i].returnResult().result().ispresent())
        {
          SS__TCAP__ReturnResult__Result temp_res = temp_val[i].returnResult().result();
          par_val.opcode = temp_res.operationCode();
          par_val.stream = temp_res.parameter();
        }
        else
        {
          par_val.opcode = OMIT_VALUE;
          par_val.stream = OMIT_VALUE;
        }
        decode = dec_SS_ReturnResult(par_val, pdu[i].returnResult());
      }
      else if (temp_val[i].get_selection() == SS__TCAP__Component::ALT_returnError)
      {
        SS_ReturnError_for_dec par_val;
        par_val.invokeId = temp_val[i].returnError().invokeID();
        par_val.errorCode = temp_val[i].returnError().errorCode();
        par_val.stream  =   temp_val[i].returnError().parameter();

        decode = dec_SS_ReturnError(par_val, pdu[i].returnError());
      }
      else if (temp_val[i].get_selection() == SS__TCAP__Component::ALT_reject)
      {
        SS_Reject_for_dec par_val;
        if( temp_val[i].reject().invokeID().get_selection() == SS__TCAP__Reject_invokeID::ALT_derivable)
          par_val.invokeId = temp_val[i].reject().invokeID().derivable();
        else
          par_val.invokeId = OMIT_VALUE;
        par_val.problem = temp_val[i].reject().problem();

        decode = dec_SS_Reject(par_val, pdu[i].reject());
      }
    }
  }

  if (TTCN_Logger::log_this_event(TTCN_Logger::DEBUG_ENCDEC))
  {
    TTCN_Logger::begin_event(TTCN_Logger::DEBUG_ENCDEC);
    TTCN_Logger::log_event_str("dec_SS_FacilityInformation__backtrack(): Decoded @SS_Types.SS_FacilityInformation: ");
    pdu.log();
    TTCN_Logger::end_event();
  }

  return decode;
}

SS__FacilityInformation dec__SS__FacilityInformation(const OCTETSTRING& stream)
{
  SS__FacilityInformation ret;
  dec__SS__FacilityInformation__backtrack(stream, ret);
  return ret;
}

}//namespace

