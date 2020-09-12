#!/bin/bash

FILES="*.ttcn *.ttcnpp BSSGP_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc "
FILES+="AF_PACKET_PT.cc "
FILES+="Native_FunctionDefs.cc "
FILES+="NS_CodecPort_CtrlFunctDef.cc LLC_EncDec.cc LLC_Types.cc TELNETasp_PT.cc "

export CPPFLAGS_TTCN3="-DNS_EMULATION_FR"

../regen-makefile.sh FRNET_Tests.ttcn $FILES
