#!/bin/sh -e

NAME=SIP_Tests

FILES="
	*.c
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	MNCC_EncDec.cc
	Native_FunctionDefs.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	SDP_EncDec.cc
	SIPmsg_PT.cc
	TCCConversion.cc
	TCCInterface.cc
	TCCOpenSecurity.cc
	TCCDateTime.cc
	TELNETasp_PT.cc
	UD_PT.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DRAN_EMULATION_CTRL
"

. ../_buildsystem/regen_makefile.inc.sh
