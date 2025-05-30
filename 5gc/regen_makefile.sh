#!/bin/sh -e

NAME=C5G_Tests

FILES="
	*.asn
	*.c
	*.ttcn
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	common_ext.cc
	Milenage_FunctionDefs.cc
	NGAP_CodecPort_CtrlFunctDef.cc
	NGAP_EncDec.cc
	NG_CryptoFunctionDefs.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lfftranscode -lgnutls/' Makefile
