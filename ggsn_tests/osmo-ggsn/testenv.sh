#!/bin/sh -ex
CONFIG="$1"  # all, v4_only, etc.

check_usage() {
	local valid="all|v4_only|v6_only|v4v6_only"

	if [ -z "$CONFIG" ]; then
		set +x
		echo "Do not run this script manually."
		echo "Run 'testenv.py run ggsn' instead."
		exit 1
	fi

	case "|$valid|" in
		*"|$CONFIG|"*)
			;;
		*)
			set +x
			echo "usage: testenv.sh $valid"
			exit 1
			;;
	esac
}

check_usage_qemu() {
	if [ -n "$TESTENV_QEMU_KERNEL" ] && [ "$CONFIG" = "all" ]; then
		set +x
		echo "ERROR: '--config osmo_ggsn_all' uses multiple APNs, which is currently not supported with kernel" \
			"gtp-u! (OS#6106)"
		exit 1
	fi
}

replace_ips() {
	# Run osmo-ggsn on 172.18.3.x (testenv0 bridge) instead of
	# 127.0.0.1 (lo), so it works when running osmo-ggsn in QEMU to test
	# kernel GTP-U too. We keep 127.0.0.x in the configs, so they can be
	# used without testenv too.
	sed -i 's/127\.0\.0\./172.18.3./g' "$1"
}

adjust_osmo_ggsn_config() {
	osmo-config-merge \
		osmo-ggsn/osmo-ggsn.src.cfg \
		osmo-ggsn/osmo-ggsn-"$CONFIG".confmerge \
		> osmo-ggsn.cfg

	replace_ips osmo-ggsn.cfg

	if [ -n "$TESTENV_QEMU_KERNEL" ]; then
		sed -i "s/gtpu-mode tun/gtpu-mode kernel-gtp/" osmo-ggsn.cfg
	fi
}

adjust_ttcn3_config() {
	local config_upper="$(echo "$CONFIG" | tr "[:lower:]" "[:upper:]")"

	sed -i "s/^GGSN_Tests.m_ggsn_conf := .*/GGSN_Tests.m_ggsn_conf := GGSN_CONF_$config_upper/" \
		../testsuite/GGSN_Tests.cfg

	replace_ips ../testsuite/GGSN_Tests.cfg
}

setcap_osmo_ggsn() {
	if [ -z "$TESTENV_QEMU_KERNEL" ]; then
		sudo setcap CAP_NET_ADMIN=+eip $(which osmo-ggsn)
	fi
}

rename_junit_xml_classname() {
	if [ "$CONFIG" != "all" ]; then
		( cd ../testsuite
		  rename_junit_xml_classname.sh "_$CONFIG" )
	fi
}

check_usage

# Add a bridge reachable through the GTP tunnel that can answer ICMP
# pings (for e.g. TC_pdp4_act_deact_gtpu_access). The bridge is also used to
# connect the SUT when it runs in QEMU.
if [ -n "$TESTENV_QEMU_KERNEL" ]; then
	add_remove_testenv0_bridge.sh
else
	EXTRA_IPS="172.18.3.201 fd02:db8:3::201 172.18.3.2" add_remove_testenv0_bridge.sh
fi

case "$TESTENV_CLEAN_REASON" in
	prepare)
		check_usage_qemu
		adjust_osmo_ggsn_config
		adjust_ttcn3_config
		setcap_osmo_ggsn
		;;
	crashed|finished)
		rename_junit_xml_classname
		;;
	*)
		set +x
		echo "ERROR: unexpected TESTENV_CLEAN_REASON: $TESTENV_CLEAN_REASON"
		exit 1
		;;
esac
