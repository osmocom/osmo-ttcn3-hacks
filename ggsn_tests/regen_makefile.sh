#!/bin/sh

FILES="*.ttcn IPL4asp_PT.cc  IPL4asp_discovery.cc  TCCConversion.cc  TCCInterface.cc GTPC_EncDec.cc GTPU_EncDec.cc GTP_CodecPort_CtrlFunctDef.cc ICMPv6_EncDec.cc IP_EncDec.cc Native_FunctionDefs.cc UDP_EncDec.cc"

ttcn3_makefilegen -l -f GGSN_Tests.ttcn $FILES
sed -i -e 's/# TTCN3_DIR = /TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/LDFLAGS = /LDFLAGS = -L \/usr\/lib\/titan /' Makefile
#sed -i -e 's/TTCN3_LIB = ttcn3-parallel/TTCN3_LIB = ttcn3/' Makefile
sed -i -e 's/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include -I\/usr\/include\/titan/' Makefile
