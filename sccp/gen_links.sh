#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

DIR=$BASEDIR/titan.TestPorts.UNIX_DOMAIN_SOCKETasp/src
FILES="UD_PT.cc  UD_PT.hh  UD_PortType.ttcn  UD_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
FILES+=" TCCEncoding_Functions.ttcn TCCEncoding.cc " # GSM 7-bit coding
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

# Required by MGCP and IPA
DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

# required by M3UA_Emulation
DIR=$BASEDIR/titan.ProtocolModules.M3UA/src
FILES="M3UA_Types.ttcn"
gen_links $DIR $FILES

# required by M3UA_Emulation
DIR=$BASEDIR/titan.TestPorts.SCTPasp/src
FILES="SCTPasp_PT.cc  SCTPasp_PT.hh  SCTPasp_PortType.ttcn  SCTPasp_Types.ttcn"
gen_links $DIR $FILES

# required by M3UA Emulation
DIR=$BASEDIR/titan.TestPorts.MTP3asp/src
FILES="MTP3asp_PortType.ttcn  MTP3asp_Types.ttcn"
gen_links $DIR $FILES

# required by SCCP Emulation
DIR=$BASEDIR/titan.ProtocolEmulations.M3UA/src
FILES="M3UA_Emulation.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolEmulations.SCCP/src
FILES="SCCP_Emulation.ttcn  SCCP_EncDec.cc  SCCP_Mapping.ttcnpp  SCCP_Types.ttcn  SCCPasp_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.TELNETasp/src
FILES="TELNETasp_PT.cc  TELNETasp_PT.hh  TELNETasp_PortType.ttcn"
gen_links $DIR $FILES


DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn "
FILES+="Osmocom_VTY_Functions.ttcn Native_Functions.ttcn Native_FunctionDefs.cc "
FILES+="IPA_Types.ttcn IPA_Emulation.ttcnpp IPA_CodecPort.ttcn IPA_CodecPort_CtrlFunct.ttcn IPA_CodecPort_CtrlFunctDef.cc "
FILES+="Osmocom_CTRL_Types.ttcn Osmocom_CTRL_Functions.ttcn Osmocom_CTRL_Adapter.ttcn "
FILES+="SCCP_Templates.ttcn "
gen_links $DIR $FILES

gen_links_finish
