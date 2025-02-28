#!/bin/bash

BASEDIR=../deps

. ../gen_links.sh.inc

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Abstract_Socket/src
FILES="Abstract_Socket.cc Abstract_Socket.hh "
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.HTTPmsg/src
FILES="HTTPmsg_MessageLen.ttcn HTTPmsg_MessageLen_Function.cc HTTPmsg_PT.cc HTTPmsg_PT.hh HTTPmsg_PortType.ttcn "
FILES+="HTTPmsg_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn IPL4asp_PT.cc IPL4asp_PT.hh IPL4asp_PortType.ttcn IPL4asp_Types.ttcn "
FILES+="IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.PIPEasp/src
FILES="PIPEasp_PT.cc PIPEasp_PT.hh PIPEasp_Types.ttcn PIPEasp_PortType.ttcn "
gen_links $DIR $FILES

DIR=../library/euicc
FILES="PEDefinitions.asn PKIX1Explicit88.asn PKIX1Implicit88.asn RSPDefinitions.asn SGP32Definitions.asn "
FILES+="PKIX1Explicit88_Templates.ttcn PKIX1Explicit88_Types.ttcn PKIX1Implicit88_Templates.ttcn "
FILES+="PKIX1Implicit88_Types.ttcn RSPDefinitions_Templates.ttcn RSPDefinitions_Types.ttcn "
FILES+="SGP32Definitions_Templates.ttcn SGP32Definitions_Types.ttcn "
FILES+="PKIX1Explicit88_EncDec.cc PKIX1Implicit88_EncDec.cc RSPDefinitions_EncDec.cc SGP32Definitions_EncDec.cc "
FILES+="es9p_Types_JSON.ttcn esx_header_Types_JSON.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="HTTP_Server_Emulation.ttcn HTTP_Adapter.ttcn"
gen_links $DIR $FILES

ignore_pp_results
