#!/bin/sh -e
# Simple watchdog script that exits if either:
# * testenv doesn't create /tmp/watchdog every 10s
# * 4 hours have passed
# This ensures the podman container stops a few seconds after a jenkins job was
# aborted, or if a test is stuck in a loop for hours.

stop_time=$(($(date +%s) + 3600 * 4))

while [ $(date +%s) -lt $stop_time ]; do
	sleep 10

	if ! [ -e /tmp/watchdog ]; then
		break
	fi

	rm /tmp/watchdog
done

exit 1
