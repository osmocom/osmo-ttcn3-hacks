#!/bin/sh

MAIN=Selftest.ttcn

FILES="*.ttcn IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc SDP_EncDec.cc *.c"

../regen-makefile.sh $MAIN $FILES
