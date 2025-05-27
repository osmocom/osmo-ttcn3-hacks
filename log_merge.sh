#!/bin/sh
# Copyright 2018 Harald Welte
# Copyright 2018-2025 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: Apache-2.0

# This script generates per-testcase merged logs.
# In order to work, you need to set the following test config:
#	[LOGGING]
#	LogFile := "%e-%c-%h-%r-%p.%s"
#
# the output files will be called "Module-Testcase.merged"

# OS#6787: Get the same prefix as for the pcap files by parsing the external
# command line from the logfile, e.g:
# 11:34:37.106541 STP_Tests_M3UA.ttcn:877 Starting external command `/home/user/…/ttcn3-tcpdump-start.sh STP_Tests_M3UA.TC_rkm_unreg_active'.
get_new_prefix() {
	local prefix="$1"
	local prefix_new
	local start_cmd_line

	for i in "$prefix"-*.log; do
		if ! [ -e "$i" ]; then
			continue
		fi

		start_cmd_line="$(grep -o 'Starting external command `.*-start.sh.*' "$i")"
		if [ -z "$start_cmd_line" ]; then
			continue
		fi

		prefix_new="$(echo "${start_cmd_line##* }" | cut -d "'" -f 1)"
		if [ -z "$prefix_new" ]; then
			continue
		fi

		echo "$prefix_new"
		return
	done

	>&2 echo "log_merge: WARNING: get_new_prefix failed for: $prefix"
	echo "$prefix"
}

if [ "x$1" = "x" ]; then
	echo "You have to specify the Test Suite prefix"
	exit 2
fi

BASE_NAME="$1"
LOG_FILES="$BASE_NAME*.log"

TEST_CASES=$(ls -1 $LOG_FILES | awk 'BEGIN { FS = "-" } { print $2 }' | sort | uniq)

if [ -n "$TEST_CASES" ]; then
	for t in $TEST_CASES; do
		PREFIX="$BASE_NAME-$t"
		OUTPUT="$(get_new_prefix "$PREFIX").merged"
		if [ -e "$OUTPUT" ]; then
			>&2 echo "log_merge: ERROR: file already exists: $OUTPUT"
			exit 1
		fi
		ttcn3_logmerge $PREFIX-*.log > "$OUTPUT"
		echo "Generated $OUTPUT"
	done
else
	>&2 echo "log_merge: WARNING: Couldn't find logs for test cases! Merging everything"
	ttcn3_logmerge $LOG_FILES > "$BASE_NAME.merged"
fi

if [ "$2" = "--rm" ]; then
	echo "Removing Input log files !!!"
	rm $LOG_FILES
fi
