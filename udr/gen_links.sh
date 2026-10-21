#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

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

# Needed by HTTP_Adapter (Prometheus_Checker.ttcn metrics):
DIR=$BASEDIR/titan.TestPorts.Common_Components.Abstract_Socket/src
FILES="Abstract_Socket.cc Abstract_Socket.hh "
gen_links $DIR $FILES

# Needed by HTTP_Adapter (Prometheus_Checker.ttcn metrics):
DIR=$BASEDIR/titan.TestPorts.HTTPmsg/src
FILES="HTTPmsg_MessageLen.ttcn HTTPmsg_MessageLen_Function.cc HTTPmsg_PT.cc HTTPmsg_PT.hh HTTPmsg_PortType.ttcn HTTPmsg_Types.ttcn "
gen_links $DIR $FILES

# Needed by HTTP2_Adapter:
DIR=$BASEDIR/titan.ProtocolModules.HTTP2/src
FILES="HTTP2_EncDec.cc HTTP2_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.JSON_v07_2006/src
FILES="JSON_Generic_Null_Def.asn JSON_Generic.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/osmo-ttcn3-openapi-generator/openapi-specs/3GPP_5GC_Rel19/ttcn3
FILES="TS29571_CommonData.ttcn TS29505_Subscription_Data.ttcn "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc IPCP_Types.ttcn IPCP_Templates.ttcn "
FILES+="SCTP_Templates.ttcn "
FILES+="HTTP2_CodecPort.ttcn HTTP2_CodecPort_CtrlFunct.ttcn HTTP2_CodecPort_CtrlFunctDef.cc HTTP2_Templates.ttcn HTTP2_Functions.ttcn HTTP2_Msg.ttcn HTTP2_Adapter.ttcn HTTP2_Server_Emulation.ttcn "
FILES+="HTTP_Adapter.ttcn Prometheus_Checker.ttcn "
FILES+="JSON_Generic_Templates.ttcn TS29505_Subscription_Data_Templates.ttcn "
gen_links $DIR $FILES

gen_links_finish
