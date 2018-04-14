#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

# Required by MGCP and IPA
DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

# required by M3UA_Emulation
DIR=$BASEDIR/titan.ProtocolModules.M3UA/src
FILES="M3UA_Types.ttcn"
gen_links $DIR $FILES

# required by M3UA_Emulation
DIR=$BASEDIR/titan.TestPorts.SCTPasp/src
FILES="SCTPasp_PT.cc  SCTPasp_PT.hh  SCTPasp_PortType.ttcn  SCTPasp_Types.ttcn"
gen_links $DIR $FILES

# required by M3UA Emulation
DIR=$BASEDIR/titan.TestPorts.MTP3asp/src
FILES="MTP3asp_PortType.ttcn  MTP3asp_Types.ttcn"
gen_links $DIR $FILES

# required by SCCP Emulation
DIR=$BASEDIR/titan.ProtocolEmulations.M3UA/src
FILES="M3UA_Emulation.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolEmulations.SCCP/src
FILES="SCCP_Emulation.ttcn  SCCP_EncDec.cc  SCCP_Mapping.ttcnpp  SCCP_Types.ttcn  SCCPasp_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.BSSMAP_v11.2.0/src
FILES="BSSAP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.SDP/src
FILES="SDP_EncDec.cc SDP_Types.ttcn SDP_parse_.tab.c SDP_parse_.tab.h SDP_parse_parser.h SDP_parser.l
SDP_parser.y lex.SDP_parse_.c"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.RTP/src
FILES="RTP_EncDec.cc RTP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.SMPP/src
FILES="SMPP_EncDec.cc  SMPP_Types.ttcn"
gen_links $DIR $FILES


DIR=../library
FILES="General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn MNCC_Types.ttcn MNCC_EncDec.cc MNCC_CodecPort.ttcn mncc.h MNCC_Emulation.ttcn Osmocom_VTY_Functions.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="IPA_Types.ttcn IPA_Emulation.ttcnpp IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc GSUP_Types.ttcn GSUP_Emulation.ttcn "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn L3_Templates.ttcn L3_Common.ttcn "
FILES+="BSSMAP_Emulation.ttcn BSSAP_CodecPort.ttcn BSSMAP_Templates.ttcn BSSAP_Adapter.ttcn MGCP_Types.ttcn MGCP_Templates.ttcn MGCP_CodecPort_CtrlFunct.ttcn MGCP_Emulation.ttcn "
FILES+="RTP_CodecPort.ttcn RTP_CodecPort_CtrlFunctDef.cc "
FILES+="MGCP_CodecPort.ttcn MGCP_CodecPort_CtrlFunctDef.cc "
FILES+="SMPP_CodecPort.ttcn SMPP_CodecPort_CtrlFunct.ttcn SMPP_CodecPort_CtrlFunctDef.cc SMPP_Emulation.ttcn SMPP_Templates.ttcn "
gen_links $DIR $FILES

ignore_pp_results
