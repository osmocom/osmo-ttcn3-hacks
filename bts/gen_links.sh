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

DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.RTP/src
FILES="RTP_EncDec.cc RTP_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn GSM_RR_Types.ttcn Osmocom_VTY_Functions.ttcn GSM_SystemInformation.ttcn GSM_RestOctets.ttcn Osmocom_Types.ttcn RLCMAC_Templates.ttcn RLCMAC_Types.ttcn RLCMAC_CSN1_Templates.ttcn RLCMAC_CSN1_Types.ttcn RLCMAC_EncDec.cc L1CTL_Types.ttcn L1CTL_PortType.ttcn L1CTL_PortType_CtrlFunct.ttcn L1CTL_PortType_CtrlFunctDef.cc LAPDm_RAW_PT.ttcn LAPDm_Types.ttcn "
#FILES+="BSSGP_Emulation.ttcn Osmocom_Gb_Types.ttcn "
FILES+="IPA_Types.ttcn IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc IPA_Emulation.ttcnpp IPA_CodecPort.ttcn RSL_Types.ttcn RSL_Emulation.ttcn AbisOML_Types.ttcn "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn  "
FILES+="L3_Templates.ttcn L3_Common.ttcn "
FILES+="Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="TRXC_Types.ttcn TRXC_CodecPort.ttcn TRXC_CodecPort_CtrlFunct.ttcn TRXC_CodecPort_CtrlFunctDef.cc "
FILES+="AMR_Types.ttcn "
FILES+="RTP_CodecPort.ttcn RTP_Emulation.ttcn IuUP_Types.ttcn IuUP_Emulation.ttcn IuUP_EncDec.cc "
FILES+="RTP_CodecPort_CtrlFunct.ttcn RTP_CodecPort_CtrlFunctDef.cc "
FILES+="OSMUX_CodecPort.ttcn OSMUX_Emulation.ttcn OSMUX_Types.ttcn OSMUX_CodecPort_CtrlFunct.ttcn OSMUX_CodecPort_CtrlFunctDef.cc "
FILES+="PCUIF_Types.ttcn PCUIF_CodecPort.ttcn "
FILES+="IPA_Testing.ttcn"
gen_links $DIR $FILES

gen_links_finish
