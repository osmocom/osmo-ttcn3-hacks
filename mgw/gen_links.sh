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

DIR=$BASEDIR/titan.ProtocolModules.SDP/src
FILES="SDP_EncDec.cc SDP_Types.ttcn SDP_parse_.tab.c SDP_parse_.tab.h SDP_parse_parser.h SDP_parser.l
SDP_parser.y lex.SDP_parse_.c"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.RTP/src
FILES="RTP_EncDec.cc RTP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.IP/src
FILES="IP_EncDec.cc  IP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn SDP_Templates.ttcn MGCP_Types.ttcn MGCP_Templates.ttcn MGCP_CodecPort.ttcn
MGCP_CodecPort_CtrlFunct.ttcn MGCP_CodecPort_CtrlFunctDef.cc "
FILES+="AMR_Types.ttcn "
FILES+="RTP_CodecPort.ttcn RTP_Emulation.ttcn IuUP_Types.ttcn IuUP_Emulation.ttcn IuUP_EncDec.cc "
FILES+="OSMUX_CodecPort.ttcn OSMUX_Emulation.ttcn OSMUX_Types.ttcn OSMUX_CodecPort_CtrlFunct.ttcn OSMUX_CodecPort_CtrlFunctDef.cc "
FILES+="Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="Osmocom_VTY_Functions.ttcn "
FILES+="RTP_CodecPort_CtrlFunct.ttcn RTP_CodecPort_CtrlFunctDef.cc "
FILES+="StatsD_Types.ttcn StatsD_CodecPort.ttcn StatsD_CodecPort_CtrlFunct.ttcn StatsD_CodecPort_CtrlFunctdef.cc StatsD_Checker.ttcnpp "
FILES+="IPA_Types.ttcn IPA_Emulation.ttcnpp IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn "

gen_links $DIR $FILES

gen_links_finish
