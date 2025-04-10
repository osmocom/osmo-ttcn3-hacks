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

DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.IP/src
FILES="IP_EncDec.cc IP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.ICMP/src
FILES="ICMP_EncDec.cc ICMP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.DIAMETER_ProtocolModule_Generator/src
FILES="DIAMETER_EncDec.cc"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/osmo-uecups/ttcn3
FILES="UECUPS_CodecPort.ttcn  UECUPS_CodecPort_CtrlFunct.ttcn  UECUPS_CodecPort_CtrlFunctDef.cc UECUPS_Types.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTP_v13.5.0/src
FILES="GTPU_EncDec.cc GTPU_Types.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTPv2_v13.7.0/src
FILES="GTPv2_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="Osmocom_CTRL_Types.ttcn "
FILES+="L3_Common.ttcn "
FILES+="DNS_Helpers.ttcn "
FILES+="DIAMETER_Types.ttcn DIAMETER_CodecPort.ttcn DIAMETER_CodecPort_CtrlFunct.ttcn DIAMETER_CodecPort_CtrlFunctDef.cc DIAMETER_Emulation.ttcn "
FILES+="DIAMETER_Templates.ttcn DIAMETER_rfc4004_Templates.ttcn DIAMETER_rfc5447_Templates.ttcn DIAMETER_ts29_229_Templates.ttcn DIAMETER_ts29_272_Templates.ttcn DIAMETER_ts29_273_Templates.ttcn "
FILES+="IPA_Types.ttcn IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc
IPA_Emulation.ttcnpp "
FILES+="PCO_Types.ttcn GSUP_Types.ttcn GSUP_Templates.ttcn GSUP_Emulation.ttcn "
FILES+="GTPv1U_CodecPort.ttcn GTPv1U_CodecPort_CtrlFunct.ttcn GTPv1U_CodecPort_CtrlFunctDef.cc GTPv1U_Templates.ttcn "
FILES+="GTPv2_PrivateExtensions.ttcn GTPv2_Templates.ttcn "
FILES+="GTPv2_CodecPort.ttcn GTPv2_CodecPort_CtrlFunctDef.cc GTPv2_CodecPort_CtrlFunct.ttcn GTPv2_Emulation.ttcn "
FILES+="ICMP_Templates.ttcn "
FILES+="SCTP_Templates.ttcn "
gen_links $DIR $FILES

gen_links_finish
