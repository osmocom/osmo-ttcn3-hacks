#!/bin/sh -e

NAME=PCRF_Tests

FILES="
	*.ttcn
	Abstract_Socket.cc
	DIAMETER_CodecPort_CtrlFunctDef.cc
	DIAMETER_EncDec.cc
	HTTPmsg_MessageLen_Function.cc
	HTTPmsg_PT.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
"

. ../_buildsystem/regen_makefile.inc.sh
