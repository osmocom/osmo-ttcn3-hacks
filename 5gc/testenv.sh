#!/bin/sh -ex

check_usage() {
	if [ -z "$TESTENV_CLEAN_REASON" ]; then
		set +x
		echo "Do not run this script manually."
		echo "Run 'testenv.py run 5gc' instead."
		exit 1
	fi
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
		setcap_open5gs_upfd
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
