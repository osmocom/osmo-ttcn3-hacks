#!/bin/sh

NAME=smdpp2_Tests

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
	JSON_EncDec.cc
"
../regen-makefile.sh smdpp2_Tests.ttcn smdpp2_json.ttcn3 smdpp2_es9p_types.ttcn3 $FILES

# required for forkpty(3) used by PIPEasp
sed -i -e '/^LINUX_LIBS/ s/$/ -lutil/' Makefile
