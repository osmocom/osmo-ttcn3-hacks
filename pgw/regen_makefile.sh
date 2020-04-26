#!/bin/bash

FILES="*.ttcn IPL4asp_PT.cc  IPL4asp_discovery.cc  TCCConversion.cc  TCCInterface.cc GTPC_EncDec.cc GTPU_EncDec.cc GTP_CodecPort_CtrlFunctDef.cc GTPv2_CodecPort_CtrlFunctDef.cc ICMPv6_EncDec.cc IP_EncDec.cc Native_FunctionDefs.cc UDP_EncDec.cc ICMP_EncDec.cc "
FILES+="UECUPS_CodecPort_CtrlFunctDef.cc "

../regen-makefile.sh PGW_Tests.ttcn $FILES
