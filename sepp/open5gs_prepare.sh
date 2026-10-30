#!/bin/sh -ex

adjust_sysconfdir_config() {
	sed -i "s~@sysconfdir@~$TESTENV_INSTALL_DIR/etc/~g" $1
}

adjust_sysconfdir_config ./*.yaml
