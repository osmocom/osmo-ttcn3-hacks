#!/bin/bash

NAME=FR_Tests

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
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

export CPPFLAGS_TTCN3="
	-DNS_EMULATION_FR
"

../_buildsystem/regen-makefile.sh -e $NAME $FILES
