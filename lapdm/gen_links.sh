#!/bin/sh

BASEDIR=../deps

. ../gen_links.sh.inc

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES


DIR=../library
FILES="General_Types.ttcn GSM_Types.ttcn GSM_RR_Types.ttcn RLCMAC_CSN1_Types.ttcn Osmocom_Types.ttcn L1CTL_PortType.ttcn L1CTL_PortType_CtrlFunct.ttcn L1CTL_PortType_CtrlFunctDef.cc L1CTL_Types.ttcn LAPDm_RAW_PT.ttcn LAPDm_Types.ttcn RLCMAC_Types.ttcn RLCMAC_EncDec.cc"
gen_links $DIR $FILES
