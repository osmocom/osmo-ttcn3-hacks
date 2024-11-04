#!/bin/sh -e
# Simple watchdog script that exits if either:
# * testenv doesn't create /tmp/watchdog every 60s
# * 4 hours have passed
# This ensures the podman container stops a soon after a jenkins job was
# aborted, or if a test is stuck in a loop for hours.

echo "Running testenv-podman-main.sh"

stop_time=$(($(date +%s) + 3600 * 4))

while [ $(date +%s) -lt $stop_time ]; do
	sleep 60

	if ! [ -e /tmp/watchdog ]; then
		echo "ERROR: /tmp/watchdog was not created, exiting"
		exit 1
	fi

	rm /tmp/watchdog
done

echo "ERROR: timeout reached!"
exit 1
