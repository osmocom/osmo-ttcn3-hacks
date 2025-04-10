#!/bin/sh -e

NAME=GBProxy_Tests

FILES="
	*.ttcn
	*.ttcnpp
	AF_PACKET_PT.cc
	AF_PACKET_PT.hh
	BSSGP_EncDec.cc
	GTPC_EncDec.cc
	GTPU_EncDec.cc
	GTPv1C_CodecPort_CtrlFunctDef.cc
	GTPv1U_CodecPort_CtrlFunctDef.cc
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	LLC_EncDec.cc
	Native_FunctionDefs.cc
	SCCP_EncDec.cc
	SCTPasp_PT.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

CPPFLAGS_TTCN3="
	-DBSSGP_EM_L3
	-DIPA_EMULATION_CTRL
	-DNS_EMULATION_FR
	-DUSE_MTP3_DISTRIBUTOR
"

. ../_buildsystem/regen_makefile.inc.sh

sed -i -i 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lsctp/' Makefile
