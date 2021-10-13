#!/bin/bash

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
	RLCMAC_EncDec.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
	UD_PT.cc
"

export CPPFLAGS_TTCN3="
	-DBSSGP_EM_L3
	-DNS_EMULATION_FR
"

../regen-makefile.sh -e $NAME $FILES
