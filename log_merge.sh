#!/bin/sh

# This script generates per-testcase merged logs.
# In order to work, you need to set the following test config:
#	[LOGGING]
#	LogFile := "%e-%c-%h-%r.%s"
#
# the output files will be called "Module-Testcase.merged"

# Copyright 2018 Harald Welte
# Copyright 2018 sysmocom - s.f.m.c. GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
	ttcn3_logformat "$OUTPUT" > "_$OUTPUT"
	echo "Generated _$OUTPUT"
done

if [ "$2" = "--rm" ]; then
	echo "Removing Input log files !!!"
	rm $LOG_FILES
fi
