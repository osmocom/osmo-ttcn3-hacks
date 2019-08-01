#!/bin/sh

FILES="*.ttcn TCCConversion.cc TCCInterface.cc TCCEncoding.cc IPL4asp_PT.cc IPL4asp_discovery.cc TELNETasp_PT.cc Native_FunctionDefs.cc SCTPasp_PT.cc CBSP_CodecPort_CtrlFunctdef.cc SABP_EncDec.cc "
#FILES+="*.ttcnpp "
FILES+="*.asn"

export CPPFLAGS_TTCN3=""

../regen-makefile.sh CBC_Tests.ttcn $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lfftranscode/' Makefile
