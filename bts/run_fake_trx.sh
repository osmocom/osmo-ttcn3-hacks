#!/bin/bash
# Most BTS_Tests require to have fake_trx running.
# fake trx is part of osmo-trx

if [ "$TESTENV_BINARY_REPO" = 1 ]; then
	# testenv with --binary-repo -> osmocom-bb-trx-toolkit package
	FAKE_TRX_DIR="/usr/share/osmocom-bb/trx_toolkit"
elif [ -n "$TESTENV_SRC_DIR" ]; then
	# testenv without --binary-repo -> osmocom-bb cloned via osmo-dev
	FAKE_TRX_DIR="$TESTENV_SRC_DIR"/osmocom-bb/src/target/trx_toolkit
fi

FAKE_TRX_DIR="${FAKE_TRX_DIR:-../../osmo-trx/osmocom-bb/src/target/trx_toolkit}"

cd "$FAKE_TRX_DIR"
exec ./fake_trx.py  --trx TRX1@127.0.0.1:5700/1 --trx TRX2@127.0.0.1:5700/2 --trx TRX3@127.0.0.1:5700/3
