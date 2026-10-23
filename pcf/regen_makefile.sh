#!/bin/sh -e

NAME=PCF_Tests

FILES="
	*.asn
	*.ttcn
	Abstract_Socket.cc
	HTTPmsg_MessageLen_Function.cc
	HTTPmsg_PT.cc
	HTTP2_EncDec.cc
	HTTP2_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lgnutls/' Makefile
