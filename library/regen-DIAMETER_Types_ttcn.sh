#!/bin/sh -e

SELF_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)" # dirname of this script (library/)
AVP_SH_DIR="$SELF_DIR/../deps/titan.ProtocolModules.DIAMETER_ProtocolModule_Generator/src"
AVP_SH_PATH="$AVP_SH_DIR/AVP.sh"
cd $AVP_SH_DIR
$AVP_SH_PATH \
        Base_IETF_RFC3588.ddf \
        BaseTypes_IETF_RFC3588.ddf \
        MobileIPv4_Application_IETF_RFC4004.ddf \
        NetworkAccessServer_IETF_RFC4005.ddf \
        CreditControl_IETF_RFC4006.ddf \
        MobileIPv6_NAS_IETF_RFC5447.ddf \
        MobileIPv6_HA_IETF_RFC5778.ddf \
        AAAInterface_3GPP_TS29272_f10.ddf \
        GxInterface_PCC_3GPP_TS29212_f10.ddf \
        S6Interfaces_3GPP_TS29336_f00.ddf \
        RxInterface_PCC_3GPP_TS29214_f20.ddf \
        CxDxInterface_3GPP_TS29229_c30.ddf \
        GiSGiInterface_3GPP_TS29061_d70.ddf \
        ChargingApplications_3GPP_TS32299_d90.ddf \
        AAAInterface_3GPP_TS29273_f00.ddf

mv -v $AVP_SH_DIR/DIAMETER_Types.ttcn $SELF_DIR/
