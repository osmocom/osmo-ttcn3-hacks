#!/bin/sh

PIDFILE_PCAP=/tmp/pcap.pid
PIDFILE_NETCAT=/tmp/netcat.pid
TESTCASE=$1
VERDICT="$2"

kill_rm_pidfile() {
if [ -e $1 ]; then
        PSNAME="$(ps -q "$(cat "$1")" -o comm=)"
	if [ "$PSNAME" != "sudo" ]; then
		kill "$(cat "$1")"
	else
	# NOTE: This requires you to be root or something like
	# "laforge ALL=NOPASSWD: /usr/sbin/tcpdump, /bin/kill" in your sudoers file
		sudo kill "$(cat "$1")"
	fi
	rm $1
fi
}

date

if [ x"$VERDICT" = x"pass" ]; then
	printf "\033[1;32m====== $TESTCASE $VERDICT ======\033[0m\n\n"
else
	printf "\033[1;31m------ $TESTCASE $VERDICT ------\033[0m\n\n"
fi

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

# Add a numeral suffix to subsequent runs of the same test:
PCAP_FILENAME=$TTCN3_PCAP_PATH/$TESTCASE.pcapng
if [ -f "$TTCN3_PCAP_PATH/$TESTCASE.pcapng.gz" ]; then
       i=1
       while [ -f "$TTCN3_PCAP_PATH/$TESTCASE.$i.pcapng.gz" ];
               do i=$((i+1))
       done
       mv "$PCAP_FILENAME" "$TTCN3_PCAP_PATH/$TESTCASE.$i.pcapng"
       PCAP_FILENAME="$TTCN3_PCAP_PATH/$TESTCASE.$i.pcapng"
fi
gzip -f "$PCAP_FILENAME"
