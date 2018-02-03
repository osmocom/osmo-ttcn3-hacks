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

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="General_Types.ttcn GSM_Types.ttcn GSM_RR_Types.ttcn Osmocom_Types.ttcn RLCMAC_Types.ttcn RLCMAC_CSN1_Types.ttcn RLCMAC_EncDec.cc L1CTL_Types.ttcn L1CTL_PortType.ttcn LAPDm_RAW_PT.ttcn LAPDm_Types.ttcn "
FILES+="NS_Types.ttcn NS_Emulation.ttcn NS_CodecPort.ttcn NS_CodecPort_CtrlFunct.ttcn NS_CodecPort_CtrlFunctDef.cc "
FILES+="BSSGP_Emulation.ttcn BSSGP_Helper.cc BSSGP_Helper_Functions.ttcn BSSGP_Types.ttcn "
gen_links $DIR $FILES
