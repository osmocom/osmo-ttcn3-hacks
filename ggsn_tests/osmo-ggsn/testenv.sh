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

adjust_osmo_ggsn_config() {
	osmo-config-merge \
		osmo-ggsn/osmo-ggsn.src.cfg \
		osmo-ggsn/osmo-ggsn-"$CONFIG".confmerge \
		> osmo-ggsn.cfg
}

adjust_ttcn3_config() {
	local config_upper="$(echo "$CONFIG" | tr "[:lower:]" "[:upper:]")"

	sed -i "s/^GGSN_Tests.m_ggsn_conf := .*/GGSN_Tests.m_ggsn_conf := GGSN_CONF_$config_upper/" \
		../testsuite/GGSN_Tests.cfg
}

setcap_osmo_ggsn() {
	sudo setcap CAP_NET_ADMIN=+eip $(which osmo-ggsn)
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
EXTRA_IPS="172.18.3.201 fd02:db8:3::201" add_remove_testenv0_bridge.sh

case "$TESTENV_CLEAN_REASON" in
	prepare)
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
