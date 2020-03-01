#!/bin/sh

FILES="*.ttcn IPL4asp_PT.cc  IPL4asp_discovery.cc  Native_FunctionDefs.cc TCCConversion.cc  TCCEncoding.cc TCCInterface.cc  TELNETasp_PT.cc DIAMETER_EncDec.cc DIAMETER_CodecPort_CtrlFunctDef.cc "

export CPPFLAGS_TTCN3=""

../regen-makefile.sh DIAMETER_Tests.ttcn $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lfftranscode -lgnutls/' Makefile
