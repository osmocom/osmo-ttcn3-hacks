#!/bin/sh
# Restart a given test component while the testsuite is running. Unlike
# respawn-expected.sh we don't expect this to happen but need to keep going
# anyway. We log that this happens to $TESTENV_END_LOG, which then gets
# displayed at the end of the testenv run.

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

	CURRENT_TEST="$(cat "$TESTENV_TESTDIR"/testsuite/.current_test)"
	if [ -n "$CURRENT_TEST" ]; then
		echo "Unexpected respawn of \"$*\" during test \"$CURRENT_TEST\"" >>"$TESTENV_END_LOG"
	else
		echo "Unexpected respawn of \"$*\"" >>"$TESTENV_END_LOG"
	fi

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
