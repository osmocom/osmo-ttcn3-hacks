#!/bin/sh

FILES="
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IuUP_EncDec.cc
	L1CTL_PortType_CtrlFunctDef.cc
	Native_FunctionDefs.cc
	RLCMAC_EncDec.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	TRXC_CodecPort_CtrlFunctDef.cc
	UD_PT.cc
"

export CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_OML
	-DIPA_EMULATION_OSMO_PCU
	-DIPA_EMULATION_RSL
"

../regen-makefile.sh BTS_Tests.ttcn $FILES
