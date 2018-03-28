#!/bin/sh

FILES="*.ttcn SDP_EncDec.cc *.c MGCP_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc RTP_EncDec.cc RTP_CodecPort_CtrlFunctDef.cc IuUP_EncDec.cc "

../regen-makefile.sh MGCP_Test.ttcn $FILES
