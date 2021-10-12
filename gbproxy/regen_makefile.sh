#!/bin/bash

NAME=GBProxy_Tests

FILES="
	*.ttcn
	*.ttcnpp
	AF_PACKET_PT.cc
	AF_PACKET_PT.hh
	BSSGP_EncDec.cc
	GTPC_EncDec.cc
	GTPU_EncDec.cc
	GTP_CodecPort_CtrlFunctDef.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	LLC_EncDec.cc
	Native_FunctionDefs.cc
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
	-DNS_EMULATION_FR
	-DUSE_MTP3_DISTRIBUTOR
"

../regen-makefile.sh -e $NAME $FILES

sed -i -i 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lsctp/' Makefile
