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

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
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

DIR=$BASEDIR/titan.ProtocolModules.DNS/src
FILES="DNS_EncDec.cc DNS_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.UDPasp/src
FILES="UDPasp_PT.cc  UDPasp_PT.hh  UDPasp_PortType.ttcn  UDPasp_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Native_Functions.ttcn Native_FunctionDefs.cc Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn GSM_Types.ttcn IPA_Types.ttcn IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc IPA_Emulation.ttcnpp "
FILES+="PCO_Types.ttcn GSUP_Types.ttcn GSUP_Templates.ttcn GSUP_Emulation.ttcn "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn "
FILES+="Osmocom_VTY_Functions.ttcn "
FILES+="SS_Templates.ttcn USSD_Helpers.ttcn "
FILES+="MSLookup_mDNS_Types.ttcn MSLookup_mDNS_Emulation.ttcn MSLookup_mDNS_Templates.ttcn DNS_Helpers.ttcn "


gen_links $DIR $FILES

gen_links_finish
