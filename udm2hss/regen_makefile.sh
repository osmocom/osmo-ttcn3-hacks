#!/bin/sh

NAME=UDM2HSS_Tests

FILES="
	*.ttcn
	*.asn
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
	PIPEasp_PT.cc
	DIAMETER_EncDec.cc
	DIAMETER_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh -e $NAME $FILES
