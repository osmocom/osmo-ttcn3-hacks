#!/bin/sh

PIDFILE_PCAP=/tmp/pcap.pid
TCPDUMP=$(command -v tcpdump)
DUMPCAP=$(command -v dumpcap)

PIDFILE_NETCAT=/tmp/netcat.pid
NETCAT=$(command -v nc)
GSMTAP_PORT=4729

TESTCASE=$1


SUDOSTR=""
if ! [ "$(id -u)" = "0" ]; then
	SUDOSTR="sudo -n"
	# Otherwise, if sudo /usr/bin/kill, sudo /usr/bin/tcpdump cannot be run without a password prompt,
	# and this script will hang indefinitely
fi

kill_rm_pidfile() {
	# NOTE: This requires you to be root or something like
	# "laforge ALL=NOPASSWD: /usr/sbin/tcpdump, /bin/kill" in your sudoers file
	if ! [ -e "$1" ] && [ -s "$1" ]; then
		$SUDOSTR kill "$(cat "$1")" 2>&1 | grep -v "No such process"
		rm $1
	fi
}

echo "------ $TESTCASE ------"
date

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

kill_rm_pidfile $PIDFILE_NETCAT
kill_rm_pidfile $PIDFILE_PCAP

CMD="$SUDOSTR $TCPDUMP -U"

if [ -x "$DUMPCAP" ]; then
    CAP_ERR="1"
    if [ -x /sbin/setcap ]; then
	# N. B: this check requires libcap2-bin package
	/sbin/setcap -q -v 'cap_net_admin,cap_net_raw=pie' $DUMPCAP
	CAP_ERR="$?"
    fi

    # did we implicitly inherit all those caps because we're root in a netns?
    if [ -u $DUMPCAP -o "$CAP_ERR" = "1" ]; then
	getpcaps 0 2>&1 | grep -e cap_net_admin | grep -q -e cap_net_raw
	CAP_ERR="$?"
    fi

    # did we implicitly inherit all those caps because we're root in a netns?
    if [ -u $DUMPCAP -o "$CAP_ERR" = "1" ]; then
	getpcaps 0 2>&1 | grep -q -e " =ep" # all perms
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
