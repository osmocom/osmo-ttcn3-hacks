#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

#DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
#FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
#gen_links $DIR $FILES

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.HTTP2/src
FILES="HTTP2_EncDec.cc HTTP2_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.JSON_v07_2006/src
FILES="JSON_Generic.ttcn JSON_Generic_Null_Def.asn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.5G_system_TS29571_CommonData_v15/src
FILES="TS29571_CommonData.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.5G_system_TS29503_Nudm_v15/src
FILES="TS29503_Nudm_UEAU.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc"
gen_links $DIR $FILES

ignore_pp_results
