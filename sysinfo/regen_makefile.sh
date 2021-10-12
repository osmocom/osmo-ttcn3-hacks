#!/bin/sh

FILES="
	*.ttcn
	IPL4_GSMTAP_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh Test.ttcn $FILES
