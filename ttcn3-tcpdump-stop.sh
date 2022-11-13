#!/bin/sh

PIDFILE_PCAP=/tmp/pcap.pid
PIDFILE_NETCAT=/tmp/netcat.pid
FIFO=/tmp/cmderr
TESTCASE=$1
VERDICT="$2"

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

date

# -e only works/is required only in Bash; in dash/POSIX shells it isn't required and will be part of the output
SHELL="$(cat /proc/$$/cmdline |  tr -d '\0')"
if [ "$SHELL" = "bash" ]; then
	ESCAPE_OPT="-e"
else
	ESCAPE_OPT=""
fi

if [ x"$VERDICT" = x"pass" ]; then
	echo $ESCAPE_OPT "\033[1;32m====== $TESTCASE $VERDICT ======\033[0m"
else
	echo $ESCAPE_OPT "\033[1;31m------ $TESTCASE $VERDICT ------\033[0m"
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
rm $FIFO

gzip -f "$TTCN3_PCAP_PATH/$TESTCASE.pcap"
