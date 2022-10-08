#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn "
FILES+="GSM_Types.ttcn GSM_RR_Types.ttcn GSM_SystemInformation.ttcn GSM_RestOctets.ttcn "
FILES+="RLCMAC_Templates.ttcn RLCMAC_Types.ttcn RLCMAC_CSN1_Templates.ttcn RLCMAC_CSN1_Types.ttcn RLCMAC_EncDec.cc "
FILES+="L1CTL_Types.ttcn L1CTL_PortType.ttcn L1CTL_PortType_CtrlFunct.ttcn L1CTL_PortType_CtrlFunctDef.cc "
FILES+="LAPDm_RAW_PT.ttcn LAPDm_Types.ttcn "
FILES+="L3_Templates.ttcn L3_Common.ttcn "
FILES+="Native_Functions.ttcn Native_FunctionDefs.cc "
gen_links $DIR $FILES

ignore_pp_results
