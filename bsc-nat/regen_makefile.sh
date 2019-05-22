#!/bin/sh

MAIN=BSCNAT_Tests.ttcn

FILES="*.ttcn *.ttcnpp SCCP_EncDec.cc IPA_CodecPort_CtrlFunctDef.cc IPL4asp_PT.cc IPL4asp_discovery.cc TCCConversion.cc TCCInterface.cc RTP_EncDec.cc SDP_EncDec.cc *.c MGCP_CodecPort_CtrlFunctDef.cc TELNETasp_PT.cc"

export CPPFLAGS_TTCN3="-DIPA_EMULATION_SCCP -DIPA_EMULATION_MGCP -DRAN_EMULATION_BSSAP -DRAN_EMULATION_MGCP -DUSE_MTP3_DISTRIBUTOR"

../regen-makefile.sh $MAIN $FILES
