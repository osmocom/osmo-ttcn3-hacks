#!/bin/sh

FILES="*.ttcn *.ttcnpp BSSGP_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc NS_CodecPort_CtrlFunctDef.cc UD_PT.cc RLCMAC_EncDec.cc LLC_EncDec.cc TELNETasp_PT.cc Native_FunctionDefs.cc StatsD_CodecPort_CtrlFunctdef.cc"

export CPPFLAGS_TTCN3="-DBSSGP_EM_L3"

../regen-makefile.sh PCU_Tests.ttcn $FILES
