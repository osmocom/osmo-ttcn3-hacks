#!/bin/sh

MAIN=BSC_Tests.ttcn

FILES="*.ttcn SCCP_EncDec.cc IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc RTP_EncDec.cc SDP_EncDec.cc *.c MGCP_CodecPort_CtrlFunctDef.cc"

ttcn3_makefilegen -l -f $MAIN $FILES
sed -i -e 's/# TTCN3_DIR = /TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/LDFLAGS = /LDFLAGS = -L \/usr\/lib\/titan /' Makefile
#sed -i -e 's/TTCN3_LIB = ttcn3-parallel/TTCN3_LIB = ttcn3/' Makefile
sed -i -e 's/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include -I\/usr\/include\/titan/' Makefile
