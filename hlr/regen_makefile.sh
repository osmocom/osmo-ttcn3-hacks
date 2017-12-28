#!/bin/sh

MAIN="GSUP_Test.ttcn"

FILES="*.ttcn IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc SDP_EncDec.cc MGCP_CodecPort_CtrlFunctDef.cc *.c"

../regen-makefile.sh $MAIN $FILES
