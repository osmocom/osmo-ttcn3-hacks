#!/bin/sh

MAIN="HLR_Tests.ttcn"

FILES="*.ttcn *.ttcnpp IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_GSUP"

../regen-makefile.sh $MAIN $FILES
