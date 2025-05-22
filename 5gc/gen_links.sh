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

DIR=../library/ngap
FILES="NGAP_CommonDataTypes.asn  NGAP_Constants.asn  NGAP_Containers.asn  NGAP_IEs.asn  NGAP_PDU_Contents.asn  NGAP_PDU_Descriptions.asn "
FILES+="NGAP_EncDec.cc NGAP_Types.ttcn NGAP_Pixits.ttcn NGAP_Templates.ttcn "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc IPCP_Types.ttcn IPCP_Templates.ttcn "
FILES+="SCTP_Templates.ttcn "
FILES+="DNS_Helpers.ttcn "
FILES+="NGAP_CodecPort.ttcn NGAP_CodecPort_CtrlFunctDef.cc NGAP_CodecPort_CtrlFunct.ttcn NGAP_Functions.ttcn NGAP_Emulation.ttcn "
gen_links $DIR $FILES

gen_links_finish
