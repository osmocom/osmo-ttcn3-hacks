#!/bin/sh -e

NAME=FRNET_Tests

FILES="
	*.ttcn
	*.ttcnpp
	AF_PACKET_PT.cc
	BSSGP_EncDec.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	LLC_EncDec.cc
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

CPPFLAGS_TTCN3="
	-DNS_EMULATION_FR
"

. ../_buildsystem/regen_makefile.inc.sh
