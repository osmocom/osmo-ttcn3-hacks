#!/bin/sh

NAME=Asterisk_Tests

FILES="
	*.c
	*.ttcn
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	SDP_EncDec.cc
	SIPmsg_PT.cc
	TCCConversion.cc
	TCCInterface.cc
	TCCOpenSecurity.cc
	TELNETasp_PT.cc
"

../regen-makefile.sh -e $NAME $FILES
