#!/bin/sh

PIDFILE=/tmp/dumper.pid
TCPDUMP=/usr/sbin/tcpdump
TESTCASE=$1

echo "------ $TESTCASE ------"
date

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

if [ -e $PIDFILE ]; then
	kill "$(cat "$PIDFILE")"
	rm $PIDFILE
fi

if [ "$(id -u)" = "0" ]; then
	CMD="$TCPDUMP -U"
else
# NOTE: This requires you to be root or something like
# "laforge ALL=NOPASSWD: /usr/sbin/tcpdump, /bin/kill" in your sudoers file
	CMD="sudo $TCPDUMP -U"
fi
$CMD -s 1500 -n -i any -w "$TTCN3_PCAP_PATH/$TESTCASE.pcap" >$TTCN3_PCAP_PATH/$TESTCASE.pcap.stdout 2>&1 &
PID=$!
echo $PID > $PIDFILE

# Wait until packet dumper creates the pcap file and starts recording.
# We generate some traffic until we see packet dumper catches it.
# Timeout is 10 seconds.
ping 127.0.0.1 >/dev/null 2>&1 &
PID=$!
i=0
while [ ! -f "$TTCN3_PCAP_PATH/$TESTCASE.pcap" ] ||
      [ "$(stat -c '%s' "$TTCN3_PCAP_PATH/$TESTCASE.pcap")" -eq 32 ]
do
	echo "Waiting for packet dumper to start... $i"
	sleep 1
	i=$((i+1))
	if [ $i -eq 10 ]; then
		break
	fi
done
kill $PID
