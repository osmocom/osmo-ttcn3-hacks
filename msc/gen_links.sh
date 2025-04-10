#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
FILES+=" TCCEncoding_Functions.ttcn TCCEncoding.cc " # GSM 7-bit coding
FILES+=" TCCOpenSecurity_Functions.ttcn TCCOpenSecurity.cc TCCOpenSecurity_Functions.hh"
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

DIR=$BASEDIR/titan.ProtocolModules.BSSMAP/src
FILES="BSSAP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.ROSE/src
FILES="Remote_Operations_Generic_ROS_PDUs.asn  Remote_Operations_Information_Objects.asn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MAP/src
FILES="MAP_ApplicationContexts.asn MAP_CH_DataTypes.asn MAP_CallHandlingOperations.asn MAP_CommonDataTypes.asn MAP_DialogueInformation.asn MAP_ER_DataTypes.asn MAP_EncDec.cc MAP_Errors.asn MAP_GR_DataTypes.asn MAP_Group_Call_Operations.asn MAP_LCS_DataTypes.asn MAP_LocationServiceOperations.asn MAP_MS_DataTypes.asn MAP_MobileServiceOperations.asn MAP_OM_DataTypes.asn MAP_OperationAndMaintenanceOperations.asn MAP_PDU_Defs.asn MAP_Protocol.asn MAP_SM_DataTypes.asn MAP_SS_Code.asn MAP_SS_DataTypes.asn MAP_ShortMessageServiceOperations.asn MAP_SupplementaryServiceOperations.asn MAP_TS_Code.asn MAP_Types.ttcn "
FILES+="MAP_BS_Code.asn MAP_ExtensionDataTypes.asn MobileDomainDefinitions.asn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn "
FILES+="SS_DataTypes.asn SS_Errors.asn SS_Operations.asn SS_PDU_Defs.asn SS_Protocol.asn SS_Types.ttcn SS_EncDec.cc"
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

DIR=$BASEDIR/titan.ProtocolModules.SGsAP_13.2.0/src
FILES="SGsAP_Types.ttcn"
gen_links $DIR $FILES

DIR=../library/ranap
FILES="RANAP_CommonDataTypes.asn RANAP_Constants.asn RANAP_Containers.asn RANAP_IEs.asn RANAP_PDU_Contents.asn RANAP_PDU_Descriptions.asn "
FILES+="RANAP_Types.ttcn RANAP_Templates.ttcn RANAP_CodecPort.ttcn RANAP_EncDec.cc "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn MNCC_Types.ttcn MNCC_EncDec.cc MNCC_CodecPort.ttcn mncc.h MNCC_Emulation.ttcn Osmocom_VTY_Functions.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="IPA_Types.ttcn IPA_Emulation.ttcnpp IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc "
FILES+="PCO_Types.ttcn GSUP_Types.ttcn GSUP_Templates.ttcn GSUP_Emulation.ttcn "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn L3_Templates.ttcn RLCMAC_CSN1_Templates.ttcn RLCMAC_CSN1_Types.ttcn L3_Common.ttcn "
FILES+="RAN_Emulation.ttcnpp BSSAP_CodecPort.ttcn BSSMAP_Templates.ttcn RAN_Adapter.ttcnpp SDP_Templates.ttcn MGCP_Types.ttcn MGCP_Templates.ttcn MGCP_CodecPort_CtrlFunct.ttcn MGCP_Emulation.ttcn "
FILES+="RTP_CodecPort.ttcn RTP_CodecPort_CtrlFunctDef.cc "
FILES+="MGCP_CodecPort.ttcn MGCP_CodecPort_CtrlFunctDef.cc "
FILES+="SMPP_CodecPort.ttcn SMPP_CodecPort_CtrlFunct.ttcn SMPP_CodecPort_CtrlFunctDef.cc SMPP_Emulation.ttcn SMPP_Templates.ttcn "
FILES+="SS_Templates.ttcn SCCP_Templates.ttcn USSD_Helpers.ttcn "
FILES+="SGsAP_Templates.ttcn SGsAP_CodecPort.ttcn SGsAP_CodecPort_CtrlFunct.ttcn SGsAP_CodecPort_CtrlFunctDef.cc SGsAP_Emulation.ttcn DNS_Helpers.ttcn "
FILES+="SCTP_Templates.ttcn "
gen_links $DIR $FILES

gen_links_finish
