#!/bin/sh

FILES="*.ttcn *.ttcnpp BSSGP_EncDec.cc LLC_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc NS_CodecPort_CtrlFunctDef.cc RLCMAC_EncDec.cc Native_FunctionDefs.cc TELNETasp_PT.cc IPA_CodecPort_CtrlFunctDef.cc GTPU_EncDec.cc GTPC_EncDec.cc GTP_CodecPort_CtrlFunctDef.cc"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_GSUP -DIPA_EMULATION_CTRL"

../regen-makefile.sh SGSN_Tests.ttcn $FILES
