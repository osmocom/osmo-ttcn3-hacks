#!/bin/sh -ex
if [ -z "$TESTENV_QEMU_KERNEL" ]; then
	exec osmo-ggsn "$@"
fi

build_initrd() {
	qemu_initrd_init

	qemu_initrd_add_mod \
		gtp \
		tun

	qemu_initrd_add_bin \
		osmo-ggsn

	qemu_initrd_add_file \
		osmo-ggsn.cfg

	# Enable dynamic_debug
	qemu_initrd_add_cmd \
		"mount -t debugfs none /sys/kernel/debug || true" \
		"echo -n 'module gtp +p' > /sys/kernel/debug/dynamic_debug/control || true"

	qemu_initrd_add_cmd \
		"ip addr add 172.18.3.2/24 brd 172.18.3.255 dev eth0" \
		"ip addr add 172.18.3.201/24 brd 172.18.3.255 dev eth0" \
		"ip route add default via 172.18.3.1 dev eth0" \
		"sysctl net.ipv4.ip_forward=1" \
		"osmo-ggsn"

	qemu_initrd_finish
}

. qemu_functions.sh

if ! build_initrd >build_initrd.log 2>&1; then
	qemu_initrd_exit_error "build_initrd.log"
fi

qemu_run
