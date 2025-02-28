#!/bin/sh

NAME=smdpp_Tests

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
	PIPEasp_PT.cc
"
../regen-makefile.sh smdpp_Tests.ttcn $FILES

# required for forkpty(3) used by PIPEasp
sed -i -e '/^LINUX_LIBS/ s/$/ -lutil/' Makefile
