#!/bin/sh

FILES="*.ttcn BSSGP_EncDec.cc LLC_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc NS_CodecPort_CtrlFunctDef.cc RLCMAC_EncDec.cc Native_FunctionDefs.cc SDP_EncDec.cc SDP_parse_.tab.c lex.SDP_parse_.c TELNETasp_PT.cc IPA_CodecPort_CtrlFunctDef.cc"

../regen-makefile.sh SGSN_Tests.ttcn $FILES
