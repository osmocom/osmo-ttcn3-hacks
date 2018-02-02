#!/bin/sh

# Helper script to starte a TITAN-generated test suite, supporting
# dynamically linked suites to ensure JUNIT generation is possible.

if [ $# -lt 1 ]; then
	echo "You have to specify the test suite name"
	echo "Syntax example: $0 osmo-ttcn3-hacks/ggsn_tests/GGSN_Test ./GGSN_Test.cfg"
	exit 1
fi

SUITE=$1
CFG=`basename $SUITE`.cfg
if [ $# -gt 1 ]; then
	CFG=$2
fi

if [ $# -gt 2 ]; then
	TEST=$3
fi

LD_LIBRARY_PATH=`dirname $SUITE`:/usr/lib/titan:/usr/ttcn3/lib ttcn3_start $SUITE $CFG $TEST
