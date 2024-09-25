#!/bin/sh -e
br="testenv0"
qemu_if="$1"

echo "qemu_ifup.sh: $br, $qemu_if"
set -x

ip link set "$qemu_if" up

brctl addif "$br" "$qemu_if"
