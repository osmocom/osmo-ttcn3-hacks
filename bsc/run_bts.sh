#!/bin/sh -ex
BTS_FEATURES="-fCCN,EGPRS,GPRS,IPv6_NSVC,PAGING_COORDINATION,OSMUX"
BTS="$1"
TRXN="$2"
EXTRA_BTS_FEATURES="$3"

test -n "$BTS"
test -n "$TRXN"

if [ -n "$EXTRA_BTS_FEATURES" ]; then
	BTS_FEATURES="$BTS_FEATURES,$EXTRA_BTS_FEATURES"
fi

respawn.sh \
	osmo-bts-omldummy \
		$BTS_FEATURES \
		127.0.0.20 \
		$(($BTS + 1234)) \
		$TRXN
