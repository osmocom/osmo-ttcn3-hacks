#!/bin/sh -e

NAME=S1GW_Tests

FILES="
	*.asn
	*.ttcn
	*.ttcnpp
	Abstract_Socket.cc
	HTTPmsg_MessageLen_Function.cc
	HTTPmsg_PT.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	JSON_EncDec.cc
	Native_FunctionDefs.cc
	PFCP_CodecPort_CtrlFunctDef.cc
	S1AP_CodecPort_CtrlFunctDef.cc
	S1AP_EncDec.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
"

. ../_buildsystem/regen_makefile.inc.sh
