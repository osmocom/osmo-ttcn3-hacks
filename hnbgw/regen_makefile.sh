#!/bin/sh

NAME=HNBGW_Tests

FILES="
	*.asn
	*.c
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IuUP_EncDec.cc
	Iuh_CodecPort_CtrlFunctDef.cc
	Native_FunctionDefs.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	SCTPasp_PT.cc
	SDP_EncDec.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	HNBAP_EncDec.cc
	RUA_EncDec.cc
	RANAP_EncDec.cc
	MGCP_CodecPort_CtrlFunctDef.cc
	UD_PT.cc
"

export CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DRAN_EMULATION_RANAP
	-DUSE_MTP3_DISTRIBUTOR
"

../regen-makefile.sh -e $NAME $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lfftranscode/' Makefile
