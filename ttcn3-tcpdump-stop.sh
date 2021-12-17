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
	echo -e "\033[1;32m====== $TESTCASE $VERDICT ======\033[0m"
else
	echo -e "\033[1;31m------ $TESTCASE $VERDICT ------\033[0m"
fi
echo

if [ "z$TTCN3_PCAP_PATH" = "z" ]; then
	TTCN3_PCAP_PATH=/tmp
fi

# Order the SUT to print a talloc report
if [ "z$OSMO_SUT_HOST" != "z" ] && [ "z$OSMO_SUT_PORT" != "z" ]; then
	if [ -x "$(command -v osmo_interact_vty.py)" ]; then
		echo "Saving talloc report from $OSMO_SUT_HOST:$OSMO_SUT_PORT to $TESTCASE.talloc"
		if ! timeout 5 osmo_interact_vty.py \
			-H $OSMO_SUT_HOST -p $OSMO_SUT_PORT \
			-c "en; show talloc-context application full" \
				> "$TTCN3_PCAP_PATH/$TESTCASE.talloc"; then
			echo
			echo "ERROR: failed to get talloc report via vty"
			echo
		fi
	else
		echo "Missing osmo_interact_vty.py from osmo-python-tests!"
		echo " -> Unable to obtain talloc report from the SUT"
	fi
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

kill_rm_pidfile "$PIDFILE_PCAP"
kill_rm_pidfile "$PIDFILE_NETCAT"

gzip -f "$TTCN3_PCAP_PATH/$TESTCASE.pcap"
