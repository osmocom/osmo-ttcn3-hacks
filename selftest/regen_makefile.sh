#!/bin/sh -e

NAME=Selftest

FILES="
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_RSL
	-DUSE_MTP3_DISTRIBUTOR
"

. ../_buildsystem/regen_makefile.inc.sh
