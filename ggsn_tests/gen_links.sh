#!/bin/bash

BASEDIR=../deps

gen_links() {
	DIR=$1
	FILES=$*
	for f in $FILES; do
		echo "Linking $f"
		ln -sf $DIR/$f $f
	done
}

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

DIR=$BASEDIR/titan.ProtocolModules.ICMP/src
FILES="ICMP_EncDec.cc  ICMP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.ICMPv6/src
FILES="ICMPv6_EncDec.cc  ICMPv6_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.IP/src
FILES="IP_EncDec.cc  IP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.UDP/src
FILES="UDP_EncDec.cc  UDP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTP_v13.5.0/src
FILES="GTPC_EncDec.cc  GTPC_Types.ttcn  GTPU_EncDec.cc  GTPU_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc IPCP_Types.ttcn "
FILES+="GTP_CodecPort.ttcn GTP_CodecPort_CtrlFunct.ttcn GTP_CodecPort_CtrlFunctDef.cc GTP_Templates.ttcn "
gen_links $DIR $FILES
