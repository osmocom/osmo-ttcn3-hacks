#!/bin/sh

PIDFILE=/tmp/tcpdump.pid
TESTCASE=$1
VERDICT="$2"

if [ x"$VERDICT" = x"pass" ]; then
	echo "\033[1;32m====== $TESTCASE $VERDICT ======\033[0m"
else
	echo "\033[1;31m------ $TESTCASE $VERDICT ------\033[0m"
fi
echo

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

# Wait for up to 2 seconds if we keep receiving traffinc from tcpdump,
# otherwise we might lose last packets from test.
i=0
prev_count=-1
count=$(stat --format="%s" "$TTCN3_PCAP_PATH/$TESTCASE.pcap")
while [ $count -gt $prev_count ] && [ $i -lt 2 ]
do
	echo "Waiting for tcpdump to finish... $i (prev_count=$prev_count, count=$count)"
	sleep 1
	prev_count=$count
	count=$(stat --format="%s" "$TTCN3_PCAP_PATH/$TESTCASE.pcap")
	i=$((i+1))
done

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
