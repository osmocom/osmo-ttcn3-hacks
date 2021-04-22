#!/bin/bash

FILES="*.ttcn TCCConversion.cc TCCInterface.cc TCCEncoding.cc IPL4asp_PT.cc IPL4asp_discovery.cc TELNETasp_PT.cc Native_FunctionDefs.cc OPCAP_CodecPort_CtrlFunctdef.cc "
#FILES+="*.ttcnpp "
#FILES+="*.asn"

export CPPFLAGS_TTCN3=""

../regen-makefile.sh OPCAP_CLIENT_Tests.ttcn $FILES

#sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lfftranscode/' Makefile
