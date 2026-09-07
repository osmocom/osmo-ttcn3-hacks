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
