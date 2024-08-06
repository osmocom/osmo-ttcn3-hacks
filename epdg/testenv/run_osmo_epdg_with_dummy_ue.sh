#!/bin/sh -ex
EPDG_TUN="gtp0"
UE_IFACE="ue"
UE_SUBNET="192.168.0.0/16"
UE_ADDR="192.168.0.2/16"

ip link add "$UE_IFACE" type dummy
ip addr add "$UE_ADDR" dev "$UE_IFACE"
ip link set "$UE_IFACE" up
ip rule add from "$UE_SUBNET" table 45

ERL_FLAGS='-config osmo-epdg.config' osmo-epdg &

set +x

FOUND=0
for i in $(seq 1 10); do
	sleep 1
	if ip link ls dev "$EPDG_TUN" 2>&1 >/dev/null; then
		FOUND=1
		break
	fi
done

if [ "$FOUND" = 0 ]; then
	echo
	echo "ERROR: run_osmo_epdg_with_dummy_ue.sh: osmo-epdg did not create $EPDG_TUN!"
	echo
	exit 1
fi

ip route add default dev $EPDG_TUN table 45

echo
# wait_until_osmo_epdg_is_ready.sh checks for this string
echo "run_osmo_epdg_with_dummy_ue.sh: osmo-epdg is ready"
echo

wait
