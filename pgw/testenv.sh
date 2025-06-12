#!/bin/sh -ex

check_usage() {
	if [ -z "$TESTENV_CLEAN_REASON" ]; then
		set +x
		echo "Do not run this script manually."
		echo "Run 'testenv.py run pgw' instead."
		exit 1
	fi
}

adjust_ttcn3_config() {
	local as_user="${USER:-root}"
	sed -i "s/^PGW_Tests.mp_run_prog_as_user := .*/PGW_Tests.mp_run_prog_as_user := \"$as_user\"/" \
		PGW_Tests.cfg
}

run_setcap() {
	sudo setcap "CAP_NET_ADMIN=+eip" "$(which open5gs-upfd)"
	sudo setcap "CAP_NET_ADMIN=+eip" "$(which open5gs-smfd)"
	sudo setcap "CAP_NET_ADMIN,CAP_SYS_ADMIN=+eip" "$(which osmo-uecups-daemon)"
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

	add_addr "ogstun46" "10.45.0.1/16"
	add_addr "ogstun46" "cafe::1/64"

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

check_usage

case "$TESTENV_CLEAN_REASON" in
	prepare)
		run_setcap
		adjust_ttcn3_config
		del_tun_all
		add_tun_all
		;;
	crashed|finished)
		del_tun_all
		;;
	*)
		set +x
		echo "ERROR: unexpected TESTENV_CLEAN_REASON: $TESTENV_CLEAN_REASON"
		exit 1
		;;
esac
