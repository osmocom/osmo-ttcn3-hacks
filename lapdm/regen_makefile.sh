#!/bin/sh

FILES="
	*.ttcn
	L1CTL_PortType_CtrlFunctDef.cc
	RLCMAC_EncDec.cc
	UD_PT.cc
	UD_PT.hh
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh L1CTL_Test.ttcn $FILES
