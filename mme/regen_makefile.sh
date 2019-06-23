#!/bin/sh

FILES="*.ttcn IPL4asp_PT.cc  IPL4asp_discovery.cc  Native_FunctionDefs.cc SGsAP_CodecPort_CtrlFunctDef.cc  TCCConversion.cc  TCCEncoding.cc  TCCInterface.cc  TELNETasp_PT.cc "

export CPPFLAGS_TTCN3=""

../regen-makefile.sh MME_Tests.ttcn $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lfftranscode/' Makefile
