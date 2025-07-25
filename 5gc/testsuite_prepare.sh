#!/bin/sh -ex

adjust_ttcn3_config() {
	local as_user="${USER:-root}"
	sed -i "s/^C5G_Tests.mp_run_prog_as_user := .*/C5G_Tests.mp_run_prog_as_user := \"$as_user\"/" \
		C5G_Tests.cfg
}

adjust_ttcn3_config
