#!/bin/sh -e

NAME=DIA2GSUP_Tests

FILES="
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	DIAMETER_EncDec.cc
	DIAMETER_CodecPort_CtrlFunctDef.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_GSUP
	-DIPA_EMULATION_CTRL
"

. ../_buildsystem/regen_makefile.inc.sh
