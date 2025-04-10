#!/bin/sh -e

NAME=Asterisk_Tests

FILES="
	*.c
	*.ttcn
	PIPEasp_PT.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	Native_FunctionDefs.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	SDP_EncDec.cc
	SIPmsg_PT.cc
	TCCConversion.cc
	TCCEncoding.cc
	TCCInterface.cc
	TCCOpenSecurity.cc
	TCCDateTime.cc
	TELNETasp_PT.cc
"

. ../_buildsystem/regen_makefile.inc.sh

# required for forkpty(3) used by PIPEasp
sed -i -e '/^LINUX_LIBS/ s/$/ -lutil/' Makefile
