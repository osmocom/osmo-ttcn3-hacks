#!/bin/sh

FILES="*.ttcn *.ttcnpp IPA_CodecPort_CtrlFunctDef.cc SDP_EncDec.cc *.c MGCP_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc RTP_EncDec.cc RTP_CodecPort_CtrlFunctDef.cc OSMUX_CodecPort_CtrlFunctDef.cc IuUP_EncDec.cc Native_FunctionDefs.cc TELNETasp_PT.cc IP_EncDec.cc StatsD_CodecPort_CtrlFunctdef.cc "

export CPPFLAGS_TTCN3="-DIPA_EMULATION_CTRL"

../regen-makefile.sh MGCP_Test.ttcn $FILES
