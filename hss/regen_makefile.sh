#!/bin/bash

NAME=HSS_Tests

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

export CPPFLAGS_TTCN3="
"

../_buildsystem/regen-makefile.sh -e $NAME $FILES
