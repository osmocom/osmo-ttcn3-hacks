# This file gets included by ttcn3-{tcpdump,dumpcap}-{start,stop} scripts

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
