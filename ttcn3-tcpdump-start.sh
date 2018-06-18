#!/bin/sh

PIDFILE_PCAP=/tmp/pcap.pid
TCPDUMP=/usr/sbin/tcpdump
DUMPCAP=/usr/bin/dumpcap

PIDFILE_NETCAT=/tmp/netcat.pid
NETCAT=/bin/nc
GSMTAP_PORT=4729

TESTCASE=$1

kill_rm_pidfile() {
	if [ -e $1 ]; then
		kill "$(cat "$1")"
		rm $1
	fi
}

echo "------ $TESTCASE ------"
date

exit 0

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

kill_rm_pidfile $PIDFILE_NETCAT
kill_rm_pidfile $PIDFILE_PCAP

if [ "$(id -u)" = "0" ]; then
	CMD="$TCPDUMP -U"
else
# NOTE: This requires you to be root or something like
# "laforge ALL=NOPASSWD: /usr/sbin/tcpdump, /bin/kill" in your sudoers file
	CMD="sudo $TCPDUMP -U"
fi

if [ -x $DUMPCAP ]; then
    CAP_ERR="1"
    if [ -x /sbin/setcap ]; then
	# N. B: this check requires libcap2-bin package
	setcap -q -v 'cap_net_admin,cap_net_raw=pie' $DUMPCAP
	CAP_ERR="$?"
    fi
    if [ -u $DUMPCAP -o "$CAP_ERR" = "0" ]; then
	CMD="$DUMPCAP -q"
    else
 	echo "NOTE: unable to use dumpcap due to missing capabilities or suid bit"
    fi
fi

# Create a dummy sink for GSMTAP packets
$NETCAT -l -u -k -p $GSMTAP_PORT >/dev/null 2>$TESTCASE.netcat.stderr &
PID=$!
echo $PID > $PIDFILE_NETCAT

$CMD -s 1500 -n -i any -w "$TTCN3_PCAP_PATH/$TESTCASE.pcap" >$TTCN3_PCAP_PATH/$TESTCASE.pcap.stdout 2>&1 &
PID=$!
echo $PID > $PIDFILE_PCAP

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
