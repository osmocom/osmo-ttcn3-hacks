#!/bin/sh -ex
DEV=ggsn_dummy

check_usage() {
	if [ -z "$TESTENV_CLEAN_REASON" ]; then
		set +x
		echo "Do not run this script manually."
		echo "Run 'testenv.py run ggsn' instead."
		exit 1
	fi
}

adjust_ttcn3_config() {
	sed -i 's/^GGSN_Tests.m_ggsn_impl := .*/GGSN_Tests.m_ggsn_impl := GGSN_IMPL_OPEN5GS/' \
		../testsuite/GGSN_Tests.cfg
	sed -i 's/^GGSN_Tests.m_ggsn_ip_gtpu := .*/GGSN_Tests.m_ggsn_ip_gtpu := "127.0.0.222"/' \
		../testsuite/GGSN_Tests.cfg
}

setcap_open5gs_upfd() {
	sudo setcap CAP_NET_RAW=+eip $(which open5gs-upfd)
}

add_tun() {
	local name="$1"
	if ! grep "$name" /proc/net/dev > /dev/null; then
		sudo ip tuntap add name $name mode tun
	fi
}

add_addr() {
	local name="$1"
	local addr="$2"

	sudo ip addr add "$addr" dev "$name"
}

add_tun_all() {
	add_tun "ogstun4"
	add_tun "ogstun6"
	add_tun "ogstun46"

	add_addr "ogstun4" "176.16.16.1/20"
	add_addr "ogstun6" "2001:780:44:2000:0:0:0:1/56"
	add_addr "ogstun46" "176.16.32.1/20"
	add_addr "ogstun46" "2001:780:44:2100:0:0:0:1/56"

	sudo ip link set ogstun4 up
	sudo ip link set ogstun6 up
	sudo ip link set ogstun46 up
}

del_tun() {
	local name="$1"

	if ip link ls dev "$name" >/dev/null 2>&1; then
		sudo ip link set "$name" down
		sudo ip link del "$name"
	fi
}

del_tun_all() {
	del_tun "ogstun4"
	del_tun "ogstun6"
	del_tun "ogstun46"
}

add_dummy_netdev() {
	# Add a network device reachable through the GTP tunnel that can answer ICMP
	# pings (for e.g. TC_pdp4_act_deact_gtpu_access)
	sudo ip link add "$DEV" type dummy
	sudo ip addr add "172.18.3.201" dev "$DEV"
	sudo ip addr add "fd02:db8:3::201" dev "$DEV"
	sudo ip link set "$DEV" up
}

del_dummy_netdev() {
	if ip link ls dev "$DEV" >/dev/null 2>&1; then
		sudo ip link del "$DEV"
	fi
}

check_usage

case "$TESTENV_CLEAN_REASON" in
	prepare)
		setcap_open5gs_upfd
		adjust_ttcn3_config
		del_dummy_netdev
		del_tun_all
		add_dummy_netdev
		add_tun_all
		;;
	crashed|finished)
		del_dummy_netdev
		del_tun_all
		;;
	*)
		set +x
		echo "ERROR: unexpected TESTENV_CLEAN_REASON: $TESTENV_CLEAN_REASON"
		exit 1
		;;
esac