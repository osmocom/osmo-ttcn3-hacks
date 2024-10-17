#!/bin/sh -ex
CONFIG="$1"
SITE_ID="$2"
TRX_NUM="$3"

case "$CONFIG" in
	generic|sccplite)
		BTS_FEATURES="-fCCN,EGPRS,GPRS,IPv6_NSVC,PAGING_COORDINATION,OSMUX"
		;;
	vamos)
		BTS_FEATURES="-fCCN,EGPRS,GPRS,IPv6_NSVC,PAGING_COORDINATION,VAMOS,OSMUX"
		;;
	*)
		set +x
		echo "ERROR: unknown CONFIG=$1"
		exit 1
		;;
esac

respawn.sh \
	osmo-bts-omldummy \
		"$BTS_FEATURES" \
		"127.0.0.1" \
		"$SITE_ID" \
		"$TRX_NUM"
