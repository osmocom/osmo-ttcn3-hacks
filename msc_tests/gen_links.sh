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
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCConversion.hh TCCInterface.cc TCCInterface_ip.h"
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

DIR=../SCCP_CNL113341/src
FILES="SCCP_Emulation.ttcn  SCCP_EncDec.cc  SCCP_Mapping.ttcnpp  SCCP_Types.ttcn  SCCPasp_Types.ttcn"
gen_links $DIR $FILES
ln -s SCCP_Mapping.ttcnpp SCCP_Mapping.ttcn

DIR=$BASEDIR/titan.ProtocolModules.BSSMAP_v11.2.0/src
FILES="BSSAP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.MobileL3_v13.4.0/src
FILES="MobileL3_CC_Types.ttcn MobileL3_CommonIE_Types.ttcn MobileL3_GMM_SM_Types.ttcn MobileL3_MM_Types.ttcn MobileL3_RRM_Types.ttcn MobileL3_SMS_Types.ttcn MobileL3_SS_Types.ttcn MobileL3_Types.ttcn"
gen_links $DIR $FILES


DIR=../library
FILES="General_Types.ttcn Osmocom_Types.ttcn"
gen_links $DIR $FILES
