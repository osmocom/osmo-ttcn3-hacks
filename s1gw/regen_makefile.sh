#!/bin/sh -e

NAME=S1GW_Tests

FILES="
	*.asn
	*.ttcn
	*.ttcnpp
	Abstract_Socket.cc
	HTTPmsg_MessageLen_Function.cc
	HTTPmsg_PT.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	PFCP_CodecPort_CtrlFunctDef.cc
	S1AP_CodecPort_CtrlFunctDef.cc
	S1AP_EncDec.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCDateTime.cc
	TCCEncoding.cc
	TCCInterface.cc
"

# https://gitlab.eclipse.org/eclipse/titan/titan.core/-/work_items/798
REGEN_MAKEFILE_ADD_TTCN3_COMPILER_FLAGS_IF_SUPPORTED="
	--legacy-template-subrefs
"

. ../_buildsystem/regen_makefile.inc.sh
