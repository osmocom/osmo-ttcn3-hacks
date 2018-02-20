#!/bin/sh

MAIN=Selftest.ttcn

FILES="*.ttcn *.ttcnpp IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_RSL"

../regen-makefile.sh $MAIN $FILES
