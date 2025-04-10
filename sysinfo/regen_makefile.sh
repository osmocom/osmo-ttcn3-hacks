#!/bin/sh -e

NAME=Test

FILES="
	*.ttcn
	IPL4_GSMTAP_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

. ../_buildsystem/regen_makefile.inc.sh
