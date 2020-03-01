#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
FILES+=" TCCEncoding_Functions.ttcn TCCEncoding.cc " # GSM 7-bit coding
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

# Required by MGCP and IPA
DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

#DIR=$BASEDIR/titan.ProtocolModules.GTPv2_v13.7.0/src
#FILES="GTPv2_Types.ttcn"
#gen_links $DIR $FILES

#DIR=$BASEDIR/titan.ProtocolModules.GTP_v13.5.0/src
#FILES="GTPC_EncDec.cc   GTPC_Types.ttcn  GTPU_EncDec.cc   GTPU_Types.ttcn"
#gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.DIAMETER_ProtocolModule_Generator/src
FILES="DIAMETER_EncDec.cc"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="DNS_Helpers.ttcn "
FILES+="DIAMETER_Types.ttcn DIAMETER_CodecPort.ttcn DIAMETER_CodecPort_CtrlFunct.ttcn DIAMETER_CodecPort_CtrlFunctDef.cc DIAMETER_Emulation.ttcn DIAMETER_Templates.ttcn "
gen_links $DIR $FILES

ignore_pp_results
