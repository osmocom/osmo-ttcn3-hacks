#!/bin/bash

FILES="*.ttcn IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc "
FILES+="RLAPD_CodecPort_CtrlFunctDef.cc "

../regen-makefile.sh LAPD_Tests.ttcn $FILES
