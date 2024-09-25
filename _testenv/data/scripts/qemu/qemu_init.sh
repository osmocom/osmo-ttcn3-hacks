#!/bin/busybox sh
echo "Running initrd-init.sh"
set -ex

export HOME=/root
export LD_LIBRARY_PATH=/usr/local/lib
export PATH=/usr/local/bin:/usr/bin:/bin:/sbin:/usr/local/sbin:/usr/sbin
export TERM=screen

/bin/busybox --install -s

mknod /dev/null c 1 3

# Required for osmo-ggsn
mknod /dev/net/tun c 10 200

hostname qemu

mount -t proc proc /proc
mount -t sysfs sys /sys

# Load modules from qemu_initrd_add_mod()
if [ -e /modules ]; then
	cat /modules | xargs -t -n1 modprobe
fi

ip link set lo up
ip link set eth0 up

echo "KERNEL_TEST_VM_IS_READY"

# Use '|| true' to avoid "attempting to kill init" kernel panic on failure
/cmd.sh || true

# Avoid kernel panic when init exits
poweroff -f
