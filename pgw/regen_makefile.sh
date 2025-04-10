#!/bin/bash

NAME=PGW_Tests

FILES="
	*.ttcn
	BSSGP_EncDec.cc
	DIAMETER_CodecPort_CtrlFunctDef.cc
	DIAMETER_EncDec.cc
	GTPC_EncDec.cc
	GTPU_EncDec.cc
	GTPv1C_CodecPort_CtrlFunctDef.cc
	GTPv1U_CodecPort_CtrlFunctDef.cc
	GTPv2_CodecPort_CtrlFunctDef.cc
	ICMP_EncDec.cc
	ICMPv6_EncDec.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IP_EncDec.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
	TCCEncoding.cc
	UDP_EncDec.cc
	UECUPS_CodecPort_CtrlFunctDef.cc
"

export CPPFLAGS_TTCN3="
"

../_buildsystem/regen-makefile.sh -e $NAME $FILES
