#!/bin/sh -e

NAME=HLR_Tests

FILES="
	*.asn
	*.ttcn
	*.ttcnpp
	DNS_EncDec.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	MAP_EncDec.cc
	SS_EncDec.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	UDPasp_PT.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_GSUP
	-DIPA_EMULATION_CTRL
"

. ../_buildsystem/regen_makefile.inc.sh
