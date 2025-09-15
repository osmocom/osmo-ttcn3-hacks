#!/bin/sh -e

NAME=STP_Tests

FILES="
	*.asn
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
	TCAP_EncDec.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_SCCP
	-DIPA_EMULATION_TCAP_ROUTING
	-DUSE_MTP3_DISTRIBUTOR
"

. ../_buildsystem/regen_makefile.inc.sh
