#!/bin/sh -e

NAME=IPAd_Tests

FILES="
	*.ttcn
	*.asn
	Abstract_Socket.cc
	HTTPmsg_MessageLen_Function.cc
	HTTPmsg_PT.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
	SGP32Definitions_EncDec.cc
	RSPDefinitions_EncDec.cc
	PKIX1Explicit88_EncDec.cc
	PKIX1Implicit88_EncDec.cc
	VPCD_CodecPort_CtrlFunctDef.cc
"
. ../_buildsystem/regen_makefile.inc.sh

# required for forkpty(3) used by PIPEasp
sed -i -e '/^LINUX_LIBS/ s/$/ -lutil/' Makefile
