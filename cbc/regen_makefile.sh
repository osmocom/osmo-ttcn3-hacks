#!/bin/sh -e

NAME=CBC_Tests

FILES="
	*.asn
	*.ttcn
	Abstract_Socket.cc
	CBSP_CodecPort_CtrlFunctdef.cc
	HTTPmsg_MessageLen_Function.cc
	HTTPmsg_PT.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	JSON_EncDec.cc
	Native_FunctionDefs.cc
	SABP_CodecPort_CtrlFunctDef.cc
	SABP_EncDec.cc
	SBC_AP_CodecPort_CtrlFunctDef.cc
	SBC_AP_EncDec.cc
	SCTPasp_PT.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lfftranscode/' Makefile
