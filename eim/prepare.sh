#!/bin/sh -ex

# Set mp_restop_path
RESTOP="$TESTENV_SRC_DIR"/onomondo-eim/contrib/rest_api_usage_example/restop.py
test -e "$RESTOP"
sed \
	-i \
	"s,^eIM_Tests\.mp_restop_path := .*,eIM_Tests.mp_restop_path := \"$RESTOP\"," \
	eIM_Tests.cfg
