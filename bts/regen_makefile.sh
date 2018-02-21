#!/bin/sh

FILES="*.ttcn *.ttcnpp IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc
TCCInterface.cc UD_PT.cc RLCMAC_EncDec.cc Native_FunctionDefs.cc TRXC_CodecPort_CtrlFunctDef.cc"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_RSL"

../regen-makefile.sh BTS_Tests.ttcn $FILES
