#!/bin/sh -e

NAME=AMF_Tests

FILES="
	*.asn
	*.c
	*.ttcn
	Abstract_Socket.cc
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
	HTTPmsg_MessageLen_Function.cc
	HTTPmsg_PT.cc
	HTTP2_EncDec.cc
	HTTP2_CodecPort_CtrlFunctDef.cc
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lgnutls/' Makefile
