#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

DIR=$BASEDIR/titan.Libraries.TCCUsefulFunctions/src
FILES="TCCInterface_Functions.ttcn TCCConversion_Functions.ttcn TCCConversion.cc TCCInterface.cc TCCInterface_ip.h"
FILES+=" TCCEncoding_Functions.ttcn TCCEncoding.cc " # BCD coding
FILES+="TCCDateTime.cc TCCDateTime_Functions.ttcn " # NTP_Functions (NTP timestamp)
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.Common_Components.Socket-API/src
FILES="Socket_API_Definitions.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.TestPorts.IPL4asp/src
FILES="IPL4asp_Functions.ttcn  IPL4asp_PT.cc  IPL4asp_PT.hh IPL4asp_PortType.ttcn  IPL4asp_Types.ttcn  IPL4asp_discovery.cc IPL4asp_protocol_L234.hh"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.ICMP/src
FILES="ICMP_EncDec.cc  ICMP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.ICMPv6/src
FILES="ICMPv6_EncDec.cc  ICMPv6_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.IP/src
FILES="IP_EncDec.cc  IP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.UDP/src
FILES="UDP_EncDec.cc  UDP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.NS_v7.3.0/src
FILES="NS_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.BSSGP_v13.0.0/src
FILES="BSSGP_EncDec.cc  BSSGP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTP_v13.5.0/src
FILES="GTPC_EncDec.cc  GTPC_Types.ttcn  GTPU_EncDec.cc  GTPU_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.GTPv2_v13.7.0/src
FILES="GTPv2_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.PFCP_v15.1.0/src
FILES="PFCP_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.DIAMETER_ProtocolModule_Generator/src
FILES="DIAMETER_EncDec.cc"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.HTTP2/src
FILES="HTTP2_EncDec.cc HTTP2_Types.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/titan.ProtocolModules.JSON_v07_2006/src
FILES="JSON_Generic_Null_Def.asn JSON_Generic.ttcn"
gen_links $DIR $FILES

DIR=$BASEDIR/osmo-ttcn3-openapi-generator/openapi-specs/3GPP_5GC_Rel19/ttcn3
FILES="TS26510_CommonData.ttcn TS26512_EventExposure.ttcn TS26512_M5_DynamicPolicies.ttcn TS26512_M5_NetworkAssistance.ttcn TS26512_R4_DataReporting.ttcn TS26532_CommonData.ttcn TS26532_Ndcaf_DataReportingProvisioning.ttcn TS28104_MdaNrm.ttcn TS28104_MdaReport.ttcn TS28105_AiMlNrm.ttcn TS28111_FaultNrm.ttcn TS28310_EnergyInformationNrm.ttcn TS28312_IntentNrm.ttcn TS28317_RanScNrm.ttcn TS28318_DsoNrm.ttcn TS28319_MsacNrm.ttcn TS28532_FileDataReportingMnS.ttcn TS28532_HeartbeatNtf.ttcn TS28532_PerfMnS.ttcn TS28532_ProvMnS.ttcn TS28536_CoslaNrm.ttcn TS28538_EdgeNrm.ttcn TS28541_5GcNrm.ttcn TS28541_NrNrm.ttcn TS28541_SliceNrm.ttcn TS28561_NdtNrm.ttcn TS28567_CclNrm.ttcn TS28623_ComDefs.ttcn TS28623_ExternalDataMgmtNrm.ttcn TS28623_FeatureNrm.ttcn TS28623_FileManagementNrm.ttcn TS28623_GenericNrm.ttcn TS28623_ManagementDataCollectionNrm.ttcn TS28623_MnSRegistryNrm.ttcn TS28623_PmControlNrm.ttcn TS28623_QoEMeasurementCollectionNrm.ttcn TS28623_SubscriptionControlNrm.ttcn TS28623_ThresholdMonitorNrm.ttcn TS28623_TraceControlNrm.ttcn TS29122_AsSessionWithQoS.ttcn TS29122_CommonData.ttcn TS29122_CpProvisioning.ttcn TS29122_MonitoringEvent.ttcn TS29122_PfdManagement.ttcn TS29122_ResourceManagementOfBdt.ttcn TS29502_Nsmf_PDUSession.ttcn TS29503_Nudm_EE.ttcn TS29503_Nudm_NIDDAU.ttcn TS29503_Nudm_PP.ttcn TS29503_Nudm_SDM.ttcn TS29503_Nudm_UECM.ttcn TS29505_Subscription_Data.ttcn TS29507_Npcf_AMPolicyControl.ttcn TS29508_Nsmf_EventExposure.ttcn TS29509_Nausf_SoRProtection.ttcn TS29509_Nausf_UPUProtection.ttcn TS29510_Nnrf_AccessToken.ttcn TS29510_Nnrf_NFDiscovery.ttcn TS29510_Nnrf_NFManagement.ttcn TS29512_Npcf_SMPolicyControl.ttcn TS29514_Npcf_PolicyAuthorization.ttcn TS29515_Ngmlc_Location.ttcn TS29517_Naf_EventExposure.ttcn TS29518_Namf_Communication.ttcn TS29518_Namf_EventExposure.ttcn TS29518_Namf_Location.ttcn TS29519_Application_Data.ttcn TS29519_Policy_Data.ttcn TS29520_Nnwdaf_AnalyticsInfo.ttcn TS29520_Nnwdaf_DataManagement.ttcn TS29520_Nnwdaf_EventsSubscription.ttcn TS29520_Nnwdaf_MLModelMonitor.ttcn TS29520_Nnwdaf_MLModelProvision.ttcn TS29522_5GLANParameterProvision.ttcn TS29522_AMInfluence.ttcn TS29522_AMPolicyAuthorization.ttcn TS29522_GroupParametersProvisioning.ttcn TS29522_IPTVConfiguration.ttcn TS29522_ServiceParameter.ttcn TS29522_TrafficInfluence.ttcn TS29523_Npcf_EventExposure.ttcn TS29525_Npcf_UEPolicyControl.ttcn TS29531_Nnssf_NSSAIAvailability.ttcn TS29531_Nnssf_NSSelection.ttcn TS29534_Npcf_AMPolicyAuthorization.ttcn TS29536_Nnsacf_SliceEventExposure.ttcn TS29543_Npcf_PDTQPolicyControl.ttcn TS29544_Nspaf_SecuredPacket.ttcn TS29551_Nnef_PFDmanagement.ttcn TS29554_Npcf_BDTPolicyControl.ttcn TS29555_N5g_ddnmf_Discovery.ttcn TS29564_Nupf_EventExposure.ttcn TS29565_Ntsctsf_QoSandTSCAssistance.ttcn TS29571_CommonData.ttcn TS29572_Nlmf_DataExposure.ttcn TS29572_Nlmf_Location.ttcn TS29573_N32_Handshake.ttcn TS29574_Ndccf_DataManagement.ttcn TS29575_Nadrf_DataManagement.ttcn TS29576_Nmfaf_3caDataManagement.ttcn TS29591_Nnef_EventExposure.ttcn TS29594_Nchf_SpendingLimitControl.ttcn TS32291_Nchf_ConvergedCharging.ttcn"
gen_links $DIR $FILES

DIR=../library
FILES="Misc_Helpers.ttcn General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn Native_Functions.ttcn Native_FunctionDefs.cc IPCP_Types.ttcn IPCP_Templates.ttcn PAP_Types.ttcn "
FILES+="GTPv1C_CodecPort.ttcn GTPv1C_CodecPort_CtrlFunct.ttcn GTPv1C_CodecPort_CtrlFunctDef.cc GTPv1C_Templates.ttcn Osmocom_Gb_Types.ttcn "
FILES+="GTPv1U_CodecPort.ttcn GTPv1U_CodecPort_CtrlFunct.ttcn GTPv1U_CodecPort_CtrlFunctDef.cc GTPv1U_Emulation.ttcnpp "
FILES+="GTPv2_PrivateExtensions.ttcn GTPv2_Templates.ttcn "
FILES+="GTPv2_CodecPort.ttcn GTPv2_CodecPort_CtrlFunctDef.cc GTPv2_CodecPort_CtrlFunct.ttcn GTPv2_Emulation.ttcn "
FILES+="DNS_Helpers.ttcn "
FILES+="DIAMETER_Types.ttcn DIAMETER_CodecPort.ttcn DIAMETER_CodecPort_CtrlFunct.ttcn DIAMETER_CodecPort_CtrlFunctDef.cc DIAMETER_Emulation.ttcn "
FILES+="DIAMETER_Templates.ttcn DIAMETER_rfc4004_Templates.ttcn DIAMETER_rfc5447_Templates.ttcn DIAMETER_ts29_212_Templates.ttcn DIAMETER_ts29_212_Templates.ttcn DIAMETER_ts29_229_Templates.ttcn DIAMETER_ts29_272_Templates.ttcn DIAMETER_ts29_273_Templates.ttcn DIAMETER_ts32_299_Templates.ttcn "
FILES+="SCTP_Templates.ttcn "
FILES+="NTP_Functions.ttcn PFCP_Templates.ttcn PFCP_CodecPort.ttcn PFCP_CodecPort_CtrlFunct.ttcn PFCP_CodecPort_CtrlFunctDef.cc PFCP_Emulation.ttcn "
FILES+="Mutex.ttcn "
FILES+="HTTP2_CodecPort.ttcn HTTP2_CodecPort_CtrlFunct.ttcn HTTP2_CodecPort_CtrlFunctDef.cc HTTP2_Templates.ttcn HTTP2_Adapter.ttcn HTTP2_Server_Emulation.ttcn "
FILES+="TS29502_Nsmf_PDUSession_Templates.ttcn "
gen_links $DIR $FILES

gen_links_finish
