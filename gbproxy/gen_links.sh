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

#DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
#FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
#gen_links $DIR $FILES

# for Osmocom_VTY
DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.NS_v7.3.0/src
FILES="NS_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.BSSGP_v13.0.0/src
FILES="BSSGP_EncDec.cc  BSSGP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.LLC_v7.1.0/src
FILES="LLC_EncDec.cc LLC_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.SNDCP_v7.0.0/src
FILES="SNDCP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn"
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

DIR=$BASEDIR/titan.TestPorts.AF_PACKET/src
FILES="AF_PACKET_PT.cc AF_PACKET_PT.hh AF_PACKET_PortType.ttcn AF_PACKET_PortTypes.ttcn "
FILES+="FrameRelay_Types.ttcn FrameRelay_CodecPort.ttcn FrameRelay_Emulation.ttcn Q931_Types.ttcn Q933_Types.ttcn "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn "
FILES+="RAW_NS.ttcnpp NS_Provider_IPL4.ttcn NS_Provider_FR.ttcn NS_Emulation.ttcnpp "
FILES+="BSSGP_Emulation.ttcnpp Osmocom_Gb_Types.ttcn "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn "
FILES+="Osmocom_VTY_Functions.ttcn "
FILES+="LLC_Templates.ttcn L3_Templates.ttcn L3_Common.ttcn "
# IPA_Emulation + dependencies
FILES+="IPA_Types.ttcn IPA_Emulation.ttcnpp IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc Native_Functions.ttcn Native_FunctionDefs.cc "
gen_links $DIR $FILES

gen_links_finish
