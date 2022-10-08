#!/bin/sh

NAME=GprsModem_Tests

FILES="
	*.ttcn
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	L1CTL_PortType_CtrlFunctDef.cc
	Native_FunctionDefs.cc
	RLCMAC_EncDec.cc
	TCCConversion.cc
	TCCInterface.cc
	UD_PT.cc
"

../regen-makefile.sh -e $NAME $FILES
