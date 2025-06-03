#!/bin/sh -e

NAME=MME_Tests

FILES="
	*.asn
	*.c
	*.ttcn
	BSSGP_EncDec.cc
	DIAMETER_CodecPort_CtrlFunctDef.cc
	DIAMETER_EncDec.cc
	GTPC_EncDec.cc
	GTPU_EncDec.cc
	GTPv1C_CodecPort_CtrlFunctDef.cc
	GTPv1U_CodecPort_CtrlFunctDef.cc
	GTPv2_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	LTE_CryptoFunctionDefs.cc
	Native_FunctionDefs.cc
	S1AP_CodecPort_CtrlFunctDef.cc
	S1AP_EncDec.cc
	SGsAP_CodecPort_CtrlFunctDef.cc
	Snow3G_FunctionDefs.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	UECUPS_CodecPort_CtrlFunctDef.cc
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lfftranscode -lgnutls/' Makefile
