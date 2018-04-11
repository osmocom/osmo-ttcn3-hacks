#!/bin/sh

# Helper script to starte a TITAN-generated test suite, supporting
# dynamically linked suites to ensure JUNIT generation is possible.

if [ $# -lt 1 ]; then
	echo "You have to specify the test suite name"
	echo "Syntax example: $0 osmo-ttcn3-hacks/ggsn_tests/GGSN_Test ./GGSN_Test.cfg"
	exit 1
fi

SUITE=$1
SUITE_DIR="$(dirname "$SUITE")"
SUITE_NAME="$(basename "$SUITE")"
CFG="$SUITE_NAME.cfg"
if [ $# -gt 1 ]; then
	CFG=$2
fi

if [ $# -gt 2 ]; then
	TEST=$3
fi

LD_LIBRARY_PATH="$SUITE_DIR:/usr/lib/titan:/usr/ttcn3/lib" ttcn3_start $SUITE $CFG $TEST

expected="$SUITE_DIR/expected-results.xml"
if [ ! -f "$expected" ]; then
  echo "No expected results found, not comparing outcome. ($expected)"
  exit 0
fi

# find the most recent junit output log here
last_log="$(ls -1tr junit*.log | tail -n 1)"
if [ ! -f "$last_log" ]; then
  echo "No junit log found."
  exit 1
fi

compare="$SUITE_DIR/../compare-results.sh"
if [ ! -x "$compare" ]; then
  echo "ERROR: cannot find $compare"
  exit 1
fi

set -e
"$compare" "$expected" "$last_log" $OSMO_TTCN3_COMPARE_ARGS
