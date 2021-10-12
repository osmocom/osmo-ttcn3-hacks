#!/bin/sh

MAIN=BSC_Tests.ttcn

FILES="
	*.c
	*.ttcn
	*.ttcnpp
	CBSP_CodecPort_CtrlFunctdef.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IuUP_EncDec.cc
	MGCP_CodecPort_CtrlFunctDef.cc
	Native_FunctionDefs.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	SCTPasp_PT.cc
	SDP_EncDec.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

export CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_MGCP
	-DIPA_EMULATION_RSL
	-DIPA_EMULATION_SCCP
	-DRAN_EMULATION_BSSAP
	-DRAN_EMULATION_CTRL
	-DRAN_EMULATION_MGCP
	-DUSE_MTP3_DISTRIBUTOR
"

../regen-makefile.sh $MAIN $FILES
