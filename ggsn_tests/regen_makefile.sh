#!/bin/sh

FILES="
	*.ttcn
	BSSGP_EncDec.cc
	GTPC_EncDec.cc
	GTPU_EncDec.cc
	GTP_CodecPort_CtrlFunctDef.cc
	ICMP_EncDec.cc
	ICMPv6_EncDec.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IP_EncDec.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	UDP_EncDec.cc
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh GGSN_Tests.ttcn $FILES
