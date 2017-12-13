#!/bin/sh

MAIN=IPA_Test.ttcn

FILES="*.ttcn SCCP_EncDec.cc IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc RTP_EncDec.cc SDP_EncDec.cc *.c MGCP_CodecPort_CtrlFunctDef.cc"

../regen-makefile.sh $MAIN $FILES
