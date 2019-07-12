#!/bin/sh

FILES="*.ttcn *.asn *.c IPL4asp_PT.cc  IPL4asp_discovery.cc  Native_FunctionDefs.cc SGsAP_CodecPort_CtrlFunctDef.cc  TCCConversion.cc  TCCEncoding.cc  TCCInterface.cc  TELNETasp_PT.cc S1AP_EncDec.cc LTE_CryptoFunctionDefs.cc "

export CPPFLAGS_TTCN3=""

../regen-makefile.sh MME_Tests.ttcn $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lfftranscode -lgnutls/' Makefile
