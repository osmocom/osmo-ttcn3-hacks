#!/bin/sh -e

NAME=C5G_Tests

FILES="
	*.asn
	*.ttcn
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	common_ext.cc
	NGAP_CodecPort_CtrlFunctDef.cc
	NGAP_EncDec.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lfftranscode -lgnutls/' Makefile
