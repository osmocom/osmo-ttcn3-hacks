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

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn "
#FILES+="SS_DataTypes.asn SS_Errors.asn SS_Operations.asn SS_PDU_Defs.asn SS_Protocol.asn SS_Types.ttcn SS_EncDec.cc"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.SGsAP_13.2.0/src
FILES="SGsAP_Types.ttcn"
gen_links $DIR $FILES

DIR=../library/s1ap
FILES="S1AP_CommonDataTypes.asn S1AP_Constants.asn S1AP_Containers.asn S1AP_IEs.asn S1AP_PDU_Contents.asn
S1AP_PDU_Descriptions.asn "
FILES+="S1AP_EncDec.cc  S1AP_Types.ttcn "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="SGsAP_Templates.ttcn SGsAP_CodecPort.ttcn SGsAP_CodecPort_CtrlFunct.ttcn SGsAP_CodecPort_CtrlFunctDef.cc SGsAP_Emulation.ttcn DNS_Helpers.ttcn "
FILES+="L3_Templates.ttcn "
FILES+="S1AP_CodecPort.ttcn "
gen_links $DIR $FILES

ignore_pp_results
