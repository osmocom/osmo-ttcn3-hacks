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
$CMD -s 0 -n -i any -w "$TTCN3_PCAP_PATH/$TESTCASE.pcap" >$TTCN3_PCAP_PATH/$TESTCASE.pcap.log 2>&1 &
PID=$!
echo $PID > $PIDFILE

# Wait until tcpdump creates the pcap file to give it some time to start listenting.
# Timeout is 10 seconds.
i=0
while [ ! -f "$TTCN3_PCAP_PATH/$TESTCASE.pcap" ] && [ $i -lt 10 ]
do
	echo "Waiting for tcpdump to start... $i"
	sleep 1
	i=$((i+1))
done
