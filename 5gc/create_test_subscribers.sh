#!/bin/sh -ex
DB_URI="mongodb://127.0.0.103/open5gs"
DBCTL_CMD="open5gs-dbctl --db_uri=$DB_URI"

# mongod needs some time to bootstrap...
while ! mongosh --quiet $DB_URI</dev/null; do
	sleep 1
done

NUM_SUBCRS=256
get_subscr_imsi() {
	subscr_idx=$1
	printf "999700000000%03u" "$subscr_idx"
}

# Create a test subscriber with IMSI=001010000000000
DBCTL_PARALLEL_JOBS=20
idx=0
while test "$idx" -lt "$NUM_SUBCRS"; do
	remain="$((NUM_SUBCRS - idx))"
	if test "$remain" -gt "$DBCTL_PARALLEL_JOBS"; then
		iterations=$DBCTL_PARALLEL_JOBS
	else
		iterations=$remain
	fi
	for it in $(seq "$iterations"); do
		$DBCTL_CMD add "$(get_subscr_imsi $idx)" 3c6e0b8a9c15224a8228b9a98ca1531d 762a2206fe0b4151ace403c86a11e479 &
		idx=$((idx + 1))
	done
	wait $(jobs -p)
done
