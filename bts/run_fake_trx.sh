#!/bin/bash

# Most BTS_Tests require to have fake_trx running.
# fake trx is part of osmo-trx
FAKE_TRX_DIR="${FAKE_TRX_DIR:-../../osmo-trx/osmocom-bb/src/target/trx_toolkit}"

cd "$FAKE_TRX_DIR"
exec ./fake_trx.py  --trx TRX1@127.0.0.1:5700/1 --trx TRX2@127.0.0.1:5700/2 --trx TRX3@127.0.0.1:5700/3
