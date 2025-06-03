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

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn "
#FILES+="SS_DataTypes.asn SS_Errors.asn SS_Operations.asn SS_PDU_Defs.asn SS_Protocol.asn SS_Types.ttcn SS_EncDec.cc"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.SGsAP_13.2.0/src
FILES="SGsAP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.NAS_EPS_15.2.0.1/src
FILES="NAS_EPS_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTPv2_v13.7.0/src
FILES="GTPv2_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTP_v13.5.0/src
FILES="GTPC_EncDec.cc GTPC_Types.ttcn GTPU_EncDec.cc GTPU_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.NS_v7.3.0/src
FILES="NS_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.BSSGP_v13.0.0/src
FILES="BSSGP_EncDec.cc  BSSGP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTPv2_v13.7.0/src
FILES="GTPv2_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/osmo-uecups/ttcn3
FILES="UECUPS_CodecPort.ttcn  UECUPS_CodecPort_CtrlFunct.ttcn  UECUPS_CodecPort_CtrlFunctDef.cc UECUPS_Types.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.DIAMETER_ProtocolModule_Generator/src
FILES="DIAMETER_EncDec.cc"
gen_links $DIR $FILES

DIR=../library/s1ap
FILES="S1AP_CommonDataTypes.asn S1AP_Constants.asn S1AP_Containers.asn S1AP_IEs.asn S1AP_PDU_Contents.asn
S1AP_PDU_Descriptions.asn "
FILES+="S1AP_EncDec.cc  S1AP_Types.ttcn  S1AP_Templates.ttcn "
gen_links $DIR $FILES

DIR=../library/snow_3g
FILES="snow-3g.c snow-3g.h Snow3G_FunctionDefs.cc  Snow3G_Functions.ttcn "
gen_links $DIR $FILES

DIR=../library/lte_crypto
FILES="key_derivation.c key_derivation.h "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc IPCP_Types.ttcn IPCP_Templates.ttcn "
FILES+="SGsAP_Templates.ttcn SGsAP_CodecPort.ttcn SGsAP_CodecPort_CtrlFunct.ttcn SGsAP_CodecPort_CtrlFunctDef.cc SGsAP_Emulation.ttcn DNS_Helpers.ttcn "
FILES+="L3_Templates.ttcn RLCMAC_CSN1_Templates.ttcn RLCMAC_CSN1_Types.ttcn "
FILES+="S1AP_CodecPort.ttcn S1AP_CodecPort_CtrlFunctDef.cc S1AP_CodecPort_CtrlFunct.ttcn S1AP_Functions.ttcn S1AP_Emulation.ttcn "
FILES+="NAS_EPS_Templates.ttcn LTE_CryptoFunctionDefs.cc  LTE_CryptoFunctions.ttcn "
FILES+="GTPv2_PrivateExtensions.ttcn GTPv2_Templates.ttcn "
FILES+="DIAMETER_Types.ttcn DIAMETER_CodecPort.ttcn DIAMETER_CodecPort_CtrlFunct.ttcn DIAMETER_CodecPort_CtrlFunctDef.cc DIAMETER_Emulation.ttcn "
FILES+="DIAMETER_Templates.ttcn DIAMETER_ts29_272_Templates.ttcn "
FILES+="GTPv1C_CodecPort.ttcn GTPv1C_CodecPort_CtrlFunct.ttcn GTPv1C_CodecPort_CtrlFunctDef.cc GTPv1U_CodecPort.ttcn GTPv1U_CodecPort_CtrlFunct.ttcn GTPv1U_CodecPort_CtrlFunctDef.cc GTP_Emulation.ttcn GTPv1C_Templates.ttcn Osmocom_Gb_Types.ttcn "
FILES+="GTPv2_PrivateExtensions.ttcn GTPv2_Templates.ttcn "
FILES+="GTPv2_CodecPort.ttcn GTPv2_CodecPort_CtrlFunctDef.cc GTPv2_CodecPort_CtrlFunct.ttcn GTPv2_Emulation.ttcn "
FILES+="BSSGP_Emulation.ttcnpp Osmocom_Gb_Types.ttcn "
FILES+="SCTP_Templates.ttcn "
gen_links $DIR $FILES

gen_links_finish
