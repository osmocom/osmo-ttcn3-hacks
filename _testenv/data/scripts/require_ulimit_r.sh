#!/bin/sh
MIN=$1

if [ -z "$MIN" ]; then
	echo "usage: require_ulimit_r.sh MIN"
	exit 1
fi

if [ -z "$USER" ]; then
	USER="$HOST_USER"
fi

if [ $(ulimit -r) -lt $MIN ]; then
	echo
	echo "===================================================="
	echo "Allow your user to set rtprio, logout and try again:"
	echo "$ echo '$USER - rtprio $MIN' | sudo tee '/etc/security/limits.d/$USER-allow-rtprio.conf'"
	echo "===================================================="
	echo
	exit 1
fi
