#!/bin/sh

. "$(dirname "$0")/_scripts/tcpdump-dumpcap.inc.sh"

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
	# dumpcap, *after dropping permissions*, needs to be able to write to the directory to create the pcap file:
	if [ "$(stat -L -c "%u" "$TTCN3_PCAP_PATH")" = "$(id -u)" ] && [ "$(stat -L -c "%A" "$TTCN3_PCAP_PATH" | head -c 4)" = "drwx" ]; then
		CMD="$DUMPCAP -q"
	else
		echo "NOTE: unable to use dumpcap due to missing permissions in $TTCN3_PCAP_PATH"
	fi
    else
 	echo "NOTE: unable to use dumpcap due to missing capabilities or suid bit"
    fi
fi

# Create a dummy sink for GSMTAP packets
$NETCAT -l -u -k -p $GSMTAP_PORT >/dev/null 2>$TESTCASE.netcat.stderr &
PID=$!
echo $PID > $PIDFILE_NETCAT

CMD_OUTFILE=$TTCN3_PCAP_PATH/$TESTCASE.pcap.stdout
CMD_OUTFILE_ERR=$TTCN3_PCAP_PATH/$TESTCASE.pcap.stderr
FIFO=/tmp/cmderr
if ! [ -e $FIFO ]; then
	mkfifo $FIFO
else
	echo "Warning: Named pipe already exists: $FIFO"
fi

# Log stderr to CMD_OUTFILE and a dedicated error log file
tee $CMD_OUTFILE < $FIFO > $CMD_OUTFILE_ERR &
CMD_STR="$CMD -s 1520 -n -i any -w \"$TTCN3_PCAP_PATH/$TESTCASE.pcap\" >$CMD_OUTFILE 2>$FIFO &"
echo "$CMD_STR"
eval $CMD_STR
# $CMD -s 1520 -n -i any -w \"$TTCN3_PCAP_PATH/$TESTCASE.pcap\" >$CMD_OUTFILE &
PID=$!
echo $PID > $PIDFILE_PCAP
if [ -f $CMD_OUTFILE_ERR ] && [ $(wc -l $CMD_OUTFILE_ERR | awk '{print $1}') -ne 0 ]; then
	echo "Warnings or error messages from command:" >&2
	echo "	$CMD_STR" >&2
	echo "Message:" >&2
	echo "$(cat $CMD_OUTFILE_ERR)" | sed 's/^/\t/' >&2
fi

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
		echo "Packet dumper didn't start filling pcap file after $i seconds!!!"
		break
	fi
done
kill $PID

echo "$TESTCASE" > "$TTCN3_PCAP_PATH/.current_test"
