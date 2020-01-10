#!/bin/sh

MAIN=SCCP_Testcases.ttcn

FILES="*.ttcn *.ttcnpp"

../regen-makefile.sh $MAIN $FILES
