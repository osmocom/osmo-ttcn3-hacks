#!/bin/sh

FILES="*.ttcn *.ttcnpp IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc UD_PT.cc RLCMAC_EncDec.cc Native_FunctionDefs.cc TRXC_CodecPort_CtrlFunctDef.cc L1CTL_PortType_CtrlFunctDef.cc TELNETasp_PT.cc"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_RSL -DIPA_EMULATION_OML -DIPA_EMULATION_CTRL"

../regen-makefile.sh BTS_Tests.ttcn $FILES
