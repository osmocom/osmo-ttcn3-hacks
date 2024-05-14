#!/bin/sh

# use: ipset_setup.sh \
# $LOC_IP $LOC_PORT_C $LOC_SPI_C $LOC_PORT_S $LOC_SPI_S \
# $REM_IP $REM_PORT_C $REM_SPI_C $REM_PORT_S $REM_SPI_S \
# $AUTH_KEY

LOC_IP="${1}"
LOC_PORT_C="${2}"
LOC_SPI_C="${3}"
LOC_PORT_S="${4}"
LOC_SPI_S="${5}"
REM_IP="${6}"
REM_PORT_C="${7}"
REM_SPI_C="${8}"
REM_PORT_S="${9}"
REM_SPI_S="${10}"
AUTH_KEY="${11}"

set -x

# Clean up state from previous tests:
ip xfrm policy flush
ip xfrm state flush

# use: ip_xfrm <src_ip> <src_port> <dst_ip> <dst_port> <spi>
ip_xfrm_state() {
        ip xfrm state add \
                src "${1}" dst "${3}" proto esp spi "${5}" reqid "${5}" mode transport \
                replay-window 32 \
                auth-trunc sha1 "${AUTH_KEY}" 96 \
                enc cipher_null "" \
                sel src "${1}/32" dst "${3}/32" sport "${2}" dport "${4}"
}

# TTCN3(Srv) -> Asterisk(Cli): REM_SPI_C
ip_xfrm_state "${LOC_IP}" "${LOC_PORT_S}" "${REM_IP}" "${REM_PORT_C}" "${REM_SPI_C}"

# TTCN3(Cli) -> Asterisk(Srv): REM_SPI_S
ip_xfrm_state "${LOC_IP}" "${LOC_PORT_C}" "${REM_IP}" "${REM_PORT_S}" "${REM_SPI_S}"

# Asterisk(Cli) -> TTCN3(Srv): LOC_SPI_S
ip_xfrm_state "${REM_IP}" "${REM_PORT_C}" "${LOC_IP}" "${LOC_PORT_S}" "${LOC_SPI_S}"

# Asterisk(Srv) -> TTCN3(Cli): LOC_SPI_C
ip_xfrm_state "${REM_IP}" "${REM_PORT_S}" "${LOC_IP}" "${LOC_PORT_C}" "${LOC_SPI_C}"

# use: ip_xfrm <src_ip> <src_port> <dst_ip> <dst_port> <req_id> <dir>
ip_xfrm_policy() {
        ip xfrm policy add \
                src "${1}/32" dst "${3}/32" sport "${2}" dport "${4}" \
                dir "${6}" \
                tmpl src "${1}" dst "${3}" \
                proto esp reqid "${5}" mode transport
}

# TTCN3(Srv) -> Asterisk(Cli): REM_SPI_C out
ip_xfrm_policy "${LOC_IP}" "${LOC_PORT_S}" "${REM_IP}" "${REM_PORT_C}" "${REM_SPI_C}" "out"

# TTCN3(Cli) -> Asterisk(Srv): REM_SPI_S out
ip_xfrm_policy "${LOC_IP}" "${LOC_PORT_C}" "${REM_IP}" "${REM_PORT_S}" "${REM_SPI_S}" "out"

# Asterisk(Cli) -> TTCN3(Srv): LOC_SPI_S in
ip_xfrm_policy "${REM_IP}" "${REM_PORT_C}" "${LOC_IP}" "${LOC_PORT_S}" "${LOC_SPI_S}" "in"

# Asterisk(Srv) -> TTCN3(Cli): LOC_SPI_C in
ip_xfrm_policy "${REM_IP}" "${REM_PORT_S}" "${LOC_IP}" "${LOC_PORT_C}" "${LOC_SPI_C}" "in"

#ip xfrm state
#ip xfrm policy
