#!/bin/sh

FILES="*.ttcn IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc IPL4_GSMTAP_CtrlFunctDef.cc TELNETasp_PT.cc"

../regen-makefile.sh Test.ttcn $FILES
