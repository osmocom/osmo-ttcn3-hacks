#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

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

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.AF_PACKET/src
FILES="AF_PACKET_PT.cc AF_PACKET_PT.hh AF_PACKET_PortType.ttcn AF_PACKET_PortTypes.ttcn "
FILES+="FrameRelay_Types.ttcn FrameRelay_CodecPort.ttcn FrameRelay_Emulation.ttcn Q931_Types.ttcn Q933_Types.ttcn "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_VTY_Functions.ttcn Native_Functions.ttcn Native_FunctionDefs.cc GSM_Types.ttcn Osmocom_Types.ttcn "
FILES+="NS_Provider_IPL4.ttcn NS_Provider_FR.ttcn NS_Emulation.ttcnpp NS_CodecPort.ttcn NS_CodecPort_CtrlFunct.ttcn NS_CodecPort_CtrlFunctDef.cc "
FILES+="BSSGP_Emulation.ttcnpp Osmocom_Gb_Types.ttcn "
FILES+="LLC_Templates.ttcn "
gen_links $DIR $FILES

ignore_pp_results
