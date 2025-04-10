#!/bin/sh -e

NAME=SMLC_Tests

FILES="
	*.c
	*.ttcn
	*.ttcnpp
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
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_RSL
	-DIPA_EMULATION_MGCP
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_SCCP
	-DRAN_EMULATION_BSSAP
	-DRAN_EMULATION_MGCP
	-DRAN_EMULATION_CTRL
	-DUSE_MTP3_DISTRIBUTOR
"

. ../_buildsystem/regen_makefile.inc.sh
