#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

#DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
#FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
#gen_links $DIR $FILES

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
gen_links $DIR $FILES

# Required by DIAMETER
DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

#DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
#FILES="Socket_API_Definitions.ttcn"
#gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.PIPEasp/src
FILES="PIPEasp_PT.cc PIPEasp_PT.hh PIPEasp_Types.ttcn PIPEasp_PortType.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.JSON_v07_2006/src
FILES="JSON_Generic.ttcn JSON_Generic_Null_Def.asn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.5G_system_TS29571_CommonData_v15/src
FILES="TS29571_CommonData.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.5G_system_TS29503_Nudm_v15/src
FILES="TS29503_Nudm_UEAU.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.DIAMETER_ProtocolModule_Generator/src
FILES="DIAMETER_EncDec.cc"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="DIAMETER_Types.ttcn DIAMETER_CodecPort.ttcn DIAMETER_CodecPort_CtrlFunct.ttcn DIAMETER_CodecPort_CtrlFunctDef.cc DIAMETER_Emulation.ttcn "
FILES+="DIAMETER_Templates.ttcn DIAMETER_ts29_272_Templates.ttcn "
FILES+="SCTP_Templates.ttcn "
gen_links $DIR $FILES

ignore_pp_results
