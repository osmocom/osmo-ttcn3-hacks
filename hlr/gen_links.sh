#!/bin/sh

BASEDIR=../deps

gen_links() {
	DIR=$1
	FILES=$*
	for f in $FILES; do
		echo "Linking $f"
		ln -sf $DIR/$f $f
	done
}

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc
TCCInterface.cc TCCInterface_ip.h"
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

DIR=../MTP3asp_CNL113337/src
FILES="MTP3asp_PortType.ttcn  MTP3asp_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="General_Types.ttcn Osmocom_Types.ttcn GSM_Types.ttcn IPA_Types.ttcn IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc IPA_Emulation.ttcn RSL_Types.ttcn MGCP_Types.ttcn MGCP_CodecPort.ttcn MGCP_CodecPort_CtrlFunctDef.cc Osmocom_CTRL_Types.ttcn GSUP_Types.ttcn"
gen_links $DIR $FILES
