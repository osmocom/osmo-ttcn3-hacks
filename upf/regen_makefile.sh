#!/bin/sh

NAME=UPF_Tests

FILES="
	*.ttcn
	*.ttcnpp
	*.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

export CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
"

../regen-makefile.sh -e $NAME $FILES
