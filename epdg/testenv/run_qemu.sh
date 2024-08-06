#!/bin/sh -ex
GEN_SOCKET_LIBDIR=""

set_gen_socket_libdir() {
	if [ -z "$OSMO_DEV_MAKE_DIR" ]; then
		# With --binary-repo
		GEN_SOCKET_LIBDIR=/usr/lib
	else
		# Without --binary-repo
		GEN_SOCKET_LIBDIR="$OSMO_DEV_MAKE_DIR"/osmo-epdg/default/lib/gen_socket/priv/lib
	fi
}

adjust_osmo_epdg_config() {
	sed -i "s~@GEN_SOCKET_LIBDIR@~$GEN_SOCKET_LIBDIR~" osmo-epdg.config
}

build_initrd() {
	qemu_initrd_init

	qemu_initrd_add_mod \
		dummy \
		gtp \
		sctp \
		tun

	qemu_initrd_add_bin \
		"$GEN_SOCKET_LIBDIR"/gen_socket.so \
		"$GEN_SOCKET_LIBDIR"/gen_socket_nif.so \
		/usr/bin/escript \
		/usr/lib/x86_64-linux-gnu/libm.so.6 \
		/usr/lib/x86_64-linux-gnu/libsctp.so.1 \
		/usr/lib/x86_64-linux-gnu/libstdc++.so.6 \
		/usr/lib/x86_64-linux-gnu/libtinfo.so.6 \
		/usr/lib/x86_64-linux-gnu/libz.so.1

	qemu_initrd_add_file \
		"$(which osmo-epdg)" \
		/usr/bin/erl \
		/usr/lib/erlang \
		osmo-epdg.config \
		testenv/run_osmo_epdg_with_dummy_ue.sh

	# Enable dynamic_debug
	qemu_initrd_add_cmd \
		"mount -t debugfs none /sys/kernel/debug || true" \
		"echo -n 'module gtp +p' > /sys/kernel/debug/dynamic_debug/control || true"

	qemu_initrd_add_cmd \
		"ip addr add 172.18.3.20/24 brd 172.18.3.255 dev eth0" \
		"ip route add default via 172.18.3.1 dev eth0" \
		"sysctl net.ipv4.ip_forward=1" \
		"sysctl net.ipv4.conf.all.rp_filter=0" \
		"sysctl net.ipv4.conf.default.rp_filter=0" \
		"testenv/run_osmo_epdg_with_dummy_ue.sh"

	qemu_initrd_finish
}


# FIXME: when using the debian kernel, osmo-epdg doesn't create the gtp device
# and then fails with: "terminating ss7_routes with reason shutdownterminating
# ss7_links with reason shutdownip: can't find device 'gtp0'"
if [ "$TESTENV_QEMU_KERNEL" = "debian" ]; then
	set +x
	echo
	echo "ERROR: epdg currently only works with -C, --custom-kernel"
	echo
	exit 1
fi

set_gen_socket_libdir
adjust_osmo_epdg_config
. qemu_functions.sh
qemu_build_initrd_with_log_redirect
qemu_run
