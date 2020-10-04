#!/bin/sh

FILES="*.ttcn *.ttcnpp *.asn BSSGP_EncDec.cc LLC_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc NS_CodecPort_CtrlFunctDef.cc RLCMAC_EncDec.cc Native_FunctionDefs.cc TELNETasp_PT.cc IPA_CodecPort_CtrlFunctDef.cc GTPU_EncDec.cc GTPC_EncDec.cc GTP_CodecPort_CtrlFunctDef.cc SCCP_EncDec.cc  SCTPasp_PT.cc RANAP_EncDec.cc "

export CPPFLAGS_TTCN3="-DIPA_EMULATION_GSUP -DIPA_EMULATION_CTRL -DUSE_MTP3_DISTRIBUTOR -DRAN_EMULATION_RANAP -DBSSGP_EM_L3"

../regen-makefile.sh SGSN_Tests.ttcn $FILES

sed -i -e 's/^LINUX_LIBS = -lxml2 -lsctp/LINUX_LIBS = -lxml2 -lsctp -lfftranscode/' Makefile
