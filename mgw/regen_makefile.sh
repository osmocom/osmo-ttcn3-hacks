#!/bin/sh

NAME=MGCP_Test

FILES="
	*.c
	*.ttcn
	*.ttcnpp
	IPA_CodecPort_CtrlFunctDef.cc
	IPL4asp_PT.cc
	IPL4asp_discovery.cc
	IP_EncDec.cc
	IuUP_EncDec.cc
	MGCP_CodecPort_CtrlFunctDef.cc
	Native_FunctionDefs.cc
	OSMUX_CodecPort_CtrlFunctDef.cc
	RTP_CodecPort_CtrlFunctDef.cc
	RTP_EncDec.cc
	SDP_EncDec.cc
	StatsD_CodecPort_CtrlFunctdef.cc
	TCCConversion.cc
	TCCInterface.cc
	TELNETasp_PT.cc
"

export CPPFLAGS_TTCN3="
	-DIPA_EMULATION_CTRL
	-DSTATSD_HAVE_VTY
"

../_buildsystem/regen-makefile.sh -e $NAME $FILES
