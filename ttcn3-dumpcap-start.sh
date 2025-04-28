#!/bin/bash
#
# contrary to ttcn3-tcpdump-start.sh, this version is dumpcap-only and
# needed when we want to capture from interfaces of different link
# types. It will also store the results as pcap-ng, not plain old pcap.

. "$(dirname "$0")/_scripts/tcpdump-dumpcap.inc.sh"

PIDFILE_PCAP=/tmp/pcap.pid
DUMPCAP=/usr/bin/dumpcap

PIDFILE_NETCAT=/tmp/netcat.pid
NETCAT=/bin/nc
GSMTAP_PORT=4729

TESTCASE=$1

echo "------ $TESTCASE ------"
date

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

kill_rm_pidfile $PIDFILE_NETCAT
kill_rm_pidfile $PIDFILE_PCAP

if [ ! -x "$DUMPCAP" ]; then
	echo "Missing required dumpcap binary at ${DUMPCAP}"
	exit 31
fi

if [ "$(id -u)" = "0" ]; then
	CMD="$DUMPCAP -q"
else
    CAP_ERR="1"
    if [ -x /sbin/setcap ]; then
	# N. B: this check requires libcap2-bin package
	/sbin/setcap -q -v 'cap_net_admin,cap_net_raw=pie' $DUMPCAP
	CAP_ERR="$?"
    fi
    if [ -u $DUMPCAP -o "$CAP_ERR" = "0" ]; then
	CMD="$DUMPCAP -q"
    else
 	echo "NOTE: unable to use dumpcap due to missing capabilities or suid bit"
	exit 32
    fi
fi

# Create a dummy sink for GSMTAP packets
$NETCAT -l -u -k -p $GSMTAP_PORT >/dev/null 2>$TESTCASE.netcat.stderr &
PID=$!
echo $PID > $PIDFILE_NETCAT

# generate the list of interface arguments.  For capturing from
# interfaces of different link-layer types, we cannot use "-i all"
# but must use dumpcap with each individual interface name.  We also
# must write pcapng files, as only those can record the interface of
# each packet
ADDL_ARGS="-i lo"
for f in /sys/class/net/*; do
	DEV=`basename $f`
	if [[ "$DEV" == "hdlcnet"* ]]; then
		# skip these as we only want the hdlcX devices, avoid capturing twice on both sides
		continue
	elif [[ "$DEV" == "hdlc"* ]]; then
		# these are the user-side of the FR links, which is
		# what we interface with from our test suite, emulating
		# a BSS.
		ADDL_ARGS="${ADDL_ARGS} -i ${DEV}"
	elif [[ "$DEV" == "eth"* ]]; then
		# we blindly assume that "normal" docker network devices
		# are called ethXXX
		ADDL_ARGS="${ADDL_ARGS} -i ${DEV}"
	fi
done

$CMD -s 1520 -n ${ADDL_ARGS} -w "$TTCN3_PCAP_PATH/$TESTCASE.pcapng" >$TTCN3_PCAP_PATH/$TESTCASE.pcapng.stdout 2>&1 &
PID=$!
echo $PID > $PIDFILE_PCAP

# Wait until packet dumper creates the pcap file and starts recording.
# We generate some traffic until we see packet dumper catches it.
# Timeout is 10 seconds.
ping 127.0.0.1 >/dev/null 2>&1 &
PID=$!
i=0
while [ ! -f "$TTCN3_PCAP_PATH/$TESTCASE.pcapng" ] ||
      [ "$(stat -c '%s' "$TTCN3_PCAP_PATH/$TESTCASE.pcapng")" -eq 32 ]
do
	echo "Waiting for packet dumper to start... $i"
	sleep 1
	i=$((i+1))
	if [ $i -eq 10 ]; then
		break
	fi
done
kill $PID
