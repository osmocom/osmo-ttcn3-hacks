#!/bin/sh

PIDFILE=/tmp/dumper.pid
TESTCASE=$1
VERDICT="$2"

date

if [ x"$VERDICT" = x"pass" ]; then
	echo "\033[1;32m====== $TESTCASE $VERDICT ======\033[0m"
else
	echo "\033[1;31m------ $TESTCASE $VERDICT ------\033[0m"
fi
echo

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

# Wait for up to 2 seconds if we keep receiving traffinc from packet dumper,
# otherwise we might lose last packets from test.
i=0
prev_count=-1
count=$(stat --format="%s" "$TTCN3_PCAP_PATH/$TESTCASE.pcap")
while [ $count -gt $prev_count ] && [ $i -lt 2 ]
do
	echo "Waiting for packet dumper to finish... $i (prev_count=$prev_count, count=$count)"
	sleep 1
	prev_count=$count
	count=$(stat --format="%s" "$TTCN3_PCAP_PATH/$TESTCASE.pcap")
	i=$((i+1))
done

if [ -e $PIDFILE ]; then
        DUMPER="$(ps -q "$(cat "$PIDFILE")" -o comm=)"
	if [ "$DUMPER" != "sudo" ]; then
		kill "$(cat "$PIDFILE")"
	else
	# NOTE: This requires you to be root or something like
	# "laforge ALL=NOPASSWD: /usr/sbin/tcpdump, /bin/kill" in your sudoers file
		sudo kill "$(cat "$PIDFILE")"
	fi
	rm $PIDFILE
fi
