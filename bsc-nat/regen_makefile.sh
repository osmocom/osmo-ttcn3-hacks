#!/bin/sh

MAIN=IPA_Test.ttcn

FILES="*.ttcn *.ttcnpp SCCP_EncDec.cc IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc RTP_EncDec.cc SDP_EncDec.cc *.c MGCP_CodecPort_CtrlFunctDef.cc"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_SCCP -DIPA_EMULATION_MGCP"

../regen-makefile.sh $MAIN $FILES
