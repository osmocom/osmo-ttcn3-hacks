#!/bin/bash

FILES="*.ttcn *.ttcnpp BSSGP_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc UD_PT.cc RLCMAC_EncDec.cc LLC_EncDec.cc TELNETasp_PT.cc Native_FunctionDefs.cc StatsD_CodecPort_CtrlFunctdef.cc "

FILES+="AF_PACKET_PT.cc AF_PACKET_PT.hh "

export CPPFLAGS_TTCN3="-DBSSGP_EM_L3 -DNS_EMULATION_FR"

../regen-makefile.sh NS_Tests.ttcn $FILES
