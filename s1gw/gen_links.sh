#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
FILES+=" TCCEncoding_Functions.ttcn TCCEncoding.cc "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.PFCP_v15.1.0/src
FILES="PFCP_Types.ttcn"
gen_links $DIR $FILES

DIR=../library/s1ap
FILES="S1AP_CommonDataTypes.asn S1AP_Constants.asn S1AP_Containers.asn S1AP_IEs.asn "
FILES+="S1AP_PDU_Contents.asn S1AP_PDU_Descriptions.asn "
FILES+="S1AP_EncDec.cc S1AP_Types.ttcn S1AP_Templates.ttcn "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc IPCP_Types.ttcn "
FILES+="PFCP_CodecPort.ttcn PFCP_CodecPort_CtrlFunct.ttcn PFCP_CodecPort_CtrlFunctDef.cc PFCP_Emulation.ttcn PFCP_Templates.ttcn "
FILES+="S1AP_CodecPort.ttcn S1AP_CodecPort_CtrlFunctDef.cc S1AP_CodecPort_CtrlFunct.ttcn "
FILES+="SCTP_Templates.ttcn "
gen_links $DIR $FILES

ignore_pp_results
