#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

#DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
#FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
#gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.USB/src
FILES="USB_PT.cc USB_PT.hh USB_PortType.ttcn USB_PortTypes.ttcn USB_Templates.ttcn USB_Types.ttcn USB_Component.ttcn "
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn "
FILES+="Native_Functions.ttcn Native_FunctionDefs.cc "
gen_links $DIR $FILES

ignore_pp_results
