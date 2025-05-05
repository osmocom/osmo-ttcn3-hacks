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

if [ "x$1" = "x" ]; then
	echo "You have to specify the Test Suite prefix"
	exit 2
fi

BASE_NAME="$1"
LOG_FILES="$BASE_NAME*.log"

TEST_CASES=$(ls -1 $LOG_FILES | awk 'BEGIN { FS = "-" } { print $2 }' | sort | uniq)

for t in $TEST_CASES; do
	PREFIX="$BASE_NAME-$t"
	OUTPUT="$BASE_NAME.$t.merged"
	ttcn3_logmerge $PREFIX-*.log > "$OUTPUT"
	echo "Generated $OUTPUT"
done

if [ "$2" = "--rm" ]; then
	echo "Removing Input log files !!!"
	rm $LOG_FILES
fi
