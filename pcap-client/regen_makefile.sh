#!/bin/bash

NAME=OPCAP_CLIENT_Tests

FILES="
	*.ttcn
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	OPCAP_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

export CPPFLAGS_TTCN3=""

../_buildsystem/regen-makefile.sh -e $NAME $FILES
