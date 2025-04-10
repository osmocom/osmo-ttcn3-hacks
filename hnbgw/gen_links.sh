#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

#DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
#FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
#gen_links $DIR $FILES

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
FILES+=" TCCEncoding_Functions.ttcn TCCEncoding.cc " # GSM 7-bit coding
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn"
gen_links $DIR $FILES

# Required by MGCP and IPA
# Required by PFCP/UDP
DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

# required by SCCP Emulation
DIR=$BASEDIR/titan.ProtocolEmulations.M3UA/src
FILES="M3UA_Emulation.ttcn"
gen_links $DIR $FILES

# required by SCCP Emulation
DIR=$BASEDIR/titan.TestPorts.MTP3asp/src
FILES="MTP3asp_PortType.ttcn  MTP3asp_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolEmulations.SCCP/src
FILES="SCCP_Emulation.ttcn  SCCP_Mapping.ttcnpp  SCCP_Types.ttcn  SCCPasp_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.BSSMAP/src
FILES="BSSAP_Types.ttcn"
gen_links $DIR $FILES

# required by M3UA_Emulation
DIR=$BASEDIR/titan.ProtocolModules.M3UA/src
FILES="M3UA_Types.ttcn"
gen_links $DIR $FILES

# required by M3UA_Emulation
DIR=$BASEDIR/titan.TestPorts.SCTPasp/src
FILES="SCTPasp_PT.cc  SCTPasp_PT.hh  SCTPasp_PortType.ttcn  SCTPasp_Types.ttcn"
gen_links $DIR $FILES

# required by SCCP Emulation
DIR=$BASEDIR/titan.ProtocolEmulations.M3UA/src
FILES="M3UA_Emulation.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
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

DIR=$BASEDIR/titan.ProtocolModules.PFCP_v15.1.0/src
FILES="PFCP_Types.ttcn"
gen_links $DIR $FILES

DIR=../library/hnbap
FILES="HNBAP_CommonDataTypes.asn HNBAP_Constants.asn HNBAP_Containers.asn HNBAP_IEs.asn HNBAP_PDU_Contents.asn HNBAP_PDU_Descriptions.asn "
FILES+="HNBAP_EncDec.cc  HNBAP_Types.ttcn  HNBAP_Templates.ttcn "
gen_links $DIR $FILES

DIR=../library/rua
FILES="RUA_CommonDataTypes.asn RUA_Constants.asn RUA_Containers.asn RUA_IEs.asn RUA_PDU_Contents.asn RUA_PDU_Descriptions.asn "
FILES+="RUA_EncDec.cc RUA_Types.ttcn RUA_Templates.ttcn RUA_Emulation.ttcn "
gen_links $DIR $FILES

DIR=../library/ranap
FILES="RANAP_CommonDataTypes.asn RANAP_Constants.asn RANAP_Containers.asn RANAP_IEs.asn RANAP_PDU_Contents.asn RANAP_PDU_Descriptions.asn "
FILES+="RANAP_Types.ttcn RANAP_Templates.ttcn RANAP_CodecPort.ttcn RANAP_EncDec.cc "
gen_links $DIR $FILES

DIR=../library
FILES="Iuh_Types.ttcn Iuh_CodecPort.ttcn Iuh_CodecPort_CtrlFunctDef.cc Iuh_CodecPort_CtrlFunct.ttcn Iuh_Emulation.ttcn DNS_Helpers.ttcn "
FILES+="SDP_Templates.ttcn MGCP_Emulation.ttcn MGCP_Types.ttcn MGCP_Templates.ttcn MGCP_CodecPort.ttcn MGCP_CodecPort_CtrlFunct.ttcn MGCP_CodecPort_CtrlFunctDef.cc "
FILES+="RAN_Adapter.ttcnpp RAN_Emulation.ttcnpp BSSAP_CodecPort.ttcn SCCP_Templates.ttcn "
FILES+="PFCP_CodecPort.ttcn PFCP_CodecPort_CtrlFunct.ttcn PFCP_CodecPort_CtrlFunctDef.cc PFCP_Emulation.ttcn PFCP_Templates.ttcn "
FILES+="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn GSM_Types.ttcn Osmocom_VTY_Functions.ttcn Native_Functions.ttcn Native_FunctionDefs.cc IPA_Types.ttcn IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc IPA_Emulation.ttcnpp Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn RTP_CodecPort.ttcn RTP_CodecPort_CtrlFunct.ttcn RTP_CodecPort_CtrlFunctDef.cc RTP_Emulation.ttcn IuUP_Types.ttcn IuUP_EncDec.cc IuUP_Emulation.ttcn "
FILES+="StatsD_Types.ttcn StatsD_CodecPort.ttcn StatsD_CodecPort_CtrlFunct.ttcn StatsD_CodecPort_CtrlFunctdef.cc StatsD_Checker.ttcnpp "
FILES+="L3_Templates.ttcn L3_Common.ttcn "
FILES+="SCTP_Templates.ttcn "
FILES+="Mutex.ttcn "
gen_links $DIR $FILES

gen_links_finish
