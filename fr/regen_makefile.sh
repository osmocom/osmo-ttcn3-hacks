#!/bin/sh

FILES="*.ttcn *.ttcnpp IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc Native_FunctionDefs.cc "
FILES+="BSSGP_EncDec.cc NS_CodecPort_CtrlFunctDef.cc LLC_EncDec.cc TELNETasp_PT.cc "
FILES+="AF_PACKET_PT.cc AF_PACKET_PT.hh "

export CPPFLAGS_TTCN3="-DNS_EMULATION_FR"

../regen-makefile.sh FR_Tests.ttcn $FILES
