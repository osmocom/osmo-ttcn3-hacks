#!/bin/sh -e

NAME=BSCNAT_Tests

FILES="
	*.c
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	MGCP_CodecPort_CtrlFunctDef.cc
	Native_FunctionDefs.cc
	RTP_EncDec.cc
	SCCP_EncDec.cc
	SDP_EncDec.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_MGCP
	-DIPA_EMULATION_SCCP
	-DRAN_EMULATION_BSSAP
	-DRAN_EMULATION_CTRL
	-DRAN_EMULATION_MGCP
	-DUSE_MTP3_DISTRIBUTOR
"

. ../_buildsystem/regen_makefile.inc.sh
