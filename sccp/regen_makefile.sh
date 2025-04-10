#!/bin/sh -e

NAME=SCCP_Tests

FILES="
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	SCCP_EncDec.cc
	SCTPasp_PT.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	UD_PT.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_SCCP
	-DUSE_MTP3_DISTRIBUTOR
"

. ../_buildsystem/regen_makefile.inc.sh
