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

DIR=$BASEDIR/titan.TestPorts.SCTPasp/src
FILES="SCTPasp_PT.cc  SCTPasp_PT.hh  SCTPasp_PortType.ttcn  SCTPasp_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.BSSMAP/src
FILES="BSSAP_Types.ttcn"
gen_links $DIR $FILES

DIR=../library/sabp
FILES="SABP_CommonDataTypes.asn SABP_Constants.asn SABP_Containers.asn SABP_IEs.asn SABP_PDU_Contents.asn SABP_PDU_Descriptions.asn SABP_Types.ttcn SABP_EncDec.cc SABP_Templates.ttcn "
FILES+="SABP_CodecPort.ttcn SABP_CodecPort_CtrlFunct.ttcn SABP_CodecPort_CtrlFunctDef.cc SABP_Adapter.ttcn "
gen_links $DIR $FILES

DIR=../library/sbcap
FILES="SBC_AP_CommonDataTypes.asn SBC_AP_Constants.asn SBC_AP_Containers.asn SBC_AP_IEs.asn SBC_AP_PDU_Contents.asn SBC_AP_PDU_Descriptions.asn "
FILES+="SBC_AP_Types.ttcn SBC_AP_EncDec.cc SBC_AP_Templates.ttcn SBC_AP_CodecPort.ttcn SBC_AP_CodecPort_CtrlFunct.ttcn SBC_AP_CodecPort_CtrlFunctDef.cc SBC_AP_Adapter.ttcn "
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

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="HTTP_Adapter.ttcn "
FILES+="BSSMAP_Templates.ttcn "
FILES+="CBSP_Types.ttcn CBSP_Templates.ttcn "
FILES+="CBSP_CodecPort.ttcn CBSP_CodecPort_CtrlFunct.ttcn CBSP_CodecPort_CtrlFunctdef.cc CBSP_Adapter.ttcn "
FILES+="SCTP_Templates.ttcn "
gen_links $DIR $FILES

gen_links_finish
