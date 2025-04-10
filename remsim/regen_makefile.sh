#!/bin/sh -e

NAME=REMSIM_Tests

FILES="
	*.asn
	*.ttcn
	*.ttcnpp
	Abstract_Socket.cc
	HTTPmsg_MessageLen_Function.cc
	HTTPmsg_PT.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	JSON_EncDec.cc
	Native_FunctionDefs.cc
	PIPEasp_PT.cc
	RSPRO_EncDec.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	VPCD_CodecPort_CtrlFunctDef.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_RSPRO
	-DIPA_EMULATION_CTRL
"

. ../_buildsystem/regen_makefile.inc.sh

# required for forkpty(3) used by PIPEasp
sed -i -e '/^LINUX_LIBS/ s/$/ -lutil/' Makefile
