#!/bin/sh

FILES="*.ttcn *.ttcnpp TCCConversion.cc TCCInterface.cc UD_PT.cc MNCC_EncDec.cc IPL4asp_PT.cc
IPL4asp_discovery.cc SDP_EncDec.cc RTP_EncDec.cc IPA_CodecPort_CtrlFunctDef.cc RTP_CodecPort_CtrlFunctDef.cc TELNETasp_PT.cc Native_FunctionDefs.cc SIPmsg_PT.cc *.c "

export CPPFLAGS_TTCN3="-DIPA_EMULATION_CTRL -DRAN_EMULATION_CTRL"

../regen-makefile.sh SIP_Tests.ttcn $FILES
