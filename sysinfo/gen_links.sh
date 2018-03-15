#!/bin/sh

BASEDIR=../deps

. ../gen_links.sh.inc

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

DIR=../library
FILES="GSMTAP_PortType.ttcn GSMTAP_Types.ttcn GSM_SystemInformation.ttcn GSM_RR_Types.ttcn RLCMAC_CSN1_Types.ttcn GSM_Types.ttcn IPL4_GSMTAP_CtrlFunct.ttcn IPL4_GSMTAP_CtrlFunctDef.cc Osmocom_Types.ttcn General_Types.ttcn Osmocom_VTY_Functions.ttcn"
gen_links $DIR $FILES

