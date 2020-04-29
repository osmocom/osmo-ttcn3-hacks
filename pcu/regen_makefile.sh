#!/bin/sh

FILES="*.ttcn BSSGP_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc NS_CodecPort_CtrlFunctDef.cc UD_PT.cc RLCMAC_EncDec.cc LLC_EncDec.cc TELNETasp_PT.cc Native_FunctionDefs.cc"

../regen-makefile.sh PCU_Tests.ttcn $FILES
