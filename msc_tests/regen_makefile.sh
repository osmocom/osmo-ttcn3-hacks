#!/bin/sh

FILES="*.ttcn SCCP_EncDec.cc  SCTPasp_PT.cc  TCCConversion.cc TCCInterface.cc"

../regen-makefile.sh MSC_Tests.ttcn $FILES
