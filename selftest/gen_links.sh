#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

#DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
#FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
#gen_links $DIR $FILES

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

# required by IPA Emulation
DIR=$BASEDIR/titan.TestPorts.MTP3asp/src
FILES="MTP3asp_PortType.ttcn  MTP3asp_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolEmulations.SCCP/src
FILES="SCCP_Emulation.ttcn  SCCP_Mapping.ttcnpp  SCCP_Types.ttcn  SCCPasp_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.BSSMAP/src
FILES="BSSAP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn GSM_Types.ttcn IPA_Types.ttcn IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc IPA_Emulation.ttcnpp L3_Templates.ttcn BSSMAP_Templates.ttcn RLCMAC_CSN1_Templates.ttcn RLCMAC_CSN1_Types.ttcn GSM_RR_Types.ttcn GSM_RestOctets.ttcn RSL_Types.ttcn BSSAP_CodecPort.ttcn Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn PCO_Types.ttcn GSUP_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc"
gen_links $DIR $FILES

gen_links_finish
