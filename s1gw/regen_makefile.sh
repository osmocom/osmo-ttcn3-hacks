#!/bin/sh

NAME=S1GW_Tests

FILES="
	*.asn
	*.ttcn
	*.ttcnpp
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	PFCP_CodecPort_CtrlFunctDef.cc
	S1AP_CodecPort_CtrlFunctDef.cc
	S1AP_EncDec.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
"


export CPPFLAGS_TTCN3="
"

../_buildsystem/regen-makefile.sh -e $NAME $FILES

sed -i -e '/^CPPFLAGS/ s/$/ `pkg-config --cflags libfftranscode`/' Makefile
sed -i -e '/^LDFLAGS/ s/$/ `pkg-config --libs libfftranscode`/' Makefile
sed -i -e '/^LINUX_LIBS/ s/$/ `pkg-config --libs libfftranscode`/' Makefile
