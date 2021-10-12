#!/bin/sh

FILES="
	*.ttcn
	*.ttcnpp
	BSSGP_EncDec.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	LLC_EncDec.cc
	Native_FunctionDefs.cc
	RLCMAC_EncDec.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	UD_PT.cc
"

export CPPFLAGS_TTCN3="
	-DBSSGP_EM_L3
	-DIPA_EMULATION_CTRL
"

../regen-makefile.sh PCU_Tests.ttcn $FILES
