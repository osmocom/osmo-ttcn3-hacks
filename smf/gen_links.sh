#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
FILES+=" TCCEncoding_Functions.ttcn TCCEncoding.cc " # BCD coding
FILES+="TCCDateTime.cc TCCDateTime_Functions.ttcn " # NTP_Functions (NTP timestamp)
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.ICMP/src
FILES="ICMP_EncDec.cc  ICMP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.ICMPv6/src
FILES="ICMPv6_EncDec.cc  ICMPv6_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.IP/src
FILES="IP_EncDec.cc  IP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.UDP/src
FILES="UDP_EncDec.cc  UDP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.NS_v7.3.0/src
FILES="NS_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.BSSGP_v13.0.0/src
FILES="BSSGP_EncDec.cc  BSSGP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTP_v13.5.0/src
FILES="GTPC_EncDec.cc  GTPC_Types.ttcn  GTPU_EncDec.cc  GTPU_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTPv2_v13.7.0/src
FILES="GTPv2_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.PFCP_v15.1.0/src
FILES="PFCP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.DIAMETER_ProtocolModule_Generator/src
FILES="DIAMETER_EncDec.cc"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.HTTP2/src
FILES="HTTP2_EncDec.cc HTTP2_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.JSON_v07_2006/src
FILES="JSON_Generic_Null_Def.asn JSON_Generic.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/osmo-ttcn3-openapi-generator/openapi-specs/3GPP_5GC_Rel19/ttcn3
FILES="TS29571_CommonData.ttcn TS29502_Nsmf_PDUSession.ttcn TS29503_Nudm_UECM.ttcn TS29503_Nudm_SDM.ttcn TS29512_Npcf_SMPolicyControl.ttcn TS29518_Namf_Communication.ttcn "
gen_links $DIR $FILES


####################
# NG_NAS start
####################
DIR=$BASEDIR/nas/ccsrc/Externals
FILES="common_ext.cc "
gen_links $DIR $FILES

DIR=$BASEDIR/nas/ttcn/Lib3GPP/Common
FILES="CommonDefs.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/nas/ttcn/Lib3GPP/PicsPixit
FILES="EUTRA_NR_Parameters.ttcn  NAS_5GC_Parameters.ttcn  Parameters.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/nas/ttcn/Lib3GPP/CommonIP
FILES="CommonIP.ttcn  LoopbackIP_PacketFilterTest.ttcn LoopbackIP.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/nas/ttcn/Lib3GPP/EPS_NAS
FILES="EPS_NAS_LoopBack_TypeDefs.ttcn  EPS_NAS_TypeDefs.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/nas/ttcn/Lib3GPP/Common4G5G
FILES="Common4G5G_LoopBack.ttcn  Common4G5G.ttcn  EUTRA_NR_SecurityFunctions.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/nas/ttcn/Lib3GPP/NAS
FILES="NAS_AuthenticationCommon.ttcn NAS_AuxiliaryDefsAndFunctions.ttcn NAS_CommonTemplates.ttcn NAS_CommonTypeDefs.ttcn SMS_Templates.ttcn SMS_TypeDefs.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/nas/ttcn/Lib3GPP/NG_NAS
FILES="EAP_TypeDefs.ttcn NG_NAS_Common.ttcn NG_NAS_MsgContainers.ttcn NG_NAS_Templates.ttcn NG_NAS_TypeDefs.ttcn NG_V2X_MsgContainers.ttcn NG_V2X_TypeDefs.ttcn "
gen_links $DIR $FILES
####################
# NG_NAS end
####################

DIR=../library/ngap
FILES="NGAP_CommonDataTypes.asn  NGAP_Constants.asn  NGAP_Containers.asn  NGAP_IEs.asn  NGAP_PDU_Contents.asn  NGAP_PDU_Descriptions.asn "
FILES+="NGAP_EncDec.cc NGAP_Types.ttcn NGAP_Pixits.ttcn NGAP_Templates.ttcn "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc IPCP_Types.ttcn IPCP_Templates.ttcn PAP_Types.ttcn "
FILES+="GTPv1C_CodecPort.ttcn GTPv1C_CodecPort_CtrlFunct.ttcn GTPv1C_CodecPort_CtrlFunctDef.cc GTPv1C_Templates.ttcn Osmocom_Gb_Types.ttcn "
FILES+="GTPv1U_CodecPort.ttcn GTPv1U_CodecPort_CtrlFunct.ttcn GTPv1U_CodecPort_CtrlFunctDef.cc GTPv1U_Emulation.ttcnpp "
FILES+="GTPv2_PrivateExtensions.ttcn GTPv2_Templates.ttcn "
FILES+="GTPv2_CodecPort.ttcn GTPv2_CodecPort_CtrlFunctDef.cc GTPv2_CodecPort_CtrlFunct.ttcn GTPv2_Emulation.ttcn "
FILES+="DNS_Helpers.ttcn "
FILES+="DIAMETER_Types.ttcn DIAMETER_CodecPort.ttcn DIAMETER_CodecPort_CtrlFunct.ttcn DIAMETER_CodecPort_CtrlFunctDef.cc DIAMETER_Emulation.ttcn "
FILES+="DIAMETER_Templates.ttcn DIAMETER_rfc4004_Templates.ttcn DIAMETER_rfc5447_Templates.ttcn DIAMETER_ts29_212_Templates.ttcn DIAMETER_ts29_212_Templates.ttcn DIAMETER_ts29_229_Templates.ttcn DIAMETER_ts29_272_Templates.ttcn DIAMETER_ts29_273_Templates.ttcn DIAMETER_ts32_299_Templates.ttcn "
FILES+="SCTP_Templates.ttcn "
FILES+="NTP_Functions.ttcn PFCP_Templates.ttcn PFCP_CodecPort.ttcn PFCP_CodecPort_CtrlFunct.ttcn PFCP_CodecPort_CtrlFunctDef.cc PFCP_Emulation.ttcn "
FILES+="Mutex.ttcn "
FILES+="HTTP2_CodecPort.ttcn HTTP2_CodecPort_CtrlFunct.ttcn HTTP2_CodecPort_CtrlFunctDef.cc HTTP2_Templates.ttcn HTTP2_Functions.ttcn HTTP2_Msg.ttcn HTTP2_Adapter.ttcn HTTP2_Server_Emulation.ttcn "
FILES+="TS29503_Nudm_UECM_Templates.ttcn TS29503_Nudm_SDM_Templates.ttcn TS29502_Nsmf_PDUSession_Templates.ttcn TS29512_Npcf_SMPolicyControl_Templates.ttcn TS29518_Namf_Communication_Templates.ttcn "
FILES+="NG_NAS_Osmo_Types.ttcn NG_NAS_Osmo_Templates.ttcn NG_NAS_Functions.ttcn "
gen_links $DIR $FILES

gen_links_finish
