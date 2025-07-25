#!/bin/sh -e

NAME=C5G_Tests

FILES="
	*.asn
	*.c
	*.ttcn
	*.ttcnpp
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	common_ext.cc
	Milenage_FunctionDefs.cc
	NGAP_CodecPort_CtrlFunctDef.cc
	NGAP_EncDec.cc
	NG_CryptoFunctionDefs.cc
	Snow3G_FunctionDefs.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	GTPU_EncDec.cc
	GTPv1U_CodecPort_CtrlFunctDef.cc
	UECUPS_CodecPort_CtrlFunctDef.cc
"

CPPFLAGS_TTCN3="
	-DGTP1U_EMULATION_HAVE_UECUPS
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lgnutls/' Makefile
