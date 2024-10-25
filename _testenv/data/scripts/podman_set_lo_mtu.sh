#!/bin/sh -e
# Change the MTU for the loopback device, but only when running in podman (as
# we don't want to just make this change for the host without asking). This
# is used as workaround for OS#6602 where we have some tests that don't pass if
# the MTU=65536, the default of lo.

MTU="$1"

if [ "$PODMAN" = 1 ]; then
	echo "Setting MTU for lo to $MTU (OS#6602)"
	sudo ip link set dev lo mtu "$1"
else
	if [ "$(cat /sys/class/net/lo/mtu)" = 65536 ]; then
		echo
		echo "========================================================================"
		echo "WARNING: your loopback network device (lo) has the default MTU of 65536."
		echo "This is known to cause a some tests to fail, see OS#6602."
		echo
		echo "Workaround: 'ip link set dev lo mtu $1' or --podman"
		echo "========================================================================"
		echo
	fi
fi
