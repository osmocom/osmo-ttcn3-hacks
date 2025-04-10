#!/bin/sh -e

NAME=NS_Tests

FILES="
	*.ttcn
	*.ttcnpp
	AF_PACKET_PT.cc
	AF_PACKET_PT.hh
	BSSGP_EncDec.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	LLC_EncDec.cc
	Native_FunctionDefs.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	UD_PT.cc
"

CPPFLAGS_TTCN3="
	-DBSSGP_EM_L3
	-DNS_EMULATION_FR
	-DSTATSD_HAVE_VTY
"

. ../_buildsystem/regen_makefile.inc.sh
