#!/bin/sh -ex
TESTSUITE_CONFIG="$1"

# Set mp_restop_path
RESTOP="$TESTENV_SRC_DIR"/onomondo-eim/contrib/rest_api_usage_example/restop.py
test -e "$RESTOP"
test -e "$TESTSUITE_CONFIG"
sed \
	-i \
	"s,^eIM_Tests\.mp_restop_path := .*,eIM_Tests.mp_restop_path := \"$RESTOP\"," \
	$TESTSUITE_CONFIG

# Reconfigure eIM REST timeout (sys.config, rest_timeout_stuck) to a value
# suitable for this testsuite.
SYS_CONFIG="$TESTENV_INSTALL_DIR"/etc/osmocom/onomondo-eim.config
cat $SYS_CONFIG
sed \
	-i \
	"s/\(^[[:space:]]*{rest_timeout_stuck,[[:space:]]\)[0-9]*/\115/g" \
	$SYS_CONFIG
