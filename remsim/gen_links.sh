#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
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

DIR=$BASEDIR/titan.TestPorts.Common_Components.Abstract_Socket/src
FILES="Abstract_Socket.cc Abstract_Socket.hh "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.HTTPmsg/src
FILES="HTTPmsg_MessageLen.ttcn HTTPmsg_MessageLen_Function.cc HTTPmsg_PT.cc HTTPmsg_PT.hh HTTPmsg_PortType.ttcn HTTPmsg_Types.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.JSON_v07_2006/src
FILES="JSON_EncDec.cc JSON_Types.ttcn "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.PIPEasp/src
FILES="PIPEasp_PT.cc PIPEasp_PT.hh PIPEasp_Types.ttcn PIPEasp_PortType.ttcn "
gen_links $DIR $FILES


DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_VTY_Functions.ttcn Osmocom_Types.ttcn "
FILES+="PIPEasp_Templates.ttcn "
FILES+="IPA_Types.ttcn IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc IPA_Emulation.ttcnpp IPA_CodecPort.ttcn " #RSL_Types.ttcn RSL_Emulation.ttcn "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn  "
FILES+="Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="HTTP_Adapter.ttcn "
FILES+="VPCD_Types.ttcn VPCD_CodecPort.ttcn VPCD_CodecPort_CtrlFunct.ttcn VPCD_CodecPort_CtrlFunctDef.cc
VPCD_Adapter.ttcn "
gen_links $DIR $FILES

gen_links_finish
