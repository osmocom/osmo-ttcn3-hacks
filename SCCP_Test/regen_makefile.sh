#!/bin/sh

NAME=SCCP_Testcases

FILES="
	*.ttcn
	*.ttcnpp
"

export CPPFLAGS_TTCN3="
"

../regen-makefile.sh -e $NAME $FILES
