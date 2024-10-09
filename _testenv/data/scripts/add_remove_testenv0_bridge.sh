#!/bin/sh -e
# Call this as or from within a clean= script to add and remove a testenv0
# bridge device, which can be used to talk to a SUT running in QEMU.
# Set env var EXTRA_IPS to add additional IP addresses to the device.
DEV="testenv0"
IP4="172.18.3.1/24"
IP6="fd02:db8:3::1/128"
set -x

add_bridge() {
	local ip

	sudo brctl addbr "$DEV"
	sudo ip link set "$DEV" up

	for ip in $IP4 $IP6 $EXTRA_IPS; do
		sudo ip addr add "$ip" dev "$DEV"
	done
}

del_bridge() {
	if ip link ls dev "$DEV" >/dev/null 2>&1; then
		sudo ip link set "$DEV" down
		sudo brctl delbr "$DEV"
	fi
}

case "$TESTENV_CLEAN_REASON" in
	prepare)
		del_bridge
		add_bridge
		;;
	crashed|finished)
		del_bridge
		;;
	*)
		set +x
		echo "ERROR: unexpected TESTENV_CLEAN_REASON: $TESTENV_CLEAN_REASON"
		exit 1
		;;
esac
