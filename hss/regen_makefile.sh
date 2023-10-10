#!/bin/bash

NAME=HSS_Tests

FILES="
	*.ttcn
	DIAMETER_CodecPort_CtrlFunctDef.cc
	DIAMETER_EncDec.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh -e $NAME $FILES
