#!/bin/sh

FILES="*.ttcn *.ttcnpp *.asn IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc TELNETasp_PT.cc Native_FunctionDefs.cc RSPRO_EncDec.cc Abstract_Socket.cc HTTPmsg_PT.cc HTTPmsg_MessageLen_Function.cc JSON_EncDec.cc VPCD_CodecPort_CtrlFunctDef.cc PIPEasp_PT.cc "

export CPPFLAGS_TTCN3="-DIPA_EMULATION_RSPRO -DIPA_EMULATION_CTRL"

../regen-makefile.sh REMSIM_Tests.ttcn $FILES

# required for forkpty(3) used by PIPEasp
sed -i -e '/^LINUX_LIBS/ s/$/ -lutil/' Makefile
