#!/bin/sh

FILES="*.ttcn *.ttcnpp BSSGP_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc UD_PT.cc RLCMAC_EncDec.cc LLC_EncDec.cc TELNETasp_PT.cc IPA_CodecPort_CtrlFunctDef.cc Native_FunctionDefs.cc StatsD_CodecPort_CtrlFunctdef.cc"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_CTRL -DBSSGP_EM_L3"

../regen-makefile.sh PCU_Tests.ttcn $FILES
