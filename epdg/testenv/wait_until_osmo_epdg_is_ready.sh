#!/bin/sh -e
LOGFILE="$(basename "$PWD").log"

# Wait until osmo-epdg is ready and fail early if errors are logged during
# startup. Otherwise osmo-epdg may keep running and we get less obvious
# failures later.
epdg_wait() {
	i=0
	for i in $(seq 1 150); do
		sleep 0.1

		if grep -q "\[error\]" "$LOGFILE"; then
			# The log output is visible in stdout
			exit 1
		fi

		if grep -q "run_osmo_epdg_with_dummy_ue.sh: osmo-epdg is ready" "$LOGFILE"; then
			exit 0
		fi
	done

	echo "ERROR: wait_until_osmo_epdg_is_ready.sh: osmo-epdg failed to start"
	exit 1
}

qemu_wait.sh
epdg_wait
