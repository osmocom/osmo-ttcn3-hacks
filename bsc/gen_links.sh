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

#DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
#FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
#gen_links $DIR $FILES

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCConversion.hh TCCInterface.cc TCCInterface_ip.h SDP_EncDec.cc"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

# Required by MGCP and IPA
DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
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
DIR=../MTP3asp_CNL113337/src
FILES="MTP3asp_PortType.ttcn  MTP3asp_Types.ttcn"
gen_links $DIR $FILES

# required by SCCP Emulation
DIR=../M3UA_CNL113537/src
FILES="M3UA_Emulation.ttcn"
gen_links $DIR $FILES

# required by SCCP Emulation
DIR=../MTP3asp_CNL113337/src
FILES="MTP3asp_PortType.ttcn  MTP3asp_Types.ttcn"
gen_links $DIR $FILES

DIR=../SCCP_CNL113341/src
FILES="SCCP_Emulation.ttcn  SCCP_Mapping.ttcnpp  SCCP_Types.ttcn  SCCPasp_Types.ttcn"
gen_links $DIR $FILES
ln -sf SCCP_Mapping.ttcnpp SCCP_Mapping.ttcn

DIR=$BASEDIR/titan.ProtocolModules.BSSMAP_v11.2.0/src
FILES="BSSAP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.SDP/src
FILES="SDP_EncDec.cc SDP_Types.ttcn SDP_parse_.tab.c SDP_parse_.tab.h SDP_parse_parser.h SDP_parser.l
SDP_parser.y lex.SDP_parse_.c"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.RTP/src
FILES="RTP_EncDec.cc RTP_Types.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="General_Types.ttcn Osmocom_Types.ttcn GSM_Types.ttcn IPA_Types.ttcn IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc IPA_Emulation.ttcn L3_Templates.ttcn BSSMAP_Templates.ttcn BSSMAP_Emulation.ttcn RLCMAC_CSN1_Types.ttcn GSM_RR_Types.ttcn RSL_Types.ttcn RSL_Emulation.ttcn MGCP_Types.ttcn MGCP_Templates.ttcn BSSAP_CodecPort.ttcn Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn"
gen_links $DIR $FILES
