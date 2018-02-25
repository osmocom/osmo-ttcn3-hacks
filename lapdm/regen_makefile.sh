#!/bin/sh

FILES="*.ttcn UD_PT.cc UD_PT.hh RLCMAC_EncDec.cc L1CTL_PortType_CtrlFunctDef.cc"

../regen-makefile.sh L1CTL_Test.ttcn $FILES
