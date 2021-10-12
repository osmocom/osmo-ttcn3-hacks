#!/bin/sh

FILES="
	*.asn
	*.ttcn
	*.ttcnpp
	BSSGP_EncDec.cc
	GTPC_EncDec.cc
	GTPU_EncDec.cc
	GTP_CodecPort_CtrlFunctDef.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	LLC_EncDec.cc
	Native_FunctionDefs.cc
	RANAP_EncDec.cc
	RLCMAC_EncDec.cc
	SCCP_EncDec.cc
	SCTPasp_PT.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

export CPPFLAGS_TTCN3="
	-DBSSGP_EM_L3
	-DIPA_EMULATION_CTRL
	-DIPA_EMULATION_GSUP
	-DRAN_EMULATION_RANAP
	-DUSE_MTP3_DISTRIBUTOR
"

../regen-makefile.sh SGSN_Tests.ttcn $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lfftranscode/' Makefile
