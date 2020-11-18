#!/bin/bash

FILES="*.ttcn *.ttcnpp BSSGP_EncDec.cc LLC_EncDec.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc NS_CodecPort_CtrlFunctDef.cc RLCMAC_EncDec.cc Native_FunctionDefs.cc TELNETasp_PT.cc IPA_CodecPort_CtrlFunctDef.cc GTPU_EncDec.cc GTPC_EncDec.cc GTP_CodecPort_CtrlFunctDef.cc SCCP_EncDec.cc  SCTPasp_PT.cc "
FILES+="AF_PACKET_PT.cc AF_PACKET_PT.hh "

export CPPFLAGS_TTCN3="-DIPA_EMULATION_GSUP -DIPA_EMULATION_CTRL -DUSE_MTP3_DISTRIBUTOR -DBSSGP_EM_L3 -DNS_EMULATION_FR"

../regen-makefile.sh GBProxy_Tests.ttcn $FILES

sed -i -i 's/^LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lsctp/' Makefile
