#!/bin/sh -ex

set_ogs_sysconf_dir() {
	if [ -z "$OSMO_DEV_MAKE_DIR" ]; then
		# With --binary-repo
		OGS_INSTALL_DIR="/"
		OGS_SYSCONF_DIR="$OGS_INSTALL_DIR/etc/"
	else
		# Without --binary-repo
		OGS_INSTALL_DIR="$OSMO_DEV_MAKE_DIR"/..
		OGS_SYSCONF_DIR="$OGS_INSTALL_DIR/usr/etc/"
	fi
}

adjust_sysconfdir_config() {
	sed -i "s~@sysconfdir@~$1~g" $2
}

set_ogs_sysconf_dir
adjust_sysconfdir_config "$OGS_SYSCONF_DIR" ./*.yaml
