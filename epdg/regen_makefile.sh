#!/bin/sh -e

NAME=EPDG_Tests

FILES="
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	IP_EncDec.cc
	ICMP_EncDec.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	DIAMETER_EncDec.cc
	DIAMETER_CodecPort_CtrlFunctDef.cc
	UECUPS_CodecPort_CtrlFunctDef.cc
	GTPU_EncDec.cc
	GTPv1U_CodecPort_CtrlFunctDef.cc
	GTPv2_CodecPort_CtrlFunctDef.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_GSUP
"

. ../_buildsystem/regen_makefile.inc.sh
