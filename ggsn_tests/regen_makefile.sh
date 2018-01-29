#!/bin/sh

FILES="*.ttcn IPL4asp_PT.cc  IPL4asp_discovery.cc  TCCConversion.cc  TCCInterface.cc GTPC_EncDec.cc GTPU_EncDec.cc GTP_CodecPort_CtrlFunctDef.cc ICMPv6_EncDec.cc IP_EncDec.cc Native_FunctionDefs.cc UDP_EncDec.cc ICMP_EncDec.cc"

../regen-makefile.sh GGSN_Tests.ttcn $FILES
