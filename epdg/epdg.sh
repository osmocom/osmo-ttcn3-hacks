#!/bin/bash
set +e
set -x

EPDG_TUN="gtp0"
UE_IFACE="ue"
UE_SUBNET="192.168.0.0/16"
UE_ADDR="192.168.0.2/16"

if [ "$IMAGE_SUFFIX" = "latest" ]; then
	EPDG_CFG="osmo-epdg.latest.config"
else
	EPDG_CFG="osmo-epdg.config"
fi

ip link add $UE_IFACE type dummy
ip addr add $UE_ADDR dev $UE_IFACE
ip link set $UE_IFACE up
ip rule add from $UE_SUBNET table 45

sed -i "s#/tmp/osmo-epdg/_build/default/lib/gen_socket/priv/lib#$OSMO_DEV_MAKE_DIR/osmo-epdg/default/lib/gen_socket/priv/lib#g" "$EPDG_CFG"

ERL_FLAGS="-config $EPDG_CFG" osmo-epdg &
MYPID=$!

# We cannot set a route for the interface until it is created by osmo-epdg...
echo "Waiting for interface ${EPDG_TUN}..."
pipework --wait -i ${EPDG_TUN}
echo "Adding src ${UE_SUBNET} default route to ${EPDG_TUN}"
ip route add default dev $EPDG_TUN table 45

wait $MYPID
