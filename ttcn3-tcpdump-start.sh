#!/bin/sh

PIDFILE=/tmp/tcpdump.pid
TESTCASE=$1

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

if [ -e $PIDFILE ]; then
	kill "$(cat "$PIDFILE")"
	rm $PIDFILE
fi

# NOTE: This requires you to be root or something like
# "laforge ALL=NOPASSWD: /usr/sbin/tcpdump, /bin/kill" in your sudoers file
if [ "$(id -u)" = "0" ]; then
	CMD=/usr/sbin/tcpdump
else
	CMD="sudo /usr/sbin/tcpdump"
fi
$CMD -s 0 -n -i any -w "$TTCN3_PCAP_PATH/$TESTCASE.pcap" >/dev/null 2>&1 &
PID=$!
echo $PID > $PIDFILE
