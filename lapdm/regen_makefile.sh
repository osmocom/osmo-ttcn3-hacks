#!/bin/sh

FILES="*.ttcn UD_PT.cc UD_PT.hh RLCMAC_EncDec.cc"

../regen-makefile.sh L1CTL_Test.ttcn $FILES
