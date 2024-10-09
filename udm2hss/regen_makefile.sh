#!/bin/sh

NAME=UDM2HSS_Tests

FILES="
	*.ttcn
	*.asn
	Native_FunctionDefs.cc
	TCCConversion.cc
	TCCInterface.cc
	HTTP2_EncDec.cc
	PIPEasp_PT.cc
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh -e $NAME $FILES
