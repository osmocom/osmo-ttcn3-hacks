#!/bin/sh

PIDFILE_PCAP=/tmp/pcap.pid
PIDFILE_NETCAT=/tmp/netcat.pid
TESTCASE=$1
VERDICT="$2"

if ! [ "$(id -u)" = "0" ]; then
	SUDOSTR="sudo -n"
else
	SUDOSTR=""
fi

kill_rm_pidfile() {
	if ! [ -e "$1" ] && [ -s "$1" ]; then
		$SUDOSTR kill "$(cat "$1")" 2>&1 || grep -v "No such process"
		rm $1
	fi
}

date

if [ x"$VERDICT" = x"pass" ]; then
	echo -e "\033[1;32m====== $TESTCASE $VERDICT ======\033[0m"
else
	echo -e "\033[1;31m------ $TESTCASE $VERDICT ------\033[0m"
fi
echo

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

# Wait for up to 2 seconds if we keep receiving traffinc from packet dumper,
# otherwise we might lose last packets from test.
i=0
prev_count=-1
count=$(stat --format="%s" "$TTCN3_PCAP_PATH/$TESTCASE.pcapng")
while [ $count -gt $prev_count ] && [ $i -lt 2 ]
do
	echo "Waiting for packet dumper to finish... $i (prev_count=$prev_count, count=$count)"
	sleep 1
	prev_count=$count
	count=$(stat --format="%s" "$TTCN3_PCAP_PATH/$TESTCASE.pcapng")
	i=$((i+1))
done

kill_rm_pidfile "$PIDFILE_PCAP"
kill_rm_pidfile "$PIDFILE_NETCAT"

gzip -f "$TTCN3_PCAP_PATH/$TESTCASE.pcapng"
