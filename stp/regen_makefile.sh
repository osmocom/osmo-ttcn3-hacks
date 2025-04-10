#!/bin/sh -e

NAME=STP_Tests

FILES="
	*.ttcn
	*.ttcnpp
	SCCP_EncDec.cc
	SCTPasp_PT.cc
	TCCConversion.cc
	TCCInterface.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IPA_CodecPort_CtrlFunctDef.cc
	TELNETasp_PT.cc
	Native_FunctionDefs.cc
	TCCEncoding.cc
	M3UA_CodecPort_CtrlFunctDef.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_SCCP
	-DUSE_MTP3_DISTRIBUTOR
"

. ../_buildsystem/regen_makefile.inc.sh
