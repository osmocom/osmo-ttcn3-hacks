#!/bin/sh
# Restart a given test component while the testsuite is running.

trap "kill 0" EXIT

SLEEP_BEFORE_RESPAWN=${SLEEP_BEFORE_RESPAWN:-0}

i=0
max_i=500
while [ $i -lt $max_i ]; do
	echo "respawn: $i: starting: $*"
	$* &
	LAST_PID=$!
	wait $LAST_PID
	LAST_STATUS=$?
	echo "respawn: $i: stopped pid $LAST_PID with status $LAST_STATUS"
	if [ $LAST_STATUS -ge 128 ]; then
		echo "respawn: process was terminated abnormally, not respawning"
		exit $LAST_STATUS
	fi
	if [ $SLEEP_BEFORE_RESPAWN -gt 0 ]; then
		echo "respawn: sleeping $SLEEP_BEFORE_RESPAWN seconds..."
		sleep $SLEEP_BEFORE_RESPAWN
	fi
	i=$(expr $i + 1)
done
echo "respawn: exiting after $max_i runs"
