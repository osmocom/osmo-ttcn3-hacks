#!/bin/sh

PIDFILE=/tmp/tcpdump.pid

if [ -e $PIDFILE ]; then
	# NOTE: This requires you to be root or something like
	# "laforge ALL=NOPASSWD: /usr/sbin/tcpdump, /bin/kill" in your sudoers file
	if [ "$(id -u)" = "0" ]; then
		kill "$(cat "$PIDFILE")"
	else
		sudo kill "$(cat "$PIDFILE")"
	fi
	rm $PIDFILE
fi
