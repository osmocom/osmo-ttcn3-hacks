#!/bin/sh -e

NAME=UPF_Tests

FILES="
	*.ttcn
	*.ttcnpp
	*.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DSTATSD_HAVE_VTY
"

. ../_buildsystem/regen_makefile.inc.sh
