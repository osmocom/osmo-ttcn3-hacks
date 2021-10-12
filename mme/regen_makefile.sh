#!/bin/sh

FILES="
	*.asn
	*.c
	*.ttcn
	DIAMETER_CodecPort_CtrlFunctDef.cc
	DIAMETER_EncDec.cc
	GTPC_EncDec.cc
	GTPU_EncDec.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	LTE_CryptoFunctionDefs.cc
	Native_FunctionDefs.cc
	S1AP_CodecPort_CtrlFunctDef.cc
	S1AP_EncDec.cc
	SGsAP_CodecPort_CtrlFunctDef.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh MME_Tests.ttcn $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lfftranscode/' Makefile
