#!/bin/sh
osmo-bts-trx -c osmo-bts.cfg
EXIT_CODE=$?

if [ $EXIT_CODE != 0 ] && grep -q "SCHED_RR.*Operation not permitted" bts.log; then
	echo
	echo "===================================================="
	echo "Allow your user to set rtprio, logout and try again:"
	echo "$ echo '$USER - rtprio 30' | sudo tee '/etc/security/limits.d/${USER}_allow-rtprio.conf'"
	echo "===================================================="
	echo
	exit 128  # Don't respawn
fi

exit $EXIT_CODE
