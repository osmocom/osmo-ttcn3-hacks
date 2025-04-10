#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h "
FILES+="TCCOpenSecurity_Functions.ttcn TCCOpenSecurity.cc TCCOpenSecurity_Functions.hh "
FILES+="TCCDateTime.cc TCCDateTime_Functions.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

# Required by MGCP and IPA
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

DIR=$BASEDIR/titan.TestPorts.SIPmsg/src
FILES="SIP_parse.h SIP_parse.y SIP_parse_.tab.h SIPmsg_PT.hh SIPmsg_Types.ttcn SIP_parse.l SIP_parse_.tab.c SIPmsg_PT.cc SIPmsg_PortType.ttcn lex.SIP_parse_.c"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn MNCC_Types.ttcn MNCC_EncDec.cc MNCC_CodecPort.ttcn mncc.h MNCC_Emulation.ttcn Osmocom_VTY_Functions.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="IPA_Types.ttcn IPA_Emulation.ttcnpp IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn "
FILES+="RTP_CodecPort.ttcn RTP_CodecPort_CtrlFunctDef.cc "
FILES+="SDP_Templates.ttcn "
FILES+="SIP_Emulation.ttcn SIP_Templates.ttcn "
gen_links $DIR $FILES

gen_links_finish
