#!/bin/sh -e

NAME=BTS_Tests

FILES="
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IuUP_EncDec.cc
	L1CTL_PortType_CtrlFunctDef.cc
	Native_FunctionDefs.cc
	OSMUX_CodecPort_CtrlFunctDef.cc
	RLCMAC_EncDec.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	TRXC_CodecPort_CtrlFunctDef.cc
	UD_PT.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_OML
	-DIPA_EMULATION_OSMO_PCU
	-DIPA_EMULATION_RSL
"

. ../_buildsystem/regen_makefile.inc.sh
