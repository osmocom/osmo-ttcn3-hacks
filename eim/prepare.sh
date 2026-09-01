#!/bin/sh -ex

# Set mp_restop_path
RESTOP="$TESTENV_SRC_DIR"/onomondo-eim/contrib/restop.py
test -e "$RESTOP"
sed \
	-i \
	"s,^eIM_Tests\.mp_restop_path := .*,eIM_Tests.mp_restop_path := \"$RESTOP\"," \
	eIM_Tests.cfg
