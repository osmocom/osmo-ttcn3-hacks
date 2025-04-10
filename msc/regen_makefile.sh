#!/bin/sh -e

NAME=MSC_Tests

FILES="
	*.asn
	*.c
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	MAP_EncDec.cc
	MGCP_CodecPort_CtrlFunctDef.cc
	MNCC_EncDec.cc
	Native_FunctionDefs.cc
	RANAP_EncDec.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	SCCP_EncDec.cc
	SCTPasp_PT.cc
	SDP_EncDec.cc
	SGsAP_CodecPort_CtrlFunctDef.cc
	SMPP_CodecPort_CtrlFunctDef.cc
	SMPP_EncDec.cc
	SS_EncDec.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TCCOpenSecurity.cc
	TELNETasp_PT.cc
	UD_PT.cc
"

CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_GSUP
	-DIPA_EMULATION_MGCP
	-DIPA_EMULATION_SCCP
	-DRAN_EMULATION_BSSAP
	-DRAN_EMULATION_CTRL
	-DRAN_EMULATION_MGCP
	-DRAN_EMULATION_RANAP
	-DUSE_MTP3_DISTRIBUTOR
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lfftranscode -lssl/' Makefile
