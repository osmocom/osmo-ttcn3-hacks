#!/bin/sh -e

NAME=SMF_Tests

FILES=" *.asn
	*.ttcn
	*.ttcnpp
	BSSGP_EncDec.cc
	DIAMETER_CodecPort_CtrlFunctDef.cc
	DIAMETER_EncDec.cc
	GTPC_EncDec.cc
	GTPU_EncDec.cc
	GTPv1C_CodecPort_CtrlFunctDef.cc
	GTPv1U_CodecPort_CtrlFunctDef.cc
	GTPv2_CodecPort_CtrlFunctDef.cc
	HTTP2_EncDec.cc
	HTTP2_CodecPort_CtrlFunctDef.cc
	ICMP_EncDec.cc
	ICMPv6_EncDec.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IP_EncDec.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCDateTime.cc
	TCCInterface.cc
	TCCEncoding.cc
	UDP_EncDec.cc
	PFCP_CodecPort_CtrlFunctDef.cc
"

CPPFLAGS_TTCN3=""

. ../_buildsystem/regen_makefile.inc.sh
