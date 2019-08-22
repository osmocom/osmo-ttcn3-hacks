#!/bin/sh

FILES="*.ttcn *.ttcnpp IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc Native_FunctionDefs.cc TCCConversion.cc TCCEncoding.cc TCCInterface.cc TELNETasp_PT.cc DIAMETER_EncDec.cc DIAMETER_CodecPort_CtrlFunctDef.cc "

export CPPFLAGS_TTCN3="-DIPA_EMULATION_GSUP -DIPA_EMULATION_CTRL"

../regen-makefile.sh DIA2GSUP_Tests.ttcn $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lfftranscode -lgnutls/' Makefile
