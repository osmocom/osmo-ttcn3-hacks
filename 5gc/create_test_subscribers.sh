#!/bin/sh -ex
DB_URI="mongodb://127.0.0.103/open5gs"
DBCTL="$TESTENV_CACHE_DIR/open5gs-dbctl"
DBCTL_CMD="$DBCTL --db_uri=$DB_URI"

if ! [ -e "$DBCTL" ]; then
	wget "https://raw.githubusercontent.com/open5gs/open5gs/v2.7.1/misc/db/open5gs-dbctl" \
		-O "$DBCTL"
fi

if ! [ -x "$DBCTL" ]; then
	chmod +x "$DBCTL"
fi

# mongod needs some time to bootstrap...
while ! mongosh --quiet $DB_URI</dev/null; do
	sleep 1
done

# Create a test subscriber with IMSI=001010000000000
$DBCTL_CMD add 999700000000000 3c6e0b8a9c15224a8228b9a98ca1531d 762a2206fe0b4151ace403c86a11e479

# Mark test subscriber with IMSI=001010000000001 as:
# Subscriber-Status=OPERATOR_DETERMINED_BARRING (1)
# Operator-Determined-Barring="Barring of all outgoing inter-zonal calls except those directed to the home PLMN country" (7)
$DBCTL_CMD add 999700000000001 3c6e0b8a9c15224a8228b9a98ca1531d 762a2206fe0b4151ace403c86a11e479
$DBCTL_CMD subscriber_status 999700000000001 1 7
