#!/bin/sh

MAIN=SCCP_Testcases.ttcn

FILES="
	*.ttcn
	*.ttcnpp
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh $MAIN $FILES
