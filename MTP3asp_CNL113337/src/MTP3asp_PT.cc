///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright Test Competence Center (TCC) ETH 2012                           //
//                                                                           //
// The copyright to the computer  program(s) herein  is the property of TCC. //
// The program(s) may be used and/or copied only with the written permission //
// of TCC or in accordance with  the terms and conditions  stipulated in the //
// agreement/contract under which the program(s) have been supplied          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
//  File:         MTP3asp_PT.cc
//  Description:  Implementation of port MTP3asp_PT
//                This test port is written to connect TTCN-3 to SEA according
//                to specification ITU-T SS7 MTP3, ANSI, TCC, IETF, MPT
//  Reference:    ITU-T Recommendation Q.704, RFC3332
//  Rev:          R11A01
//  Prodnr:       CNL 113 337
//  Updated:      2012-05-24
//  Contact:      http://ttcn.ericsson.se

// IMPORTANT MODIFICATION:
// mtp3_ttc uses ALWAYS 16 bits long SPCs, regardless of mtp3_ni sio

#include "MTP3asp_PT.hh"

#include "MTP3asp_Types.hh"
#include "MTP3asp_PortType.hh"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>


//  Constans for M3UA, see rfc 3332 and 2/1056-FCPW 101 86/P-1
// Constants for decoding M3UA common headers
#define M3UA_VER_OFFS      0
#define M3UA_CLS_OFFS      2
#define M3UA_TYP_OFFS      3
#define M3UA_LGT_OFFS      4
#define M3UA_MSG_OFFS      8
#define M3UA_MSG_LENGTH_LENGTH      4

// Constants for M3UA protocol
//Msg classes
#define M3UA_MSG_CLS_MGMT 0x00
#define M3UA_MSG_CLS_TRNSFM 0x01
#define M3UA_MSG_CLS_SSNM 0x02
#define M3UA_MSG_CLS_ASPSM 0x03
#define M3UA_MSG_CLS_ASPTM 0x04
#define M3UA_MSG_CLS_RKM 0x09
//Msg types
#define M3UA_MSG_TYP_MGMT_ERR 0x00
#define M3UA_MSG_TYP_MGMT_NTFY 0x01

#define M3UA_MSG_TYP_TRSNFM_DATA 0x01

#define M3UA_MSG_TYP_SSNM_DUNA 0x01
#define M3UA_MSG_TYP_SSNM_DAVA 0x02
#define M3UA_MSG_TYP_SSNM_DAUD 0x03
#define M3UA_MSG_TYP_SSNM_SCON 0x04
#define M3UA_MSG_TYP_SSNM_DUPU 0x05
#define M3UA_MSG_TYP_SSNM_DRST 0x06

#define M3UA_MSG_TYP_ASPSM_ASPUP 0x01
#define M3UA_MSG_TYP_ASPSM_ASPDN 0x02
#define M3UA_MSG_TYP_ASPSM_BEAT 0x03
#define M3UA_MSG_TYP_ASPSM_ASPUPAck 0x04
#define M3UA_MSG_TYP_ASPSM_ASPDNAck 0x05
#define M3UA_MSG_TYP_ASPSM_BEATAck 0x06

#define M3UA_MSG_TYP_ASPTM_ASPAC 0x01
#define M3UA_MSG_TYP_ASPTM_ASPIA 0x02
#define M3UA_MSG_TYP_ASPTM_ASPACAck 0x03
#define M3UA_MSG_TYP_ASPTM_ASPIAAck 0x04

#define M3UA_MSG_TYP_RKM_REGREQ 0x01
#define M3UA_MSG_TYP_RKM_REGRSP 0x02
#define M3UA_MSG_TYP_RKM_DEREGREQ 0x03
#define M3UA_MSG_TYP_RKM_DEREGRESP 0x04

//parameters
//common for all adaptation layer
#define PAR_PREFIX_COMMON 0x00
#define PAR_INFO_STR    0x04
#define PAR_ROUTING_CTX 0x06
#define PAR_DIAG_INFO   0x07
#define PAR_HEART_BEAT  0x09
#define PAR_TRAFFMODE_T 0x0b
#define PAR_ERROR_CODE  0x0c
#define PAR_STATUS      0x0d
#define PAR_ASP_ID      0x11
#define PAR_AFFECTED_PC 0x12
#define PAR_CORREL_ID   0x13

//M3UA specific
#define PAR_PREFIX_M3UA 0x02
#define PAR_NETW_APP    0x00
#define PAR_USR_O_CAUSE 0x04
#define PAR_CNGST_IND   0x05
#define PAR_CNCRD_IND   0x06
#define PAR_ROUTING_KEY 0x07
#define PAR_REG_RSLT    0x08
#define PAR_DEREG_RSLT  0x09
#define PAR_LOC_RK_ID   0x0a
#define PAR_DPC         0x0b
#define PAR_SI          0x0c
#define PAR_OPC_LIST    0x0e
#define PAR_CIRC_RNG    0x0f
#define PAR_PROT_DATA   0x10
#define PAR_REG_STAT    0x12
#define PAR_DEREG_STAT  0x13

// error codes:
#define PAR_ERRC_IVER   0x01
#define PAR_ERRC_UNSMC  0x03
#define PAR_ERRC_UNSMT  0x04
#define PAR_ERRC_UNTMT  0x05
#define PAR_ERRC_UNEM   0x06
#define PAR_ERRC_PERR   0x07
#define PAR_ERRC_ISI    0x09
#define PAR_ERRC_RMB    0x0D
#define PAR_ERRC_ASPIDR 0x0E
#define PAR_ERRC_IASPID 0x0F
#define PAR_ERRC_IPARV  0x11
#define PAR_ERRC_PARFE  0x12
#define PAR_ERRC_UPAR   0x13
#define PAR_ERRC_DSU    0x14
#define PAR_ERRC_INA    0x15
#define PAR_ERRC_MP     0x16
#define PAR_ERRC_IRC    0x19
#define PAR_ERRC_NCFAS  0x1A


// --------------------------
// Basic Test Port functions
// --------------------------
using namespace MTP3asp__Types;
namespace MTP3asp__PortType {

//external functions
//##########################################################

//f__MTP3__SEA__connect (for MTP3asp_PT)
BOOLEAN f__MTP3__SEA__connect(MTP3asp__PT& portRef,
  const CHARSTRING& Hostname,const INTEGER& Port,const CHARSTRING& EntityName,const BOOLEAN& Http)
{
  return f__MTP3__SEA__connect__extern(portRef,Hostname,Port,EntityName,Http);
}
//------------

//f__MTP3__SEA__disconnect (for MTP3asp_PT)
BOOLEAN f__MTP3__SEA__disconnect(MTP3asp__PT& portRef)
{
  return f__MTP3__SEA__disconnect__extern(portRef);
}
//------------

//f__MTP3__SEA__connect__extern
BOOLEAN f__MTP3__SEA__connect__extern(MTP3asp__PT_PROVIDER& portRef,
  const CHARSTRING& Hostname,const INTEGER& Port,const CHARSTRING& EntityName,const BOOLEAN& Http)
{
#ifndef TARGET_TEST
  if (portRef.dynamicConnection && (!(portRef.connectionUp)))
  {
    delete [] portRef.hostname;
    int len = strlen(Hostname);
    portRef.hostname = new char[len + 1];
    memcpy(portRef.hostname, Hostname, len + 1);

    portRef.httpport = Port;

    delete [] portRef.entityname;
    len = strlen(EntityName);
    portRef.entityname = new char[len + 1];
    memcpy(portRef.entityname, EntityName, len + 1);

    if(Http)
      portRef.MTP3_open_channel(TRUE);
    else
      portRef.MTP3_open_channel(FALSE);

    if(portRef.wait_for_open())
    { 
      portRef.user_connect();
      portRef.connectionUp = TRUE;
      return TRUE;
    }
  }
  else
#endif
    portRef.log("Dynamic connection feature is not active or already connected.");
  return FALSE;
}
//------------

//f__MTP3__SEA__disconnect__extern
BOOLEAN f__MTP3__SEA__disconnect__extern(MTP3asp__PT_PROVIDER& portRef)
{
#ifndef TARGET_TEST
  if (portRef.connectionUp)
  {
    portRef.MTP3_close_connection();
    portRef.connectionUp = FALSE;
    return TRUE;
  }
#endif
  return FALSE;
}
//------------

// Test Port constructor
MTP3asp__PT_PROVIDER::MTP3asp__PT_PROVIDER(const char *par_port_name)
        : PORT(par_port_name)
{
  MTP_fd=-1;
  httpport=-1;
  hostname=NULL;
  destinationname = NULL;
  dynamicConnection = FALSE;

  const char *str="b303d76a-266c-11d4-b8f5-08002090d3da";
  int len = strlen(str);
  iid_string= new char[len + 1];
  memcpy(iid_string, str, len + 1);

  entityname=NULL;
  Filter=Loop=-1;
  Sut_Pc=Tester_Pc=-1;
  Ni_is_set = FALSE;
  MTPServiceType = MTP3itu;
  M3UA_version = 1;
  M3UAState = AssocDown; // unnecessary...
  mtp3_ni=0;
#ifndef TARGET_TEST
  user_map_p = &MTP3asp__PT_PROVIDER::MTP3_user_map;
  user_unmap_p = &MTP3asp__PT_PROVIDER::MTP3_user_unmap;
  interpreter = &MTP3asp__PT_PROVIDER::MTP3_interpreter;
#endif
  forward_pause = FALSE;
  forward_resume = FALSE;
  forward_status = FALSE;
}
//------------

// Test Port destructor
MTP3asp__PT_PROVIDER::~MTP3asp__PT_PROVIDER()
{
  delete [] hostname;
  delete [] entityname;
  delete [] iid_string;
}
//------------

// set_parameter
void MTP3asp__PT_PROVIDER::set_parameter(const char *parameter_name,
        const char *parameter_value)
{
  log("set_parameter: %s = %s",parameter_name,parameter_value);
  if (!strcmp(parameter_name,"Hostname"))
  {
    delete [] hostname;
    int len = strlen(parameter_value);
    hostname = new char[len + 1];
    memcpy(hostname, parameter_value, len + 1);
  }
  else if (!strcmp(parameter_name, "HttpPort"))
  {
    httpport = atoi(parameter_value);
  }
  else if (!strcmp(parameter_name,"EntityName"))
  {
    delete [] entityname;
    int len = strlen(parameter_value);
    entityname= new char[len + 1];
    memcpy(entityname, parameter_value, len + 1);
  }
  else if (!strcmp(parameter_name,"DestinationName"))
  {
    delete [] destinationname;
    int len = strlen(parameter_value);
    destinationname= new char[len + 1];
    memcpy(destinationname, parameter_value, len + 1);
  }
  else if (!strcmp(parameter_name,"Filter"))
  {
    if (!strcmp(parameter_value,"ON")) Filter = MTP3_ON;
    else Filter = MTP3_OFF;
  }
  else if (!strcmp(parameter_name,"Loop"))
  {
    if (!strcmp(parameter_value,"ON")) Loop = MTP3_ON;
    else Loop = MTP3_OFF;
  }
  else if (!strcmp(parameter_name,"NI"))
  {
    Ni_is_set = TRUE;
    mtp3_ni = atoi(parameter_value);
    debuglog("Network indicator is set to %i",mtp3_ni);
  }
  else if (!strcmp(parameter_name,"SUT_Pc"))
  {
    Sut_Pc = atoi(parameter_value);
  }
  else if (!strcmp(parameter_name,"TESTER_Pc"))
  {
    Tester_Pc = atoi(parameter_value);
  }
  else if (!strcmp(parameter_name,"M3UA_version"))
  {
    M3UA_version  = atoi(parameter_value);
    debuglog("%d",M3UA_version);
  }
#ifndef TARGET_TEST
  else if (!strcmp(parameter_name,"DynamicConnection"))
  {
    if (!strcasecmp(parameter_value,"ON"))
    {
      dynamicConnection = TRUE;
    }
  }
#endif
  else if (!strcmp(parameter_name,"MTP3ServiceType"))
  {
    if (!strcmp(parameter_value,"TargetM3UA"))
    {
#ifndef TARGET_TEST
      error("TargetM3UA not supported, since TARGET_TEST not in Makefile");
#else
      log("MTP3ServiceType is set to TargetM3UA");
      user_map_p =  &MTP3asp__PT_PROVIDER::Target_user_map;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::Target_user_unmap;
      MTPServiceType = TargetM3UA;
#endif
    }
    else if (!strcmp(parameter_value,"TargetSTC"))
    {
#ifndef TARGET_TEST
      error("TargetSTC not supported, since TARGET_TEST not in Makefile");
#else
      log("MTP3ServiceType is set to TargetSTC");
      user_map_p =  &MTP3asp__PT_PROVIDER::TargetSTC_user_map;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::Target_user_unmap; //Same as by Target M3UA
      MTPServiceType = TargetSTC;
#endif
    }
#ifndef TARGET_TEST
    else if (!strcmp(parameter_value,"M3UA"))
    { //M3UA
      log("MTP3ServiceType is set to M3UA");
      interpreter = &MTP3asp__PT_PROVIDER::M3UA_interpreter;
      user_map_p =  &MTP3asp__PT_PROVIDER::M3UA_user_map;
      user_connect_p =  &MTP3asp__PT_PROVIDER::M3UA_user_connect;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::M3UA_user_unmap;
      MTPServiceType = M3UA;
    }
    else if (!strcmp(parameter_value,"MTP3itu"))
    {
      log("MTP3ServiceType is set to MTP3itu");
      interpreter = &MTP3asp__PT_PROVIDER::MTP3_interpreter;
      user_map_p =  &MTP3asp__PT_PROVIDER::MTP3_user_map;
      user_connect_p =  &MTP3asp__PT_PROVIDER::MTP3_user_connect;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::MTP3_user_unmap;
      MTPServiceType = MTP3itu;
    }
    else if ( !strcmp(parameter_value,"MTP3ansi"))
    {
      log("MTP3ServiceType is set to MTP3ansi");
      interpreter = &MTP3asp__PT_PROVIDER::MTP3_interpreter;
      user_map_p =  &MTP3asp__PT_PROVIDER::MTP3_user_map;
      user_connect_p =  &MTP3asp__PT_PROVIDER::MTP3_user_connect;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::MTP3_user_unmap;
      MTPServiceType = MTP3ansi;
    }
    else if ( !strcmp(parameter_value,"MTP3ttc"))
    {
      log("MTP3ServiceType is set to MTP3ttc");
      interpreter = &MTP3asp__PT_PROVIDER::MTP3_interpreter;
      user_map_p =  &MTP3asp__PT_PROVIDER::MTP3_user_map;
      user_connect_p =  &MTP3asp__PT_PROVIDER::MTP3_user_connect;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::MTP3_user_unmap;
      MTPServiceType = MTP3ttc;
    }
    else if ( !strcmp(parameter_value,"MTP3mpt"))
    {
      log("MTP3ServiceType is set to MTP3mpt");
      interpreter = &MTP3asp__PT_PROVIDER::MTP3_interpreter;
      user_map_p =  &MTP3asp__PT_PROVIDER::MTP3_user_map;
      user_connect_p =  &MTP3asp__PT_PROVIDER::MTP3_user_connect;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::MTP3_user_unmap;
      MTPServiceType = MTP3mpt;
    }
    else if ( !strcmp(parameter_value,"MTP3bttc"))
    {
      log("MTP3ServiceType is set to MTP3bttc");
      interpreter = &MTP3asp__PT_PROVIDER::MTP3_interpreter;
      user_map_p =  &MTP3asp__PT_PROVIDER::MTP3_user_map;
      user_connect_p =  &MTP3asp__PT_PROVIDER::MTP3_user_connect;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::MTP3_user_unmap;
      MTPServiceType = MTP3bttc;
    }
    else if ( !strcmp(parameter_value,"MTP3iup"))
    {
      log("MTP3ServiceType is set to MTP3iup");
      interpreter = &MTP3asp__PT_PROVIDER::MTP3_interpreter;
      user_map_p =  &MTP3asp__PT_PROVIDER::MTP3_user_map;
      user_connect_p =  &MTP3asp__PT_PROVIDER::MTP3_user_connect;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::MTP3_user_unmap;
      MTPServiceType = MTP3iup;
    }
    else if ( !strcmp(parameter_value,"STC"))
    {
      log("MTP3ServiceType is set to STC");
      interpreter = &MTP3asp__PT_PROVIDER::STC_interpreter;
      user_map_p =  &MTP3asp__PT_PROVIDER::STC_user_map;
      user_connect_p =  &MTP3asp__PT_PROVIDER::STC_user_connect;
      user_unmap_p =  &MTP3asp__PT_PROVIDER::STC_user_unmap;
      MTPServiceType = STC;
    }
#endif
    else
    {
      log("Unsupported MTP3ServiceType: %s, falling back to MTP3itu",
          parameter_value);
    }
  }
  else if(strcasecmp(parameter_name, "forward_pause") == 0)
  {
    if (strcasecmp(parameter_value,"forward") == 0)
      forward_pause = TRUE;
    else if(strcasecmp(parameter_value,"ignore") == 0)
      forward_pause = FALSE;
    else
      error("set_parameter(): Invalid parameter value: %s for parameter %s. Only forward and ignore can be used!" ,
            parameter_value, parameter_name);
  }
  else if(strcasecmp(parameter_name, "forward_resume") == 0)
  {
    if (strcasecmp(parameter_value,"forward") == 0)
      forward_resume = TRUE;
    else if(strcasecmp(parameter_value,"ignore") == 0)
      forward_resume = FALSE;
    else
      error("set_parameter(): Invalid parameter value: %s for parameter %s. Only forward and ignore can be used!" ,
            parameter_value, parameter_name);
  }
  else if(strcasecmp(parameter_name, "forward_status") == 0)
  {
    if (strcasecmp(parameter_value,"forward") == 0)
      forward_status = TRUE;
    else if(strcasecmp(parameter_value,"ignore") == 0)
      forward_status = FALSE;
    else
      error("set_parameter(): Invalid parameter value: %s for parameter %s. Only forward and ignore can be used!" ,
            parameter_value, parameter_name);
  }
  else
#ifdef TARGET_TEST
  if(!parameter_set(parameter_name ,parameter_value)) //TCP parameters
#endif
    log("Unsupported parameter: %s", parameter_name);
}
//------------

// User map
void MTP3asp__PT_PROVIDER::user_map(const char *system_port)
{
  if (user_map_p == NULL)
    error("Parameter MTP3ServiceType should be set to TargetM3UA in TARGET_TEST mode!");

  (this->*user_map_p)(system_port);
}
//------------

// User unmap
void MTP3asp__PT_PROVIDER::user_unmap(const char *system_port)
{
  (this->*user_unmap_p)(system_port);
}
//------------

#ifndef TARGET_TEST
// User connect
void MTP3asp__PT_PROVIDER::user_connect()
{
  (this->*user_connect_p)();
}
//------------
#endif

//User start
void MTP3asp__PT_PROVIDER::user_start()
{ debuglog("user start ordered");
}
//------------

//User stop
void MTP3asp__PT_PROVIDER::user_stop()
{ debuglog("User stop ordered");
}
//------------

//Event Handler
void MTP3asp__PT_PROVIDER::Handle_Fd_Event(int fd,
  boolean is_readable, boolean is_writable, boolean is_error)
{
  if (MTPServiceType == TargetM3UA )
  {
#ifdef TARGET_TEST
    //In case of target Abstract Socket handles the received message
    Handle_Socket_Event(fd, is_readable, is_writable, is_error);
#endif
  }
#ifndef TARGET_TEST
  else
  {
    int result;

    result = MPH_ProcessConnection(myConnection);

    if (result <= 0)
    {
      MPH_CloseConnection(myConnection);
      if (result == 0) log("Connection closed by peer.");
      else log("Error in incoming message.");
    }
  }
#endif
}

void MTP3asp__PT_PROVIDER::Handle_Timeout(double time_since_last_call)
{
#ifdef TARGET_TEST
  Handle_Timeout_Event(time_since_last_call);
#endif
}
//------------

//Outgoing send
void MTP3asp__PT_PROVIDER::outgoing_send(const ASP__MTP3__TRANSFERreq& send_par)
{
  MTP3__Field__sio sio_field = send_par.sio();
#ifndef TARGET_TEST
  int si=bit2int(sio_field.si());
#endif

  OCTETSTRING sio_oct = bit2oct(sio_field.ni()+sio_field.prio()+sio_field.si());
  OCTETSTRING bttc_oct = int2oct(stored_bttc_octet, 1); // additional octet for MTP3bttc

  //Message sending by testing on Target
  if (MTPServiceType == TargetM3UA)
  {
#ifdef TARGET_TEST
    OCTETSTRING tcpData = int2oct(1,1); //Message type
    tcpData = tcpData + int2oct(send_par.data().lengthof()+15,4); //Length
    tcpData = tcpData + sio_oct;
    tcpData = tcpData + int2oct(send_par.opc(),4);
    tcpData = tcpData + int2oct(send_par.dpc(),4);
    tcpData = tcpData + int2oct(send_par.sls(),1);
    tcpData = tcpData + send_par.data();
    send_outgoing((const unsigned char*)tcpData,tcpData.lengthof());

    TTCN_Logger::begin_event(TTCN_DEBUG);
    TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
    TTCN_Logger::log_event_str("Transfer Req message sent: ");
    tcpData.log();
    TTCN_Logger::end_event();
#endif
    return;
  }
  //Message sending by testing on Target
  if (MTPServiceType == TargetSTC)
  {
#ifdef TARGET_TEST
    OCTETSTRING tcpData = int2oct(1,1); //Message type
    tcpData = tcpData + int2oct(send_par.data().lengthof()+15,4); //Length
    tcpData = tcpData + int2oct(0,1);
    tcpData = tcpData + int2oct(0,4);
    tcpData = tcpData + int2oct(0,4);
    tcpData = tcpData + int2oct(0,1);
    tcpData = tcpData + send_par.data();
    send_outgoing((const unsigned char*)tcpData,tcpData.lengthof());

    TTCN_Logger::begin_event(TTCN_DEBUG);
    TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
    TTCN_Logger::log_event_str("Transfer Req (STC) message sent: ");
    tcpData.log();
    TTCN_Logger::end_event();
#endif
    return;
  }
#ifndef TARGET_TEST
  if (dynamicConnection &&(!connectionUp))
  {
    warn("Connection was not activated via function f_M3UA_SEA_connect.");
    return;
  }

  unsigned int offset;
  int MSU_length = send_par.data().lengthof();
  int M3UA_par_length;
  int length;
  int labellen; // sio+routinglabel length

  switch ( MTPServiceType)
  {
    case STC:
      length = MSU_length;
      memcpy(buffer, send_par.data(), send_par.data().lengthof());
      break;
    case MTP3iup:
      if(si==4)
      {
        offset = 0;
        labellen=6; //ITU-T:sio(1byte) + standard telephony label(5byte)
        length = MSU_length+labellen;
        buffer[0] = *((const unsigned char*)sio_oct);
        SetPointCodesIUP(send_par.sls(), send_par.opc(), send_par.dpc(),
           buffer + offset + 1);
        memcpy(buffer + offset + labellen, send_par.data(), send_par.data().lengthof());
      }
      else
      {
        offset = 0;
        labellen=5; //ITU-T:sio(1byte) + routing label(4byte) see /Q.704/15.4.1
        length = MSU_length + labellen; //SIF+SIO ;
        //append MTP3 MSU
        buffer[offset] = *((const unsigned char*)sio_oct);
        SetPointCodes(send_par.sls(), send_par.opc(), send_par.dpc(),
           buffer + offset + 1);
        memcpy(buffer + offset + labellen, send_par.data(), send_par.data().lengthof());
      }
      break;
    case MTP3itu:
      offset = 0;
      labellen=5; //ITU-T:sio(1byte) + routing label(4byte) see /Q.704/15.4.1
      length = MSU_length + labellen; //SIF+SIO ;
      //append MTP3 MSU
      buffer[offset] = *((const unsigned char*)sio_oct);
      SetPointCodes(send_par.sls(), send_par.opc(), send_par.dpc(),
         buffer + offset + 1);
      memcpy(buffer + offset + labellen, send_par.data(), send_par.data().lengthof());
      break;
    case MTP3ansi:
      offset = 0;
      labellen=8; //ANSI: sio(1byte) +routing label(7byte) see T1.111.4
      length = MSU_length + labellen; //SIF+SIO ;
      //append MTP3 MSU
      buffer[offset] = *((const unsigned char*)sio_oct);
      SetPointCodes(send_par.sls(), send_par.opc(), send_par.dpc(),
         buffer + offset + 1);
      memcpy(buffer + offset + labellen, send_par.data(), send_par.data().lengthof());
      break;
    case MTP3ttc:
      offset = 0;
      //if (mtp3_ni==0){ labellen=5;}
      //else {
      labellen=6;
      //} //TTC: sio(1byte) +routing label(6byte) see  ...
      length = MSU_length + labellen; //SIF+SIO ;
      //append MTP3 MSU
      buffer[offset] = *((const unsigned char*)sio_oct);
      SetPointCodes(send_par.sls(), send_par.opc(), send_par.dpc(),
         buffer + offset + 1);
      memcpy(buffer + offset + labellen, send_par.data(), send_par.data().lengthof());
      break;
    case MTP3bttc:
      offset = 0;
      buffer[offset] = *((const unsigned char*)bttc_oct);
      offset += 1;
      labellen=7; //routing label(7byte)
      length = 1 + MSU_length + labellen; //SIF+SIO ;
      //append MTP3 MSU
      buffer[offset] = *((const unsigned char*)sio_oct);
      SetPointCodes(send_par.sls(), send_par.opc(), send_par.dpc(),
         buffer + offset + 1);
      memcpy(buffer + offset + labellen, send_par.data(), send_par.data().lengthof());
      break;
    case MTP3mpt:
      offset = 0;
      if (mtp3_ni==2)
      { labellen=8;} //MPT national: sio(1byte) +routing label(7byte)
      else
      { labellen=5;} //MPT international: sio(1byte) +routing label(4byte)
      length = MSU_length + labellen; //SIF+SIO ;
      //append MTP3 MSU
      buffer[offset] = *((const unsigned char*)sio_oct);
      SetPointCodes(send_par.sls(), send_par.opc(), send_par.dpc(),
         buffer + offset + 1);
      memcpy(buffer + offset + labellen,
             send_par.data(), send_par.data().lengthof());
      break;
    case M3UA :
      TTCN_Logger::begin_event(TTCN_DEBUG);
      TTCN_Logger::log_event("MTP3 test port debug: Message to be encoded is: {");
      send_par.data().log();
      TTCN_Logger::log_event(" ");
      if( M3UAState != AssocActive )
      {
        TTCN_Logger::log_event(" M3UAState is not ready to send data. Its state code: %d",AssocActive);
        TTCN_Logger::log_event("}");
        TTCN_Logger::end_event();
        return;
      }
      //calculating lengths
      M3UA_par_length = MSU_length + 16; // ProtocolData parameter
                 // header length=16 <== see f
      TTCN_Logger::log_event(", Adjusted M3UA_par_length to %d to support 16 octets M3UA param header",M3UA_par_length);
      length = 8 + M3UA_par_length; //msg length = header+par
      TTCN_Logger::log_event(", M3UA MSU_length is %d ==> there should be %d padding octets", MSU_length, 4-(MSU_length%4));
      if (MSU_length%4) { //should be padded to be multiple of 4 octets
        length += 4 - (MSU_length%4); //padding shall be counted in msg
              //length, but not in par_length
      }
      TTCN_Logger::log_event(", Set msg length (which includes 8 octets M3UA hdr) to %d ",length);

      //filling the first part of the buffer
      //common msg hdr ======================================
      buffer[0] = M3UA_version;
      buffer[1] = 0x00; //spare
      buffer[2] = M3UA_MSG_CLS_TRNSFM; // msg class
      buffer[3] = M3UA_MSG_TYP_TRSNFM_DATA; // msg type
      encode_32b_int(buffer+4, length); //msg length, 4 bytes
      //ProtocolData parameter header=========================
      //tag
      buffer[M3UA_MSG_OFFS] = PAR_PREFIX_M3UA; //par. tag 1st octet
      buffer[M3UA_MSG_OFFS+1] = PAR_PROT_DATA; // par tag 2nd octet
      //length, NOTE: should not contain the padding bytes!
      encode_16b_int(buffer+M3UA_MSG_OFFS+2,M3UA_par_length);
      //OPC,DPC
      encode_32b_int(buffer+M3UA_MSG_OFFS+4,send_par.opc());
      encode_32b_int(buffer+M3UA_MSG_OFFS+8,send_par.dpc());
      //SI, NI, MP, SLS
      buffer[M3UA_MSG_OFFS+12] = bit2int(sio_field.si()); // SI LSb aligned
      buffer[M3UA_MSG_OFFS+13] = bit2int(sio_field.ni()); // NI LSb aligned
      buffer[M3UA_MSG_OFFS+14] = bit2int(sio_field.prio()); //MP LSb
                //aligned
      buffer[M3UA_MSG_OFFS+15] = 0xFF & send_par.sls(); //SLS
      // finally the MTP3 MSU itself....
      offset =  M3UA_MSG_OFFS + 16;
      TTCN_Logger::log_event(", buffer offset is now set to %d", offset);
      TTCN_Logger::log_event("}");
      TTCN_Logger::end_event();
      //append MTP3 MSU
      memcpy(buffer + offset, send_par.data(), send_par.data().lengthof());

      //padding
      for (int ii = 0; ii< (MSU_length%4); ++ii) buffer[offset+MSU_length+ii]= 0x00;
      break;
    default:
      error("Invalid MTP3ServiceType setting!");
  }
  if (TTCN_Logger::log_this_event(TTCN_DEBUG))
  {
    TTCN_Logger::begin_event(TTCN_DEBUG);
    TTCN_Logger::log_event("The encoded buffer is: {");
    OCTETSTRING(length, buffer).log();
    TTCN_Logger::log_event("}");
    TTCN_Logger::end_event();
  }
  send_msg(buffer, length);
#endif
}
//------------

void MTP3asp__PT_PROVIDER::log(const char *msg, ...)
{
  TTCN_Logger::begin_event(TTCN_PORTEVENT);
  TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
  va_list ap;
  va_start(ap, msg);
  TTCN_Logger::log_event_va_list(msg, ap);
  va_end(ap);
  TTCN_Logger::end_event();
}
//------------

void MTP3asp__PT_PROVIDER::warn(const char *msg, ...)
{
  TTCN_Logger::begin_event(TTCN_WARNING);
  TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
  va_list ap;
  va_start(ap, msg);
  TTCN_Logger::log_event_va_list(msg, ap);
  va_end(ap);
  TTCN_Logger::end_event();
}
//------------

void MTP3asp__PT_PROVIDER::debuglog(const char *msg, ...)
{
  TTCN_Logger::begin_event(TTCN_DEBUG);
  TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
  va_list ap;
  va_start(ap, msg);
  TTCN_Logger::log_event_va_list(msg, ap);
  va_end(ap);
  TTCN_Logger::end_event();
}
//------------

void MTP3asp__PT_PROVIDER::error(const char *msg, ...)
{
  TTCN_Logger::begin_event(TTCN_ERROR);
  TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
  va_list ap;
  va_start(ap, msg);
  TTCN_Logger::log_event_va_list(msg, ap);
  va_end(ap);
  TTCN_Logger::end_event();
  TTCN_error("Fatal error in MTP3 Test Port %s.", get_name());
}
//------------

void MTP3asp__PT_PROVIDER::close_log_event()
{
  TTCN_Logger::log_event("}");
  TTCN_Logger::end_event();
}
//------------


#ifndef TARGET_TEST
// --------------------------------------------
// Functions and definitions for test with SEA
// --------------------------------------------

// Functions of MPH toolkit
// ---------------------------
void connectCallback(CONNECTION* con, int channel, void *clientData)
{
  ((MTP3asp__PT_PROVIDER *)clientData)->log("Opening channel succeeded "
        "(channel number is %u)", channel);
  ((MTP3asp__PT_PROVIDER *)clientData)->set_channel(channel);
  if(((MTP3asp__PT_PROVIDER *)clientData)->dynamicConnection)
    ((MTP3asp__PT_PROVIDER *)clientData)->conn_state = 1;
}

void messageCallback(CONNECTION* con, int channel, int length,
                     unsigned char *msg, void *clientData)
{
  ((MTP3asp__PT_PROVIDER *)clientData)->log("Incoming message from channel: %d",channel);
  ((MTP3asp__PT_PROVIDER *)clientData)->doInterpret(msg,length,channel,con);
}

void closeCallback(CONNECTION* con, int channel, void *clientData)
{
  ((MTP3asp__PT_PROVIDER *)clientData)->log("Closed channel: %d", channel);
}

void errorCallback(CONNECTION* con, char *name, char *errorMessage,
                   void *clientData)
{
  if(((MTP3asp__PT_PROVIDER *)clientData)->dynamicConnection)
  {
    ((MTP3asp__PT_PROVIDER *)clientData)->log("Opening channel %s failed: %s", name, errorMessage);
    ((MTP3asp__PT_PROVIDER *)clientData)->conn_state = 2;
  }
  else
    ((MTP3asp__PT_PROVIDER *)clientData)->error("Opening channel %s failed: %s",
        name, errorMessage);
}
//------------

//MTP3_open_channel
void MTP3asp__PT_PROVIDER::MTP3_open_channel(boolean http)
{
  int result;
  int Mphport;
  char *perrorString;
  if( Loop == MTP3_ON )
  {
    MTP_fd=-1;
    myConnection = NULL;
    log("MTP3_open_channel finished for LOOP");
    return;
  }

  if(http)
  {
    Mphport = MPH_GetMphPort(hostname,httpport,&perrorString);
  }
  else
  {
    Mphport = httpport;
  }
    
  if (Mphport == -1)
    error("GetMphPort failed: %s", *perrorString);

  result = MPH_StringToIid(iid_string, &iid);
  if (result == -1)
    error("Converting %s to MPH_IID failed.", iid_string);

  myConnection = MPH_OpenConnection(hostname, Mphport);
  if (myConnection == NULL)
    error("Opening connection to %s:%d failed.", hostname, Mphport);

  MPH_OpenChannel(myConnection,
                  entityname,
                  &iid,
                  connectCallback,
                  messageCallback,
                  closeCallback,
                  errorCallback,
                  this);

  MTP_fd = MPH_GetConnectionFd(myConnection);
  if (MTP_fd != -1)
    Handler_Add_Fd_Read(MTP_fd);
  else
    error("Incorrect file descriptor: %d.", MTP_fd);
}
//------------

// MTP3_close_connection
void MTP3asp__PT_PROVIDER::MTP3_close_connection()
{
  MPH_CloseConnection(myConnection);
  Handler_Remove_Fd_Read(MTP_fd);
  close( MTP_fd );
  //Uninstall_Handler(); // Unnecessary if only socket MTP_fd is in use
}
//------------

// wait_for_open
boolean MTP3asp__PT_PROVIDER::wait_for_open()
{
  conn_state = 0;
  while(conn_state==0)
  {
    pollfd pollFd = { MTP_fd, POLLIN, 0 };
    int nEvents = poll(&pollFd, 1, 3000 /* ms */);
    if (nEvents == 0) {
      log("MPH channel opening time out");
      return FALSE;
    }
    if (nEvents < 0 || (pollFd.revents & (POLLIN | POLLHUP)) == 0) {
      log("MPH channel opening error (%d)", (nEvents < 0) ? errno : 0);
      return FALSE;
    }
    Handle_Fd_Event(MTP_fd, TRUE, FALSE, FALSE);
  }
  if(conn_state == 1) //connectCallback received
  {
    conn_state = 0;
    return TRUE;
  }
  else //errorCallback received
  {
    conn_state = 0;
    return FALSE;
  }
}
//------------

//send msg
void MTP3asp__PT_PROVIDER::send_msg(unsigned char *outbuff, int length)
{
  OCTETSTRING buff(length,outbuff);
  TTCN_Logger::begin_event(TTCN_DEBUG);
  TTCN_Logger::log_event("MTP3/M3UA Test Port (%s): {", get_name());
  TTCN_Logger::log_event("outgoing buffer= ");
  buff.log();
  TTCN_Logger::log_event("}");
  TTCN_Logger::end_event();
  if (Loop == MTP3_ON)
  {
    log("Message looped back");
    doInterpret(outbuff, length, channel, myConnection);
  } else
  {
    MPH_SendMessage(myConnection,channel,length,outbuff);
    log("Message sent on channel %d", channel);
  }
}
//------------

//Check TestPortVariables
void  MTP3asp__PT_PROVIDER::Check_TestPort_Variables()
{
  if(!dynamicConnection)
  {
    if (httpport==-1) error("Parameter HttpPort is not set.");
    if (hostname==NULL) error("Parameter Hostname is not set.");
    if (entityname==NULL) error("Parameter EntityName is not set.");
  }
  if (Filter==-1) error("Parameter Filter is not set.");
  if (Loop==-1) error("Parameter Loop is not set.");
  if (Sut_Pc==-1) error("Parameter SUT_Pc is not set.");
  if (Tester_Pc==-1) error("Parameter TESTER_Pc is not set.");
  if (!Ni_is_set) error("Parameter NI is not set.");
}
//------------

// -------------------------------------------------
// STC Functions and definitions for test with SEA
// -------------------------------------------------
void  MTP3asp__PT_PROVIDER::Check_TestPort_Variables_STC()
{
  if(!dynamicConnection)
  {
    if (httpport==-1) error("Parameter HttpPort is not set.");
    if (hostname==NULL) error("Parameter Hostname is not set.");
    if (entityname==NULL) error("Parameter EntityName is not set.");
  }
}
//------------

//STC user map
void MTP3asp__PT_PROVIDER::STC_user_map(const char *system_port)
{
  debuglog("Function STC_user_map started");
  Check_TestPort_Variables_STC();
  if(dynamicConnection)
  {
    connectionUp = FALSE;
  }
  else
  {
    MTP3_open_channel(TRUE);
    STC_user_connect();
  }
}
//------------

//STC user connect
void MTP3asp__PT_PROVIDER::STC_user_connect()
{
}

//STC user unmap
void MTP3asp__PT_PROVIDER::STC_user_unmap(const char *system_port)
{
  MTP3_close_connection();
  dynamicConnection = FALSE;
}
//------------

//STC interpreter
void MTP3asp__PT_PROVIDER::STC_interpreter(unsigned char* inbuffer,int length,int from_channel,CONNECTION* con)
{
  if ((length==0) || (inbuffer==NULL))
  {
    log("0 byte long message received -> packet dropped.");
    return;
  }

  if (length==1)
  {
    log("1 byte long internal SEA message received -> packet dropped.");
    return;
  }

  if ( !strcmp((const char*)inbuffer,"start") )
  {
    log("start message received from SEA");
    return;
  }
  else if (!strcmp((const char*)inbuffer,"stop"))
  {
    log("stop message received from SEA");
    return;
  }

  // writing out the contents of the buffer
  OCTETSTRING buff(length,inbuffer);
  TTCN_Logger::begin_event(TTCN_PORTEVENT);
  TTCN_Logger::log_event("incoming buffer: ");
  buff.log();
  TTCN_Logger::end_event();

  ASP__MTP3__TRANSFERind recv_msg;
  MTP3__Field__sio recv_sio;
  recv_sio.ni()= int2bit(0,2);
  recv_sio.prio()= int2bit(0,2);
  recv_sio.si()= int2bit(0,4);
  recv_msg.sio() = recv_sio;
  recv_msg.sls() = 0;
  recv_msg.opc() = 0;
  recv_msg.dpc() = 0;
  recv_msg.data() = OCTETSTRING(length, &inbuffer[0]);
  incoming_message( recv_msg );
}

// -------------------------------------------------
// MTP3 Functions and definitions for test with SEA
// -------------------------------------------------

// SLTM messages for MTP3
// -----------------------
unsigned char ttcn_in_sltm[] = {
  'T','T','C','N','-','3',' ','E','x','e','c','u','t','o','r'};

const int sizeof_ttcn_in_sltm=15;

// ITU:
unsigned char sltm_msg_itu[] =
{
  0x1,  //SIO /'test & maint' see Q.704 /14.2.1 => 0x81 suggested !!!
  0x0,  0x0, 0x0,  0x0, //4 bytes for label (dpc, opc,sls)
  0x11, //Heading code ITU, see Q.707/5.4
  0xF0, //spare+Length of the following string:
  'T', 'T', 'C', 'N', '-', '3', ' ', 'E', 'x', 'e', 'c', 'u', 't', 'o', 'r'
};
const int sizeof_sltm_msg_itu=7+15;

// ANSI:
unsigned char sltm_msg_ansi[]=
{
  0xB1, //SIO
  0x0,0x0,0x0,0x0,0x0,0x0,0x0, // 7 bytes for label
  0x11,  // Heading Code ANSI T1.111.7-2001
  0xF0, // SLC(is 0 OK???)+Length in bytes of the following string:
  'T', 'T', 'C', 'N', '-', '3', ' ', 'E', 'x', 'e', 'c', 'u', 't', 'o', 'r'
};
const int sizeof_sltm_ansi=10+15;

// TTC:
unsigned char sltm_msg_ttc_national[]=
{
  0x81, //SIO
  0x0,0x0,0x0,0x0,0x0,0x0, // 6 bytes for label TO BE CONT!!!
  0x11,  // Heading Code
  0xF0, // SLC(is 0 OK???)+Length in bytes of the following string:
  'T', 'T', 'C', 'N', '-', '3', ' ', 'E', 'x', 'e', 'c', 'u', 't', 'o', 'r'
};
const int sizeof_sltm_msg_ttc_national=9+15;

// BTTC:
unsigned char sltm_msg_bttc_national[]=
{
  0x0, // extra octet, ignored
  0x81, //SIO
  0x0,0x0,0x0,0x0,0x0,0x0, // 6 bytes for label TO BE CONT!!!
  0x23,  // Heading Code
  'T', 'T'
};
const int sizeof_sltm_msg_bttc_national=1+8+2;

// MPT:
unsigned char sltm_msg_mpt_national[]=
{
  0x81, //SIO
  0x0,0x0,0x0,0x0,0x0,0x0,0x0, // 7 bytes for label
  0x11,  // Heading Code
  0xF0, // SLC(is 0 OK???)+Length in bytes of the following string:
  'T', 'T', 'C', 'N', '-', '3', ' ', 'E', 'x', 'e', 'c', 'u', 't', 'o', 'r'
};
const int sizeof_sltm_msg_mpt_national=10+15;
//------------


// coder functions for MTP3
// -------------------------
// unsigned int<-> unsigned char array
// Integer encode/decode functions that will encode/decode from/to
// Result: Least Significant Byte first (in lowest address) = LSB = Little Endian
void MTP3asp__PT_PROVIDER::encode_56bLSB_int(unsigned char *to, unsigned long long int from)
{
  to[0] = from & 0xFF;
  from >>= 8;
  to[1] = from & 0xFF;
  from >>= 8;
  to[2] = from & 0xFF;
  from >>= 8;
  to[3] = from & 0xFF;
  from >>= 8;
  to[4] = from & 0xFF;
  from >>= 8;
  to[5] = from & 0xFF;
  from >>= 8;
  to[6] = from & 0xFF;
}
//------------

unsigned long long int MTP3asp__PT_PROVIDER::decode_56bLSB_int(const unsigned char *from)
{
  typedef unsigned long long int ull;
  return ((ull) from[0]) |
         ((ull) from[1] <<8)|
         ((ull) from[2] << 16)|
         ((ull) from[3] << 24)|
         ((ull) from[4] << 32)|
         ((ull) from[5] << 40)|
         ((ull) from[6] << 48);
}
//------------

void MTP3asp__PT_PROVIDER::encode_48bLSB_int(unsigned char *to, unsigned long long int from)
{
  to[0] = from & 0xFF;
  from >>= 8;
  to[1] = from & 0xFF;
  from >>= 8;
  to[2] = from & 0xFF;
  from >>= 8;
  to[3] = from & 0xFF;
  from >>= 8;
  to[4] = from & 0xFF;
  from >>= 8;
  to[5] = from & 0xFF;
}
//------------

unsigned long long int MTP3asp__PT_PROVIDER::decode_48bLSB_int(const unsigned char *from)
{
  typedef unsigned long long int ull;
  return ((ull) from[0]) |
         ((ull) from[1] <<8)|
         ((ull) from[2] << 16)|
         ((ull) from[3] << 24)|
         ((ull) from[4] << 32)|
         ((ull) from[5] << 40);
}
//------------

void MTP3asp__PT_PROVIDER::encode_40bLSB_int(unsigned char *to, unsigned long long int from)
{
  to[0] = from & 0xFF;
  from >>= 8;
  to[1] = from & 0xFF;
  from >>= 8;
  to[2] = from & 0xFF;
  from >>= 8;
  to[3] = from & 0xFF;
  from >>= 8;
  to[4] = from & 0xFF;
}
//------------

unsigned long long int MTP3asp__PT_PROVIDER::decode_40bLSB_int(const unsigned char *from)
{
  typedef unsigned long long int ull;
  return ((ull) from[0]) |
         ((ull) from[1] <<8)|
         ((ull) from[2] << 16)|
         ((ull) from[3] << 24)|
         ((ull) from[4] << 32);
}
//------------

void MTP3asp__PT_PROVIDER::encode_32bLSB_int(unsigned char *to, unsigned int from)
{
  to[0] = from & 0xFF;
  from >>= 8;
  to[1] = from & 0xFF;
  from >>= 8;
  to[2] = from & 0xFF;
  from >>= 8;
  to[3] = from & 0xFF;
}
//------------

unsigned int MTP3asp__PT_PROVIDER::decode_32bLSB_int(const unsigned char *from)
{
  return from[0] | (from[1] << 8) | (from[2] << 16) | (from[3] << 24);
}
//------------

void MTP3asp__PT_PROVIDER::encode_24bLSB_int(unsigned char *to, int from)
{
  to[0] = from & 0xFF;
  from >>= 8;
  to[1] = from & 0xFF;
  from >>= 8;
  to[2] = from & 0xFF;
}
//------------

unsigned int MTP3asp__PT_PROVIDER::decode_24bLSB_int(const unsigned char *from)
{
  return from[0] | (from[1] << 8) | (from[2] << 16);
}
//------------

void MTP3asp__PT_PROVIDER::encode_16bLSB_int(unsigned char *to, int from)
{
  to[0] = from & 0xFF;
  from >>= 8;
  to[1] = from & 0xFF;
}
//------------

unsigned int MTP3asp__PT_PROVIDER::decode_16bLSB_int(const unsigned char *from)
{
  return from[0] | (from[1] << 8);
}
//------------

//MTP3 user map
void MTP3asp__PT_PROVIDER::MTP3_user_map(const char *system_port)
{
  debuglog("Function MTP3_user_map started");
  Check_TestPort_Variables();
   if(dynamicConnection)
  {
    connectionUp = FALSE;
  }
  else
  { 
    MTP3_open_channel(TRUE);
    MTP3_user_connect();
  }
  debuglog("Function MTP3_user_map finished");
}
//------------

//MTP3 user connect
void MTP3asp__PT_PROVIDER::MTP3_user_connect()
{
  // Sending out an SLTM message:
  unsigned char * sltm_msg;
  unsigned int offset = 0;
  int sizeof_msg;
  switch( MTPServiceType)
  {
    case MTP3itu:
    case MTP3iup:
      sltm_msg=sltm_msg_itu;
      sizeof_msg=sizeof_sltm_msg_itu;
      break;
    case MTP3ansi:
      sltm_msg=sltm_msg_ansi;
      sizeof_msg=sizeof_sltm_ansi;
      break;
    case MTP3ttc:
  /*    if (mtp3_ni == 0){
      sltm_msg=sltm_msg_itu;
      sizeof_msg=sizeof_sltm_msg_itu;
      }
      else {
  */
      sltm_msg=sltm_msg_ttc_national;
      sizeof_msg=sizeof_sltm_msg_ttc_national;
  //    }
      break;
    case MTP3bttc:
      sltm_msg=sltm_msg_bttc_national;
      sizeof_msg=sizeof_sltm_msg_bttc_national;
      offset = 1;
      break;
    case MTP3mpt:
      if (mtp3_ni == 2)
      {
        sltm_msg = sltm_msg_mpt_national;
        sizeof_msg = sizeof_sltm_msg_mpt_national;
      } else
      {
        sltm_msg = sltm_msg_itu;
        sizeof_msg = sizeof_sltm_msg_itu;
      }
      break;
    default:
      sltm_msg=sltm_msg_itu;
      sizeof_msg=sizeof_sltm_msg_itu;
      break;
  }
  stored_bttc_octet = 0;
  unsigned char sio = ((unsigned char) mtp3_ni) << 6;

  if (Ni_is_set)
  { sltm_msg[0+offset] = sio | 0x1; }
  else
  { sltm_msg[0+offset] = 0x1; };//SIO /'test & maint' see Q.704 /14.2.1 => 0x81 suggested !!!

  SetPointCodes(0, Tester_Pc, Sut_Pc, sltm_msg + 1 +offset); // common for ITU, ANSI and TTC
  log("MTP3/SLTM message sending...");
  send_msg(sltm_msg, sizeof_msg);
}
//MTP3 user unmap
void MTP3asp__PT_PROVIDER::MTP3_user_unmap(const char *system_port)
{
  MTP3_close_connection();
  dynamicConnection = FALSE;
}

//MTP3 interpreter
void MTP3asp__PT_PROVIDER::MTP3_interpreter(unsigned char* inbuffer,int length,int from_channel,CONNECTION* con)
{
  if ((length==0) || (inbuffer==NULL))
  {
    log("0 byte long message received -> packet dropped.");
    return;
  }

  if (length==1)
  {
    log("1 byte long internal SEA message received -> packet dropped.");
    return;
  }

  if ( !strcmp((const char*)inbuffer,"start") )
  {
    log("start message received from SEA");
    return;
  }
  else if (!strcmp((const char*)inbuffer,"stop"))
  {
    log("stop message received from SEA");
    return;
  }

  // writing out the contents of the buffer
  OCTETSTRING buff(length,inbuffer);
  TTCN_Logger::begin_event(TTCN_PORTEVENT);
  TTCN_Logger::log_event("incoming buffer: ");
  buff.log();
  TTCN_Logger::end_event();

  unsigned int offset = 0;
  if ( MTPServiceType==MTP3bttc )
  {
    stored_bttc_octet = inbuffer[0];
    offset = 1;
  }
  int labellen; // sio+routinglabel length
  int rec_ni = (inbuffer[offset]) >> 6;  //network indicator
  if (rec_ni != mtp3_ni)
    error("Received NI is different from sent NI.");

  unsigned char sio = inbuffer[offset];
  unsigned int si = sio&0x0F;

  if ( MTPServiceType==MTP3itu )
  { labellen=5; //ITU-T:sio(1byte) + routing label(4byte) see /Q.704/15.4.1
  }
  else if ( MTPServiceType==MTP3iup )
  { 
    if(si==4)
      labellen=6; //ITU-T:sio(1byte) + standard telephony label(5byte)
    else
      labellen=5; //ITU-T:sio(1byte) + routing label(4byte) see /Q.704/15.4.1
  }
  else if ( MTPServiceType==MTP3ansi )
  { labellen=8; //ANSI: sio(1byte) +routing label(7byte) see T1.111.4
  }
  else if (MTPServiceType==MTP3ttc)
  { labellen=6; //new (2004-03-02): 6= sio(1byte)+ routing label(5bytes)
  }
  else if (MTPServiceType==MTP3mpt)
  {
     if ( mtp3_ni == 2)
       {labellen=8;} //MPT national: sio(1byte) +routing label(7byte)
     else
       {labellen=5;} //MPT international: sio(1byte) +routing label(4byte)
  }
  else if (MTPServiceType==MTP3bttc)
  { labellen=7; //7= sio(1byte)+ routing label(6bytes)
  }
  else
  { log("incorrect MTPServiceType - programming error-> packet dropped");
    return;
  }

  // checking SIO field (first incoming byte) - management or test message
  switch (si)
  {
    case 0: processing_MTP3_management_msg(inbuffer+offset,length-offset);
      return;
    case 1:                                                          //MTP3itu
    case 2: processing_MTP3_test_msg(inbuffer+offset,length-offset); //MTP3ansi
      return;
    default:
      break;
  }

  // filling up TTCN structure
  if ((Loop==MTP3_ON) || (!Filter) || (Filter&&Check_PcMatch(Sut_Pc,Tester_Pc,&inbuffer[offset+1])))
  {
    ASP__MTP3__TRANSFERind recv_msg;
    MTP3__Field__sio recv_sio;
    BITSTRING sio_bit = oct2bit(OCTETSTRING(1,inbuffer+offset));
    recv_sio.ni()= substr(sio_bit,0,2);
    recv_sio.prio()= substr(sio_bit,2,2);
    recv_sio.si()= substr(sio_bit,4,4);
    recv_msg.sio() = recv_sio;
    unsigned int sls,opc,dpc;
    if ( (MTPServiceType==MTP3iup) && (si==4) )
      GetPointCodesIUP(sls,opc,dpc,&inbuffer[1]);
    else
      GetPointCodes(sls,opc,dpc,&inbuffer[offset+1]);
    recv_msg.sls() = sls;
    recv_msg.opc() = opc;
    recv_msg.dpc() = dpc;
    int len;
    len= length-labellen-offset; //len= length-labellen;
    recv_msg.data() = OCTETSTRING(len, &inbuffer[offset+labellen]);
    incoming_message( recv_msg );
  }
  else
  { log("The rooting label (OPC, DPC) not matched with the filter setting -> packet dropped.");
    return;
  }
}

void MTP3asp__PT_PROVIDER::processing_MTP3_management_msg(unsigned char* inbuff,int len)
{
  int outlen=0;
  int labellen; // sio+routinglabel length
  int chm_addlen;  // (Changeback) additional length = Heading Code + SLC+ (changeback codes)
  int mim_addlen;  // (MIM)          -"-
  unsigned int offset = 0;
  OCTETSTRING bttc_oct = int2oct(stored_bttc_octet, 1); // additional octet for MTP3bttc

  if ( MTPServiceType==MTP3itu || MTPServiceType==MTP3iup)
  { labellen=5; //ITU-T:sio(1byte) + routing label(4byte) see /Q.704/15.4.1
    chm_addlen = 2;
    mim_addlen = 1;
  }
  else if ( MTPServiceType==MTP3ansi )
  { labellen=8; //ANSI: sio(1byte) +routing label(7byte) see T.1.111.4
    chm_addlen = 3;
    mim_addlen = 2;
  }
  else if (MTPServiceType==MTP3ttc)
  { //if ( mtp3_ni == 0 ) { labellen=5;} //ITU-T:sio(1byte) + routing label(4byte) see /Q.704/15.4.1
    //else {
    labellen=6;
    //} //sio(1byte)+ routing label(6bytes) see 3/15517-FAY 112 011/2 or jt-q704.
  }
  else if (MTPServiceType==MTP3mpt)
  { if ( mtp3_ni == 2 ) { labellen=8; }
    else { labellen=5; }
    chm_addlen = 2;
    mim_addlen = 1;
  }
  else if (MTPServiceType==MTP3bttc)
  { labellen=7;//sio(1byte)+routing label(6bytes) see 3/15517-FAY 112 011/2 or jt-q704
    offset = 1;
  }
  else
  { log("incorrect MTPServiceType- programming error-> packet dropped");
    return;
  }

  if (MTPServiceType==MTP3bttc)
  {
    buffer[0]=*((const unsigned char*)bttc_oct); //additional stored octet in front
    buffer[1]=inbuff[0]; //SIO
    if (!ChangePointCodes(&buffer[2],&inbuff[1],len))
    { log("incorrect incoming management message -> packet dropped.");
      return;
    }
  }
  else
  {
    buffer[0]=inbuff[0]; //SIO
    if (!ChangePointCodes(&buffer[1],&inbuff[1],len))
    { log("incorrect incoming management message -> packet dropped.");
      return;
    }
  }

  // Changeover & changeback(CHM) see ITU: Q.704/15.4.1  ANSI: T.1.111.4/15.2-4
  if (inbuff[labellen]==0x51) //CBD
  {
    outlen=labellen+chm_addlen;
    if (len<outlen)
    { log("incorrect incoming CHM message -> packet dropped.");
      return;
    }
    else
    { buffer[offset + labellen]= 0x61; // Heading Code <- CBA
      memcpy(&buffer[offset + labellen+1],&inbuff[labellen+1],len-labellen-1);
      outlen = len;
    }
  }
  else if (inbuff[labellen]==0x16) //MIM  H0=6=Mgmt inhibit msg, H1=1=LIN
  {
    outlen=labellen+mim_addlen;
    if (len<outlen)
    { log("incorrect incoming MIM message -> packet dropped.");
      return;
    }
    else
    {
      buffer[offset + labellen]= 0x56; // LIN -> LID  ; LID = link inhibit denied
      memcpy(&buffer[offset + labellen+1],&inbuff[labellen+1],(len-labellen-1)); //SLC+spare+...
      outlen = len;
    }
  }
  else if (inbuff[labellen]==0x17 && (MTPServiceType==MTP3iup))
  {
    if(forward_resume) incoming_message(ASP__MTP3__RESUME(NULL_VALUE));
    return;
  }

/*  else if (inbuff[labellen]==0x17 && (MTPServiceType==MTP3iup)) //TRM  H0=7, H1=1 : TRA
  {
    outlen=labellen+1;
    if (len<outlen)
    { log("incorrect incoming TRA message -> packet dropped.");
      return;
    }
    else
    {
      outlen = len;
    }
  }*/
  else
  { log("This management message type is not supported -> packet dropped.");
    return;
  }

  // send message
  log("function processing_MTP3_management_msg sends a msg");
  send_msg(buffer, outlen+offset);
}

// processing MTP3 test msg
// Signalling link test message handling according to Q.707 (ITU) and T1.111.7-2001 (ANSI)
void MTP3asp__PT_PROVIDER::processing_MTP3_test_msg(unsigned char* inbuff,int len)
{
  int outlen=0;
  int labellen; // sio+routinglabel length
  int addlen;   //Heading Code+length indicator
  //unsigned int ni; //network indicator
  OCTETSTRING bttc_oct = int2oct(stored_bttc_octet, 1); // additional octet for MTP3bttc
  unsigned int offset = 0;
  if ( MTPServiceType==MTP3itu || MTPServiceType==MTP3iup)
  {
    labellen=5; //ITU-T:sio(1byte) + routing label(4byte) see /Q.704/15.4.1
    addlen = 2; //HC(1 byte)+length ind(1 byte) see Q.707/5.8
  }
  else if ( MTPServiceType==MTP3ansi )
  {
    labellen=8; //ANSI: sio(1byte) +routing label(7byte) see T1.111.4
    addlen = 2; //HC(1byte) +(length ind+SLC(1byte)) see  T1.111.7-2001/5
  }
  else if (MTPServiceType==MTP3ttc)
  {
  /*  if (mtp3_ni == 0) {
        debuglog("processing_MTP3_test_msg/TTC (Japanese) international");
        labellen=5;  //TTC (Japanese) international
        addlen=2;
    } else {
  */
        labellen=6; // previously 7
                    // TTC national [ 56bits=sio(1byte)+routing label ]
                    // r.label= dpc(2bytes)+opc(2bytes)+sls(4bit)+12bits (?)
        addlen=2;
    //}
  }
  else if (MTPServiceType==MTP3mpt)
  {
    if (mtp3_ni == 2)
    { labellen=8;  //MPT national
      addlen=2;
    }
    else
    { labellen=5; // MPT international
      addlen=2;
    }
  }
  else if (MTPServiceType==MTP3bttc)
  {
    labellen=7;
    addlen=2;
    offset = 1;
  }
  else
  { log("incorrect MTPServiceType - programming error-> packet dropped");
    return;
  }

  if (MTPServiceType==MTP3bttc)
  {
    buffer[0]=*((const unsigned char*)bttc_oct); //additional stored octet in front
    buffer[1]=inbuff[0]; //SIO
    if (!ChangePointCodes(&buffer[2],&inbuff[1],len))
    {
      log("incorrect incoming test message -> packet dropped.");
      return;
    }
  }
  else
  {
    buffer[0]=inbuff[0]; //SIO
    if (!ChangePointCodes(&buffer[1],&inbuff[1],len))
    {
      log("incorrect incoming test message -> packet dropped.");
      return;
    }
  }

  // Test message handling: SLTM->SLTA, SRT->SRA, SLTA->TRA, others only logged
  debuglog("\n==>Test message handling: msg type:%x\n",   inbuff[labellen] ); // temporary
  switch( inbuff[labellen] )
  {
    case 0x11: //SLTM
      log("MTP3/SLTM message received");
      outlen=labellen+addlen;
      if (len<outlen )
      { log("incorrect incoming MTP3/SLTM message -> packet dropped.");
        return;
      }
      else
      { buffer[offset + labellen]= 0x21; // SLTA
        memcpy(&buffer[offset + labellen+1],&inbuff[labellen+1],(len-labellen-1));
        outlen = len;
      }
      break;
    case 0x21: //SLTA
      if(!strncmp((const char *)(inbuff+labellen+addlen),(const char *)ttcn_in_sltm,sizeof_ttcn_in_sltm))
      {
        log("MTP3/SLTA message received for  SLTM sent by Test Port-> TRA message sent");
        buffer[offset]--;
        buffer[offset + labellen]= 0x17; // TRA
        outlen = labellen+1;
        return;
//        if(forward_resume) incoming_message(ASP__MTP3__RESUME(NULL_VALUE));
      }
      else
      {
        log("MTP3/SLTA message received -> packet dropped");
        return;
      }
      break;
    case 0x23: // TTC (Japanese) SRT (Signalling Routing Test signal)
      log("MTP3ttc/SRT message received");
      outlen=labellen+addlen;
      if (len<outlen )
      { log("incorrect incoming MTP3ttc/SRT message (length error)-> packet dropped.");
        return;
      }
      else
      { buffer[offset + labellen]= 0x84; // TTC (Japanese) SRA
        memcpy(&buffer[offset + labellen+1],&inbuff[labellen+1],(len-labellen-1));
        outlen = len;
      }
      break;
    case 0x84: // TTC (Japanese) SRA (Signalling Routing test Ack signal)
      log("MTP3ttc/SRA message received -> packet dropped");
      return;
    default:
      log("This management message type is not supported -> packet dropped ");
      return;
  }

  // send message
  log("function processing_MTP3_test_msg sends a message");
  send_msg( buffer,outlen+offset);
}

// Point Code Manipulation (Get/Set/Change)
// -------------------------------------------------
void MTP3asp__PT_PROVIDER::GetPointCodes(unsigned int &sls,unsigned int &opc,unsigned int &dpc, unsigned char* msg)
{
  unsigned int label;
  sls=0; opc=0; dpc=0;
  unsigned long long int Label;

  switch( MTPServiceType)
  {
    case MTP3itu:
      label= decode_32bLSB_int(msg);
      sls  = (label>>28)&0xF;    //sls  = (label&0xF0000000)>>28;
      opc  = (label>>14)&0x3FFF; //opc  = (label&0x0FFFC000)>>14;
      dpc  = label&0x3FFF;       //dpc  = (label&0x00003FFF);
      debuglog("Function GetPointCodes called for service type MTP3itu");
      break;
    case MTP3iup:
      label= decode_32bLSB_int(msg);
      sls  = (label>>28)&0xF;    //sls  = (label&0xF0000000)>>28;
      opc  = (label>>14)&0x3FFF; //opc  = (label&0x0FFFC000)>>14;
      dpc  = label&0x3FFF;       //dpc  = (label&0x00003FFF);
      debuglog("Function GetPointCodes called for service type MTP3iup");
      break;
    case MTP3ansi:
      Label=decode_56bLSB_int(msg);
      sls  = (Label >> 48) & 0xFF;    //sls  = (Label&0x00FF000000000000)>>48;
      opc  = (Label >> 24) & 0xFFFFFF;//opc  = (Label&0x0000FFFFFF000000)>>24;
      dpc  = Label & 0xFFFFFF;        //dpc  = (Label&0x0000000000FFFFFF);
      debuglog("Function GetPointCodes called for service type MTP3ansi");
      break;
    case MTP3ttc:
    /*
      if( mtp3_ni == 0)
      {
        label= decode_32bLSB_int(msg);
        sls  = (label>>28)&0xF;     //sls  = (label&0xF0000000)>>28;
        opc  = (label>>14)&0x3FFF;  //opc  = (label&0x0FFFC000)>>14;
        dpc  = label&0x3FFF;
        debuglog("Function GetPointCodes called for service type MTP3ttc/international");
      } else
      {*/
      Label=decode_48bLSB_int(msg);            //0x010203040506
      sls  = (Label>>32)&0xF;   // sls  = (Label&0x000F00000000)>>32; // only 4 bits!!!
      opc  = (Label>>16)&0xFFFF;//opc  = (Label&0x0000FFFF0000)>>16;
      dpc  = Label&0xFFFF;      //dpc  = (Label&0x00000000FFFF);
      debuglog("Function GetPointCodes called for service type MTP3ttc/national");
      //}
      break;
    case MTP3bttc:
      Label=decode_48bLSB_int(msg);
      sls  = (Label>>32)&0xF;   // sls  = (Label&0x000F00000000)>>32; // only 4 bits!!!
      opc  = (Label>>16)&0xFFFF;//opc  = (Label&0x0000FFFF0000)>>16;
      dpc  = Label&0xFFFF;      //dpc  = (Label&0x00000000FFFF);
      debuglog("Function GetPointCodes called for service type MTP3bttc/national");
      break;
    case MTP3mpt:
      if( mtp3_ni == 2)
      {
        Label=decode_56bLSB_int(msg);
        sls  = (Label >> 48) & 0xFF;    //sls  = (Label&0x00FF000000000000)>>48;
        opc  = (Label >> 24) & 0xFFFFFF;//opc  = (Label&0x0000FFFFFF000000)>>24;
        dpc  = Label & 0xFFFFFF;        //dpc  = (Label&0x0000000000FFFFFF);
        debuglog("Function GetPointCodes called for service type MTP3mpt(nat)");
      }
      else
      {
        label= decode_32bLSB_int(msg);
        sls  = (label>>28)&0xF;    //sls  = (label&0xF0000000)>>28;
        opc  = (label>>14)&0x3FFF; //opc  = (label&0x0FFFC000)>>14;
        dpc  = label&0x3FFF;       //dpc  = (label&0x00003FFF);
        debuglog("Function GetPointCodes called for service type MTP3mpt(int)");
      }
      break;
    default:
      break;
  }
  debuglog("sls:%u opc:%u, dpc:%u",sls,opc,dpc);
}
//------------

void MTP3asp__PT_PROVIDER::GetPointCodesIUP(unsigned int &cic,unsigned int &opc,unsigned int &dpc, unsigned char* msg)
{
  debuglog("Function GetPointCodesIUP called");
  cic=0; opc=0; dpc=0;
  unsigned long long int Label;
  Label=decode_40bLSB_int(msg);            //0x0102030405
  cic  = (Label>>28)&0xFFF;  //cic  = (label&0xFFF0000000)>>28;
  opc  = (Label>>14)&0x3FFF; //opc  = (label&0x000FFFC000)>>14;
  dpc  = Label&0x3FFF;       //dpc  = (label&0x0000003FFF);
  debuglog("cic:%u opc:%u, dpc:%u",cic,opc,dpc);
}

void MTP3asp__PT_PROVIDER::SetPointCodes(unsigned int sls,unsigned int opc,unsigned int dpc, unsigned char* msg)
{
  unsigned long long int Sls,Opc,Dpc;

  switch( MTPServiceType)
  {
    case MTP3itu:
      encode_32bLSB_int( msg, ((sls<<28)|(opc<<14)|dpc ));
      debuglog("Function SetPointCodes called for service type MTP3itu");
      break;
    case MTP3iup:
      encode_32bLSB_int( msg, ((sls<<28)|(opc<<14)|dpc ));
      debuglog("Function SetPointCodes called for service type MTP3iup");
      break;
    case MTP3ansi:
      Sls=sls; Opc=opc; Dpc=dpc;
      encode_56bLSB_int( msg, ((Sls<<48)|(Opc<<24)|Dpc));
      debuglog("Function SetPointCodes called for service type MTP3ansi");
      break;
    case MTP3ttc:
     /* if ( mtp3_ni == 0 ){
        encode_32bLSB_int( msg, ((sls<<28)|(opc<<14)|dpc ));
        debuglog("Function SetPointCodes called for service type MTP3ttc/international");
      } else {
     */
        Sls=sls; Opc=opc; Dpc=dpc;
        encode_48bLSB_int( msg, ((Sls<<32)|(Opc<<16)|Dpc));
        debuglog("Function SetPointCodes called for service type MTP3ttc/national");
      //}
      break;
    case MTP3bttc:
        Sls=sls; Opc=opc; Dpc=dpc;
        encode_48bLSB_int( msg, ((Sls<<32)|(Opc<<16)|Dpc));
        debuglog("Function SetPointCodes called for service type MTP3bttc/national");
      break;
    case MTP3mpt:
      if ( mtp3_ni == 2 )
      { Sls=sls; Opc=opc; Dpc=dpc;
        encode_56bLSB_int( msg, ((Sls<<48)|(Opc<<24)|Dpc));
        debuglog("Function SetPointCodes called for service type MTP3mpt(nat)");
      }
      else
      { encode_32bLSB_int( msg, ((sls<<28)|(opc<<14)|dpc ));
        debuglog("Function SetPointCodes called for service type MTP3mpt(int)");
      }
      break;
    default:
      break;
  }
}
//------------

void MTP3asp__PT_PROVIDER::SetPointCodesIUP(unsigned int cic,unsigned int opc,unsigned int dpc, unsigned char* msg)
{
  unsigned long long int Cic,Opc,Dpc;
  Cic=cic; Opc=opc; Dpc=dpc;
  debuglog("Function SetPointCodesIUP called");
  encode_40bLSB_int( msg, ((Cic<<28)|(Opc<<14)|Dpc ));
}

//Changes the Point  codes: dpc<->opc ie. destination <->orig
// inbuff starts from dpc i.e doesn't contain sio !!!!
int MTP3asp__PT_PROVIDER::ChangePointCodes(unsigned char* outbuff, unsigned char *inbuff, int len)
{
  switch( MTPServiceType)
  {
    case MTP3itu:
    case MTP3iup:
      if (len<5)
      { warn("MTP3itu:len<5. Too short message!"); return 0; };
      break;
    case MTP3ansi:
      if (len<9 ) return 0;
      break;
    case MTP3ttc:
      //if ( mtp3_ni == 0 && len<5)
      //{ warn("MTP3ttc:len<5. Too short message!"); return 0; }
      //else
      if (len<6)
      { warn("MTP3ttc:len<6. Too short message!"); return 0; }
      break;
    case MTP3bttc:
      if (len<7)
      { warn("MTP3bttc:len<7. Too short message!"); return 0; }
      break;
    case MTP3mpt:
      if ( mtp3_ni == 2 && len<8)
      { warn("MTP3mpt:len<8. Too short message!"); return 0; }
      else if (len<5)
      { warn("MTP3mpt:len<5. Too short message!"); return 0; }
      break;
    default:
      warn("Unknown MTPServiceType!!!");
      break;
  }
  unsigned int sls,opc,dpc;
  GetPointCodes(sls,opc,dpc,inbuff);
  SetPointCodes(sls,dpc,opc,outbuff);
  return 1;
}
//------------

int  MTP3asp__PT_PROVIDER::Check_PcMatch(unsigned int opc, unsigned int dpc, unsigned char *buff)
{
  unsigned int temp_opc,temp_dpc,temp_sls;

  GetPointCodes(temp_sls,temp_opc,temp_dpc,buff);
  if ( (temp_opc == opc) && (temp_dpc == dpc) ) return 1;
  return 0;
}
//------------


// -------------------------------------------------
// M3UA Functions and definitions for test with SEA
// -------------------------------------------------

// Structures for M3UA
static unsigned char aspup_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  0x01, //Release Version
  0x00, //reserved
  M3UA_MSG_CLS_ASPSM, //Msg class: ASPSM
  M3UA_MSG_TYP_ASPSM_ASPUP, //Msg type: ASPUP
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x08  // length ends = 8 octets
  //      ,PAR_PREFIX_COMMON, //optional Info string tag, in included, then msg
  //       PAR_INFO_STR            //msg length should be +20, that is 28=0x1c
  //       0x00,0x0f, // length: "TTCN-3 Executor"  is 15 chars
  //       'T', 'T', 'C', 'N',
  //       '-', '3', ' ', 'E',
  //       'x', 'e', 'c', 'u',
  //       't', 'o', 'r', 0x00 //las octet is padding
};
const int sizeof_aspup_msg = 8;

static unsigned char aspupack_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  0x01, //Release Version
  0x00, //reserved
  M3UA_MSG_CLS_ASPSM, //Msg class: ASPSM
  M3UA_MSG_TYP_ASPSM_ASPUPAck, //Msg type: ASPUP
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x08  // length ends = 8 octets
};
const int sizeof_aspupack_msg = 8;

static unsigned char aspac_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  0x01, //Release Version: 01
  0x00, //reserved
  M3UA_MSG_CLS_ASPTM, //Msg class
  M3UA_MSG_TYP_ASPTM_ASPAC, //Msg type
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x08  // length ends = 8 octets
};
const int sizeof_aspac_msg = 8;

// ASP Active Acknowledge msg:
static unsigned char aspac_ack_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  0x01, //Release Version: 01
  0x00, //reserved
  M3UA_MSG_CLS_ASPTM, //Msg class
  M3UA_MSG_TYP_ASPTM_ASPACAck, //Msg type
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x08  // length ends = 8 octets
};
const int sizeof_aspac_ack_msg = 8;

static unsigned char aspia_ack_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  0x01, //Release Version: 01
  0x00, //reserved
  M3UA_MSG_CLS_ASPTM, //Msg class
  M3UA_MSG_TYP_ASPTM_ASPIAAck, //Msg type
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x08  // length ends = 8 octets
};
const int sizeof_aspia_ack_msg = 8;

// ASP DOWN Acknowledge msg:
static unsigned char aspdn_ack_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  0x01, //Release Version: 01
  0x00, //reserved
  M3UA_MSG_CLS_ASPSM, //Msg class
  M3UA_MSG_TYP_ASPSM_ASPDNAck, //Msg type
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x08  // length ends = 8 octets
};
const int sizeof_aspdn_ack_msg = 8;

//ASP Destination Available msg:
static unsigned char dava_1apc_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  //or updated doc 2/1056-FCPW 101 86/P-1
  0x01, //Release Version: 01
  0x00, //reserved
  M3UA_MSG_CLS_SSNM, //Msg class
  M3UA_MSG_TYP_SSNM_DAVA, //Msg type
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x10,  // length ends, 16 octets
  //Affected point code
  PAR_PREFIX_COMMON,
  PAR_AFFECTED_PC,
  0x00, //par length begins (2 octets)
  0x08, //length ends, 8 octets
  0x00, // point code placeholder begins
  0x00, //
  0x00, //
  0x00 // point code placeholder ends
};
const int sizeof_dava_1apc_msg = 16;

static unsigned char duna_1apc_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  0x01, //Release Version: 01
  0x00, //reserved
  M3UA_MSG_CLS_SSNM, //Msg class
  M3UA_MSG_TYP_SSNM_DUNA, //Msg type
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x10,  // length ends, 16 octets
  //Affected point code
  PAR_PREFIX_COMMON,
  PAR_AFFECTED_PC,
  0x00, //par length begins (2 octets)
  0x08, //length ends, 8 octets
  0x00, // point code placeholder begins
  0x00, //
  0x00, //
  0x00 // point code placeholder ends
};
const int sizeof_duna_1apc_msg = 16;

static unsigned char error_msg[] =
{
  //common MsUA msg hdr, see M3UA PS
  //Doc no. 1/1056-FCP 103 3571/F Uen, RevA
  0x01, //Release Version: 01
  0x00, //reserved
  M3UA_MSG_CLS_MGMT, //Msg class
  M3UA_MSG_TYP_MGMT_ERR, //Msg type
  0x00, //Msg length  begins (4 octets)
  0x00, //
  0x00, //
  0x18,  // length ends, 16+8 octets
  // Error code field:
  PAR_PREFIX_COMMON,
  PAR_ERROR_CODE,
  0x00, //par length begins (2 octets)
  0x08, //length ends, 8 octets
  0x00, // error code placeholder begins (M3UA_MSG_OFFS+4)
  0x00, //
  0x00, //
  0x00, // error code placeholder ends
  // additional field:
  0x00, // M3UA_MSG_OFFS+8
  0x00, // Routing context or Network Appearanance or APC or DI = error_code_id
  0x00, // par length begins (2 octets)
  0x08, // length ends, 8 octets
  0x00, // value placeholder begins M3UA_MSG_OFFS+12
  0x00, //
  0x00, //
  0x00 // value place holder ends
};
const int sizeof_error_msg = 24;

void MTP3asp__PT_PROVIDER::M3UA_user_unmap(const char *system_port)
{
  MTP3_close_connection();
  dynamicConnection = FALSE;
}
//------------

void MTP3asp__PT_PROVIDER::M3UA_user_map(const char *system_port)
{
  M3UAState = AssocDown;
  Check_TestPort_Variables();
  if(dynamicConnection)
  {
    connectionUp = FALSE;
  }
  else
  {
    MTP3_open_channel(TRUE);
    M3UA_user_connect();
  }
}
//------------

void MTP3asp__PT_PROVIDER::M3UA_user_connect()
{
    M3UAState = AssocEstabl;
    // Sending out an ASPUP message
    log("Message ASPUP will be sent");
    send_msg(aspup_msg, sizeof_aspup_msg);
    // NOTE: the ASPUPAck will be handled by M3UA_interpreter, which
    // also will take care of sending ASPAC upon receiving the ASPUPAck
}
//------------

// M3UA_interpreter
void MTP3asp__PT_PROVIDER::M3UA_interpreter(unsigned char* inbuffer,int length,int from_channel,CONNECTION* con)
{
  if ((length==0) || (inbuffer==NULL))
  { warn("0 byte long message received -> packet dropped.");
    return;
  }

  if (length==1)
  {
    log("1 byte long internal SEA message received -> packet dropped.");
    return;
  }

  if ( !strcmp((const char*)inbuffer,"start") )
  { log("start message received from SEA");
    return;
  }
  else if (!strcmp((const char*)inbuffer,"stop"))
  { log("stop message received from SEA");
    return;
  }

  // writing out the contents of the buffer
  OCTETSTRING buff(length,inbuffer);
  TTCN_Logger::begin_event(TTCN_PORTEVENT);
  TTCN_Logger::log_event("incoming buffer: ");
  buff.log();
  TTCN_Logger::end_event();

  // version checking
  if (  inbuffer[M3UA_VER_OFFS]  != M3UA_version )
  { warn("Incompatible M3UA protocol version in header -> packet dropped");
    return;
  }

  //length checking
  unsigned int indicated_length = decode_32b_int(inbuffer + M3UA_LGT_OFFS );
  if ( indicated_length  != (unsigned)length)
    warn("Length in common header (%d) mismatches received  buffer length (%d),"
         "Assuming that it is because of the omission of final parameter padding"
         "in indicated length",indicated_length, length);

  // checking MSG class
  int unprocessed_chars = 0;
  switch (inbuffer[M3UA_CLS_OFFS])
  {
    case M3UA_MSG_CLS_MGMT:
      unprocessed_chars =  processing_M3UA_MGMT_msg(inbuffer, length);
      break;
    case M3UA_MSG_CLS_TRNSFM :
      unprocessed_chars =  processing_M3UA_Transfer_msg(inbuffer, length);
      break;
    case M3UA_MSG_CLS_SSNM :
      unprocessed_chars = processing_M3UA_SSNM_msg(inbuffer, length);
      break;
    case M3UA_MSG_CLS_ASPSM :
      unprocessed_chars = processing_M3UA_ASPSM_msg(inbuffer, length);
      break;
    case M3UA_MSG_CLS_ASPTM :
      unprocessed_chars = processing_M3UA_ASPTM_msg(inbuffer, length);
      break;
    case M3UA_MSG_CLS_RKM :
      unprocessed_chars = processing_M3UA_RKM_msg(inbuffer, length);
      break;
    default:
      unprocessed_chars = processing_M3UA_unsupported_msg_class(inbuffer, length);
      break;
  }
  debuglog("%d chars remained unprocessed (might be due to padding)",  unprocessed_chars);
}
//------------

//processing M3UA MGMT msg
int MTP3asp__PT_PROVIDER::processing_M3UA_MGMT_msg(unsigned char* inbuffer,int length)
{
  debuglog("Entering function:processing_MGMT_msg");
  int offset = M3UA_MSG_OFFS; //pointer for processing

  TTCN_Logger::begin_event(TTCN_DEBUG);
  TTCN_Logger::log_event("MTP3 Test Port (%s): {", get_name());
  TTCN_Logger::log_event("decoded msg class: Mgmt, ");
  switch (inbuffer[M3UA_TYP_OFFS])//msg type
  {
    case M3UA_MSG_TYP_MGMT_ERR:
      TTCN_Logger::log_event("type: ERROR-> ignored");
      TTCN_Logger::log_event("}");
      TTCN_Logger::end_event();
      break;
    case M3UA_MSG_TYP_MGMT_NTFY:
      TTCN_Logger::log_event("type: NOTIFY -> ignored");
      TTCN_Logger::log_event("}");
      TTCN_Logger::end_event();
      break;
    default:
      send_M3UA_error_msg( PAR_ERRC_UNSMT,  inbuffer[M3UA_TYP_OFFS]);
      TTCN_Logger::log_event("Unsupported M3UA msg type %x of class MGMT -> packet dropped.", inbuffer[M3UA_TYP_OFFS]);
      TTCN_Logger::log_event("}");
      TTCN_Logger::end_event();
      break;
  }
  return length - offset;
}
//------------

//processing M3UA SSNM msg
int MTP3asp__PT_PROVIDER::processing_M3UA_SSNM_msg(unsigned char* inbuffer,int length)
{
  debuglog("Entering function:processing_SSNM_msg");
  TTCN_Logger::begin_event(TTCN_PORTEVENT);
  TTCN_Logger::log_event("MTP3 Test Port (%s): {", get_name());
  TTCN_Logger::log_event("decoded msg class: SSNM, ");
  int offset = M3UA_MSG_OFFS; //pointer for processing

  switch (inbuffer[M3UA_TYP_OFFS])
  {
    case M3UA_MSG_TYP_SSNM_DAUD:
      while (offset <= length-8 ) //processing potential params
      {
        switch (inbuffer[offset++]) //1st octet of tag
        {
        case PAR_PREFIX_COMMON:
          TTCN_Logger::log_event (" DAUD: COMMON parameter ");
          switch (inbuffer[offset++]) //2nd octet of COMMON tag
          {
            case PAR_ROUTING_CTX:
              TTCN_Logger::log_event ("Routing Context (unsupported par) -> skipped), ");
              skip_par_after_tag(inbuffer, offset);
              break;
            case PAR_INFO_STR:
              TTCN_Logger::log_event ("Info String (unsupported par) -> skipped), ");
              skip_par_after_tag(inbuffer, offset);
              break;
            case PAR_AFFECTED_PC:
              TTCN_Logger::log_event ("Affected Point Code  -> will send DUNA/DAVA, ");
              TTCN_Logger::log_event("will ignore remainder parameters after APC}");
              TTCN_Logger::end_event();
              Send_DAVA_DUNA_to_APCinDAUD(Tester_Pc, inbuffer, offset);
              return length-offset;
            default:
              TTCN_Logger::log_event ("invalid COMMON param tag:0x%02x%02x-> skipped", PAR_PREFIX_COMMON, inbuffer[offset-1]);
              send_M3UA_error_msg( PAR_ERRC_PARFE, inbuffer[offset-1]);
              skip_par_after_tag(inbuffer, offset);
              break;
          }
          break;
        case PAR_PREFIX_M3UA:
          TTCN_Logger::log_event ("DAUD: M3UA parameter: ");
          switch (inbuffer[offset++]) //2nd octet of M3UA tag
          {
            case PAR_NETW_APP:
              TTCN_Logger::log_event ("Network Appearance (unsupported par) -> skipped), ");
              skip_par_after_tag(inbuffer, offset);
              break;
            default:
              TTCN_Logger::log_event ("invalid M3UA  param tag:0x%02x%02x-> skipped",
              PAR_PREFIX_M3UA, inbuffer[offset-1]);
              send_M3UA_error_msg( PAR_ERRC_PARFE, inbuffer[offset-1]);
              skip_par_after_tag(inbuffer, offset);
          }
          break;
        default: //1st octet of tag
          TTCN_Logger::log_event ("invalid 1st octet param tag:0x%02x in DATA (packet dropped)",inbuffer[offset-1]);
          close_log_event();
          return length -offset;
        break;
        }
      }
      break;
    case M3UA_MSG_TYP_SSNM_DAVA: // Destination Available // Notification to the user part ?????
      TTCN_Logger::log_event("type: SSNM_DAVA -> ignored");
      close_log_event();
      break;
    case M3UA_MSG_TYP_SSNM_DUNA: // Destination Unavailable // Notification to the user part ?????
      TTCN_Logger::log_event("type: SSNM_DUNA -> ignored");
      close_log_event();
      break;
    case M3UA_MSG_TYP_SSNM_SCON: // Signalling Congestion  // Notification to the user part ?????
      TTCN_Logger::log_event("type: SSNM_SCON -> ignored");
      close_log_event();
      break;
    case M3UA_MSG_TYP_SSNM_DUPU: //Destinationn User Part Unavailable // Notification to the user part ?????
      TTCN_Logger::log_event("type: SSNM_DUPU -> ignored");
      close_log_event();
      break;
    case M3UA_MSG_TYP_SSNM_DRST: //Destination Restricted   // Notification to the user part ?????
      TTCN_Logger::log_event("type: SSNM_DRST -> ignored");
      close_log_event();
      break;
    default: //msg type
      send_M3UA_error_msg( PAR_ERRC_UNSMT,  inbuffer[M3UA_TYP_OFFS]);
      TTCN_Logger::log_event("Unsupported M3UA msg type -> packet dropped.");
      close_log_event();
      break;
  }
  return length - offset;
}
//------------

//processing M3UA ASPSM msg
int MTP3asp__PT_PROVIDER::processing_M3UA_ASPSM_msg(unsigned char* inbuffer,int length)
{
  debuglog("Entering function:processing_ASPSM_msg");
  TTCN_Logger::begin_event(TTCN_PORTEVENT);
  TTCN_Logger::log_event("MTP3 Test Port (%s): {", get_name());
  TTCN_Logger::log_event("decoded msg class: ASPSM, ");

  int offset = M3UA_MSG_OFFS; //pointer for processing
  switch (inbuffer[M3UA_TYP_OFFS])
  {
    case M3UA_MSG_TYP_ASPSM_ASPUP:
      TTCN_Logger::log_event("type: ASPSM_ASPUP -> ASPUPAck will be sent");
      close_log_event();
      send_msg(aspupack_msg, sizeof_aspupack_msg);
      break;
    case M3UA_MSG_TYP_ASPSM_ASPDN:
      TTCN_Logger::log_event("type: ASPSM_ASPDN -> ASPDNAck will be sent");
      close_log_event();
      send_msg(aspdn_ack_msg, sizeof_aspdn_ack_msg);
      break;
    case M3UA_MSG_TYP_ASPSM_BEAT:
      TTCN_Logger::log_event("type: ASPSM_BEAT -> ASPSM_BEATAck will be sent");
      close_log_event();
      //Sending back the packet as acknowledge:
      inbuffer[M3UA_TYP_OFFS]= M3UA_MSG_TYP_ASPSM_BEATAck;
      send_msg(inbuffer, length);
      break;
    case M3UA_MSG_TYP_ASPSM_ASPUPAck:
      TTCN_Logger::log_event("type: ASPSM_ASPUPAck -> ASPAC will be sent");
      M3UAState = AssocInac;
      TTCN_Logger::log_event(" M3UAState's been changed to AssocInac");
      close_log_event();
      send_msg(aspac_msg, sizeof_aspac_msg );
      break;
    case M3UA_MSG_TYP_ASPSM_ASPDNAck:
      M3UAState = AssocDown;
      TTCN_Logger::log_event(" M3UAState's been changed to AssocDown");
      TTCN_Logger::log_event("type: ASPSM_ASPDNAck ->  nothing will be sent");
      close_log_event();
      break;
    case M3UA_MSG_TYP_ASPSM_BEATAck:
      TTCN_Logger::log_event("type: ASPSM_BEATAck ->  nothing will be sent");
      close_log_event();
      break;
    default:
      TTCN_Logger::log_event("Unsupported M3UA msg type %x of class ASPSM-> packet dropped.", (unsigned char)(inbuffer[M3UA_TYP_OFFS]));
      close_log_event();
      send_M3UA_error_msg( PAR_ERRC_UNSMT,  inbuffer[M3UA_TYP_OFFS]);
      break;
  }
  return length - offset;
}
//------------

//processing M3UA ASPTM msg
int MTP3asp__PT_PROVIDER::processing_M3UA_ASPTM_msg(unsigned char* inbuffer,int length)
{
  debuglog("Entering function:processing_ASPTM_msg");
  TTCN_Logger::begin_event(TTCN_PORTEVENT);
  TTCN_Logger::log_event("MTP3 Test Port (%s): {", get_name());
  TTCN_Logger::log_event("decoded msg class: ASPTM, ");

  int offset = M3UA_MSG_OFFS; //pointer for processing
  switch (inbuffer[M3UA_TYP_OFFS])
  {
    case M3UA_MSG_TYP_ASPTM_ASPAC: //ASP Active
      M3UAState = AssocActive;
      TTCN_Logger::log_event("type: ASPTM_ASPAC -> ASPACAck will be sent");
      close_log_event();
      send_msg(aspac_ack_msg, sizeof_aspac_ack_msg);
      break;
    case M3UA_MSG_TYP_ASPTM_ASPIA:  //ASP InActive
      TTCN_Logger::log_event("type: ASPTM_ASPIA -> ASPIAAck will be sent");
      close_log_event();
      send_msg(aspia_ack_msg, sizeof_aspia_ack_msg);
      break;
    case M3UA_MSG_TYP_ASPTM_ASPACAck:
      M3UAState = AssocActive;
      TTCN_Logger::log_event("type: ASPTM_ASPACAck -> nothing will be sent");
      TTCN_Logger::log_event("M3UAState's been changed to AssocActive.");
      close_log_event();
      break;
    case M3UA_MSG_TYP_ASPTM_ASPIAAck:
      TTCN_Logger::log_event("type: ASPTM_ASPIAAck -> nothing will be sent");
      close_log_event();
      break;
    default:
      TTCN_Logger::log_event("Unsupported M3UA msg type %x of class ASPSM-> packet dropped.", (unsigned char)(inbuffer[M3UA_TYP_OFFS]));
      close_log_event();
      send_M3UA_error_msg( PAR_ERRC_UNSMT,  inbuffer[M3UA_TYP_OFFS]);
      break;
  }
  return length - offset;
}
//------------

//processing M3UA RKM msg
int MTP3asp__PT_PROVIDER::processing_M3UA_RKM_msg(unsigned char* inbuffer,int length)
{
  debuglog("Entering function:processing_RKM_msg");
  int offset = M3UA_MSG_OFFS; //pointer for processing
  warn("Unsupported M3UA msg class M3UA_RKM -> packet dropped.");
  return length - offset;
}
//------------

//processing M3UA unsupported msg class
int MTP3asp__PT_PROVIDER::processing_M3UA_unsupported_msg_class(unsigned char* inbuffer,int length)
{
  warn("Unsupported M3UA msg class -> packet dropped.");
  send_M3UA_error_msg( PAR_ERRC_UNSMC, inbuffer[M3UA_CLS_OFFS] );
  int offset = M3UA_MSG_OFFS; //pointer for processing
  return length - offset;
}
//------------

// processing_M3UA_Transfer_msg  -  called if the msg class is "Transfer" i.e M3UA_MSG_CLS_TRNSFM:
int MTP3asp__PT_PROVIDER::processing_M3UA_Transfer_msg(unsigned char* inbuffer,int length)
{
  debuglog("Entering function:processing_M3UA_Transfer_msg");
  int offset = M3UA_MSG_OFFS; //pointer for processing
  unsigned int recv_opc, recv_dpc,recv_si, recv_ni, recv_mp,recv_sls =0;
  int param_length = 0;
  TTCN_Logger::begin_event(TTCN_DEBUG);
  TTCN_Logger::log_event("MTP3 Test Port (%s): {", get_name());
  TTCN_Logger::log_event("decoded msg class: DataTrnsf, ");

  switch (inbuffer[M3UA_TYP_OFFS]) //msg type
  {
    case M3UA_MSG_TYP_TRSNFM_DATA:
      TTCN_Logger::log_event("msg type DATA, ");
      while (offset <= length-8) //processing potential params
      {
        switch (inbuffer[offset++]) //1st octet of tag. Offset already incremented after the 'case' !
        {
          case PAR_PREFIX_COMMON:
            TTCN_Logger::log_event (" DATA: COMMON parameter, ");
            switch (inbuffer[offset++]) //2nd octet of tag
            {
              case PAR_ROUTING_CTX:
                TTCN_Logger::log_event ("Routing Context (unsupported par) -> skipped), ");
                //  Send back an error msg
                skip_par_after_tag(inbuffer, offset);
                break;
              case PAR_CORREL_ID:
                TTCN_Logger::log_event ("Correlation ID (unsupported par) -> skipped), ");
                skip_par_after_tag(inbuffer, offset);
                break;
              default:
                TTCN_Logger::log_event ("invalid COMMON param tag:0x%02x%02x -> skipped", PAR_PREFIX_COMMON, inbuffer[offset-1]);
                skip_par_after_tag(inbuffer, offset);
                break;
            }
          case PAR_PREFIX_M3UA:
            TTCN_Logger::log_event ("DATA: M3UA parameter: ");
            switch (inbuffer[offset++]) //2nd octet of M3UA tag
            {
              case PAR_PROT_DATA:
                //---------------------------------------------
                TTCN_Logger::log_event ("Protocol Data");
                // retrieving length
                param_length = decode_16b_int(inbuffer+offset); offset+=2;
                TTCN_Logger::log_event(", DATA: Length of Protocol Data parameter is %d",param_length);
                //  retrieving M3UA protocol data paremeter opc, dpc, si, ni, mp,
                //  sls
                recv_opc = decode_32b_int(inbuffer+offset); offset +=4;
                recv_dpc = decode_32b_int(inbuffer+offset); offset +=4;
                recv_si = (unsigned int)(inbuffer[offset++]);
                recv_ni = (unsigned int)(inbuffer[offset++]);
                recv_mp = (unsigned int)(inbuffer[offset++]);
                recv_sls = (unsigned int)(inbuffer[offset++]);
                TTCN_Logger::log_event(", DATA: decoded Protocol Data parameter:");
                // filling up TTCN structure
                if ((Loop==MTP3_ON) ||
                    (!Filter) ||
                    (Filter &&
                     (recv_opc == (unsigned)Sut_Pc) && (recv_dpc == (unsigned)Tester_Pc) &&
                     (recv_ni == (unsigned)mtp3_ni)
                   ))
                {
                  ASP__MTP3__TRANSFERind recv_msg;
                  MTP3__Field__sio recv_sio;
                  recv_sio.ni()= int2bit(recv_ni,2);
                  recv_sio.prio()= int2bit(recv_mp,2);
                  recv_sio.si()= int2bit(recv_si,4);
                  recv_msg.sio() = recv_sio;
                  recv_msg.sls() = recv_sls;
                  recv_msg.opc()= recv_opc;
                  recv_msg.dpc() = recv_dpc;;
                  recv_msg.data() = OCTETSTRING(param_length-16, // 16 octet paramheader + 5 routing label
                  &inbuffer[offset]);
                  recv_msg.log();
                  close_log_event();
                  incoming_message ( recv_msg );
                  offset += param_length-16;
                  return  length - offset;
                }
                else
                {
                  close_log_event();
                  log("Either the received M3UA(OPC, DPC, SI) fields, or the embedded MTP3 rooting label (OPC, DPC) not matched with the filter setting -> packet dropped.");
                  return  length - offset;
                }
                break;
              default:
                TTCN_Logger::log_event ("invalid M3UA  param tag:0x%02x%02x-> skipped", PAR_PREFIX_M3UA, inbuffer[offset-1]);
                skip_par_after_tag(inbuffer, offset);
            }
            break;
          default:
            TTCN_Logger::log_event ("invalid 1st octet param tag:0x%02x in DATA (packet dropped)",inbuffer[offset-1]);
            close_log_event();
            return length -offset;
            break;
        }
      }// Checking parameter tag (offset <length-8)
      break;
    default:
      TTCN_Logger::log_event("Invalid M3UA msg class TransferMessage msg type %d -> packet dropped",
                             (unsigned int)(inbuffer[M3UA_TYP_OFFS]));
      close_log_event();
      send_M3UA_error_msg( PAR_ERRC_UNSMT,  inbuffer[M3UA_TYP_OFFS]);
      break;
  }

  return length -offset;
}

// Set M3UA SingleAPC -  Stores field Single Affected Point Code
void MTP3asp__PT_PROVIDER::Set_M3UA_SingleAPC(unsigned int pc, unsigned char* apc_par)
{
  //setting par type to APC
  apc_par[0] = PAR_PREFIX_COMMON;
  apc_par[1] = PAR_AFFECTED_PC;
  //setting the length to 4+4 = 8
  apc_par[2] = 0x00;
  apc_par[3] = 0x08;
  //setting the mask
  apc_par[4] = 0x00;
  //setting the pc
  encode_24b_int(apc_par+5,pc);
}

// Send DAVA DUNA to APCinDAUD
void MTP3asp__PT_PROVIDER::Send_DAVA_DUNA_to_APCinDAUD(unsigned int dava_sep, unsigned char* inbuffer, int & offset)
{
  unsigned int length = decode_16b_int(inbuffer+offset);
  offset +=2;
  unsigned int current_pc;

  //checking the length
  if ((length < 8) || (length%4))
  { warn("Invalid length in APC parameter -> not processed");
    return;
  }

  for (unsigned int i = 4  ; i < length; i += 4)
  {
    if (inbuffer[offset++] == 0x00)  // mask===>single pc
    {
      current_pc = decode_24b_int(inbuffer+offset);
      offset +=3;
      if (dava_sep == current_pc) //dava
      { Set_M3UA_SingleAPC(dava_sep, dava_1apc_msg+M3UA_MSG_OFFS);
        log("DAVA will be sent for pc=%d", dava_sep);
        send_msg(dava_1apc_msg, sizeof_dava_1apc_msg);
      }
      else //duna
      { Set_M3UA_SingleAPC(current_pc, duna_1apc_msg+M3UA_MSG_OFFS);
        log("DUNA will be sent for pc=%d", current_pc);
        send_msg(duna_1apc_msg, sizeof_duna_1apc_msg);
      }
    }
    else //masked pc
    { warn("Unsupported  masking (mask=0x%02x) for PC=%d in APC parameter -> ignored",
      inbuffer[offset-1], current_pc);
    }
  }
  return;
}

// send M3UA error msg
// send an eror msg with error_code. Additional parameter matches to the error code:
void MTP3asp__PT_PROVIDER::send_M3UA_error_msg(unsigned int error_code, unsigned int add_par )
{
  encode_16b_int( error_msg+M3UA_MSG_OFFS+4, error_code);
  encode_16b_int( error_msg+M3UA_MSG_OFFS+8, add_par);
  send_msg(error_msg, sizeof_error_msg );
};

// Coder functions for M3UA:  int -> unsigned char array
//------------------------------------------------------
// Result:Less significant byte in highest address
//        Most Significant Byte first (in lowest address) = MSB = Big Endian = Network Byte Order
void MTP3asp__PT_PROVIDER::encode_32b_int(unsigned char *to, unsigned int from)
{
  to[3] = from & 0xFF;
  from >>= 8;
  to[2] = from & 0xFF;
  from >>= 8;
  to[1] = from & 0xFF;
  from >>= 8;
  to[0] = from & 0xFF;
}
//------------

unsigned int MTP3asp__PT_PROVIDER::decode_32b_int(const unsigned char *from)
{
  return from[3] | (from[2] << 8) | (from[1] << 16) | (from[0] << 24);
}
//------------

void MTP3asp__PT_PROVIDER::encode_24b_int(unsigned char *to, unsigned int from)
{
  to[2] = from & 0xFF;
  from >>= 8;
  to[1] = from & 0xFF;
  from >>= 8;
  to[0] = from & 0xFF;
}
//------------

unsigned int MTP3asp__PT_PROVIDER::decode_24b_int(const unsigned char *from)
{
  return from[2] | (from[1] << 8) | (from[0] << 16);
}
//------------

void MTP3asp__PT_PROVIDER::encode_16b_int(unsigned char *to, int from)
{
  to[1] = from & 0xFF;
  from >>= 8;
  to[0] = from & 0xFF;
}
//------------

unsigned int MTP3asp__PT_PROVIDER::decode_16b_int(const unsigned char *from)
{
  return from[1] | (from[0] << 8);
}
//------------

//skip par after tag
void MTP3asp__PT_PROVIDER::skip_par_after_tag(unsigned char* inbuffer, int &offset)
{
  offset += decode_16b_int(inbuffer+offset)-2; //the length contains
                                               //the param hdr. itself
  if (offset%4) offset += 4-(offset%4); //skipping padding
}
//------------
#endif



#ifdef TARGET_TEST
// --------------------------
// Functions for Target testing
// --------------------------
// In case of target this function handles the received message
void MTP3asp__PT_PROVIDER::message_incoming(const unsigned char* msg, int messageLength, int)
{
  OCTETSTRING rcvData = OCTETSTRING(messageLength, msg);

  int msgType = oct2int(substr(rcvData,0,1));
  switch (msgType)
  {
    case 0: //TRANSFERind message received
      if(Tcp_is_up) //Registration was already performed
      {
        ASP__MTP3__TRANSFERind recv_msg;
        MTP3__Field__sio recv_sio;
        BITSTRING sio_bit = oct2bit(substr(rcvData,5,1));
        recv_sio.ni()= substr(sio_bit,0,2);
        recv_sio.prio()= substr(sio_bit,2,2);
        recv_sio.si()= substr(sio_bit,4,4);
        recv_msg.sio() = recv_sio;
        recv_msg.opc() = oct2int(substr(rcvData,6,4));
        recv_msg.dpc() = oct2int(substr(rcvData,10,4));
        recv_msg.sls() = oct2int(substr(rcvData,14,1));
        recv_msg.data() = substr(rcvData,15,rcvData.lengthof()-15);
        if (Tcp_is_up == 1) //No unregistration ongoing
          incoming_message(recv_msg);
        else                //Unregistration ongoing
          log("Received ASP_MTP3_TRANSFERind is ignored since unregistration is started.");
      }
      else
        error("Message was received before successful registration in M3UA server.");
      break;

    case 4: //Status message received
      {
        int status = oct2int(substr(rcvData,5,1));
        if(Tcp_is_up == 2) // Unregistration ongoing
        {
          const char * rcvDat = oct2str(rcvData);
          log("Message \"%s\" received. Status = %i", rcvDat, status);
          if (status == 2)
          {
            log("Unregistration performed.");
            Tcp_is_up = 0;
          }
          else if (status == 3)
            error("Unsuccessful unregistration.");
          else if (status == 5) {
            if(forward_status) {
              incoming_message(ASP__MTP3__STATUS(NULL_VALUE));
            }
            else {
              warn("Invalid STATUS message received from M3UA server with status code=%d.", status);
            }
          }
        }
        else if(Tcp_is_up == 1) // Active state
        {
          if (status == 5) {
            if(forward_status) {
              incoming_message(ASP__MTP3__STATUS(NULL_VALUE));
            } 
          }
          else {
            warn("Invalid STATUS message received from M3UA server with status code=%d.", status);
          }
        }
        else // Registration ongoing
        {
          const char * rcvDat = oct2str(rcvData);
          log("Message \"%s\" received. Status = %i", rcvDat, status);
          if (status == 0)
          {
            log("Registration performed.");
            Tcp_is_up = 1;
          }
          else {
            error("Unsuccessful registration.");
          }
        }
      }
      break;
    case 5:
      {
        if(forward_pause) incoming_message(ASP__MTP3__PAUSE(NULL_VALUE));
      }
      break;
    case 6:
      {
        if(forward_resume) incoming_message(ASP__MTP3__RESUME(NULL_VALUE));
      }
      break;


    default: //Unexpected message received
      warn("Invalid message received from M3UA server.");
  }
}

void  MTP3asp__PT_PROVIDER::Check_Target_TestPort_Variables()
{
  if (Sut_Pc==-1) error("Parameter SUT_Pc is not set.");
  if (Tester_Pc==-1) error("Parameter TESTER_Pc is not set.");
  if (!Ni_is_set) error("Parameter NI is not set.");
  if (M3UA_version==0) error("Parameter M3UA_version cannot be set to 0 in TargetM3UA mode.");
 
//packet header
  header_descr = new PacketHeaderDescr( 1, 4, PacketHeaderDescr::Header_MSB);
}

void  MTP3asp__PT_PROVIDER::Check_Target_TestPort_Variables_STC()
{
  if( destinationname == NULL) error("Parameter DestinationName is not set in TargetSTC mode.");
  
  header_descr = new PacketHeaderDescr( 1, 4, PacketHeaderDescr::Header_MSB);
}

void MTP3asp__PT_PROVIDER::Target_user_map(const char *system_port)
{
  Tcp_is_up = 0;
  Check_Target_TestPort_Variables();
  map_user();

  OCTETSTRING tcpData = int2oct(2,1); //Message type
  if( destinationname == NULL)
    tcpData = tcpData + int2oct(char2oct(system_port).lengthof()+15,4); //Length
  else
    tcpData = tcpData + int2oct(char2oct(destinationname).lengthof()+15,4);
  tcpData = tcpData + int2oct(mtp3_ni,1);
  tcpData = tcpData + int2oct(Sut_Pc,4);
  tcpData = tcpData + int2oct(Tester_Pc,4);
  tcpData = tcpData + int2oct(M3UA_version,1);
  if( destinationname == NULL)
    tcpData = tcpData + char2oct(system_port);
  else
    tcpData = tcpData + char2oct(destinationname);

  send_outgoing((const unsigned char*)tcpData,tcpData.lengthof());

  TTCN_Logger::begin_event(TTCN_DEBUG);
  TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
  TTCN_Logger::log_event_str("Registration message sent: ");
  tcpData.log();
  TTCN_Logger::end_event();

  int fd = get_socket_fd();
  pollfd pollFd = { fd, POLLIN, 0 };
  int nEvents = poll(&pollFd, 1, 3000 /* ms */);
  if (nEvents == 0)
    error("No response received for REGISTER message. Exiting after timeout.");
  if (nEvents < 0 || (pollFd.revents & (POLLIN | POLLHUP)) == 0)
    error("No response received for REGISTER message. Exiting after error (%d)",
      (nEvents < 0) ? errno : 0);
  Handle_Fd_Event(fd, TRUE, FALSE, FALSE);
}

void MTP3asp__PT_PROVIDER::TargetSTC_user_map(const char *system_port)
{
  Tcp_is_up = 0;
  Check_Target_TestPort_Variables_STC();
  map_user();

  OCTETSTRING tcpData = int2oct(2,1); //Message type
  tcpData = tcpData + int2oct(char2oct(destinationname).lengthof()+15,4);
  tcpData = tcpData + int2oct(0,1);
  tcpData = tcpData + int2oct(0,4);
  tcpData = tcpData + int2oct(0,4);
  tcpData = tcpData + int2oct(0,1);
  tcpData = tcpData + char2oct(destinationname);

  send_outgoing((const unsigned char*)tcpData,tcpData.lengthof());

  TTCN_Logger::begin_event(TTCN_DEBUG);
  TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
  TTCN_Logger::log_event_str("Registration message sent: ");
  tcpData.log();
  TTCN_Logger::end_event();

  int fd = get_socket_fd();
  pollfd pollFd = { fd, POLLIN, 0 };
  int nEvents = poll(&pollFd, 1, 3000 /* ms */);
  if (nEvents == 0)
    error("No response received for REGISTER message. Exiting after timeout.");
  if (nEvents < 0 || (pollFd.revents & (POLLIN | POLLHUP)) == 0)
    error("No response received for REGISTER message. Exiting after error (%d)",
      (nEvents < 0) ? errno : 0);
  Handle_Fd_Event(fd, TRUE, FALSE, FALSE);
}

void MTP3asp__PT_PROVIDER::Target_user_unmap(const char *system_port)
{
  OCTETSTRING tcpData = int2oct(3,1); //Message type
  tcpData = tcpData + int2oct(6,4); //Length
  tcpData = tcpData + int2oct(0,1);
  send_outgoing((const unsigned char*)tcpData,tcpData.lengthof());

  TTCN_Logger::begin_event(TTCN_DEBUG);
  TTCN_Logger::log_event("MTP3 Test Port (%s): ", get_name());
  TTCN_Logger::log_event_str("Unregistration message sent: ");
  tcpData.log();
  TTCN_Logger::end_event();

  Tcp_is_up = 2; //Unregistration ongoing

  while (Tcp_is_up == 2)
  {
    int fd = get_socket_fd();
    pollfd pollFd = { fd, POLLIN, 0 };
    int nEvents = poll(&pollFd, 1, 3000 /* ms */);
    if (nEvents == 0)
      error("No response received for UNREGISTER message. Exiting after timeout.");
    if (nEvents < 0 || (pollFd.revents & (POLLIN | POLLHUP)) == 0)
      error("No response received for UNREGISTER message. Exiting after error (%d)",
        (nEvents < 0) ? errno : 0);
    Handle_Fd_Event(fd, TRUE, FALSE, FALSE);
  }
  unmap_user();
}
#endif
}
