#!/bin/sh
id="${1:-1234}"
set -x
while sleep 1; do osmo-bts-omldummy -f VAMOS 127.0.0.1 $id; done