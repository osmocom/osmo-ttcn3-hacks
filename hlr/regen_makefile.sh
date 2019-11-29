#!/bin/sh

MAIN="HLR_Tests.ttcn"

FILES="*.ttcn *.ttcnpp DNS_EncDec.cc IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc TELNETasp_PT.cc TCCEncoding.cc UDPasp_PT.cc SS_EncDec.cc MAP_EncDec.cc *.asn"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_GSUP -DIPA_EMULATION_CTRL"

../regen-makefile.sh $MAIN $FILES
