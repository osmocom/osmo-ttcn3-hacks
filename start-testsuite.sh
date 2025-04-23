#!/bin/sh

# Helper script to starte a TITAN-generated test suite, supporting
# dynamically linked suites to ensure JUNIT generation is possible.

# Environment variables:
# * TTCN3_BIN_DIR: override where to look for ttcn3_start

# Copyright 2017 Harald Welte
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

if [ $# -lt 1 ]; then
	echo "You have to specify the test suite name"
	echo "Syntax example: $0 osmo-ttcn3-hacks/ggsn_tests/GGSN_Test ./GGSN_Test.cfg"
	exit 1
fi

TOPDIR="$(realpath "$(dirname "$0")")"
BUILDDIR="${BUILDDIR:-$TOPDIR/_build}"

SUITE=$1
SUITE_DIR="$(basename "$(dirname "$SUITE")")"
SUITE_NAME="$(basename "$SUITE")"
CFG="$SUITE_NAME.cfg"
if [ $# -gt 1 ]; then
	CFG=$2
fi

if [ $# -gt 2 ]; then
	TEST=$3
fi

if [ -z "$TTCN3_DIR" ]; then
	# below is for the debian packages
	TTCN3_BIN_DIR="${TTCN3_BIN_DIR:-/usr/bin}"
	TITAN_LIBRARY_PATH="${TITAN_LIBRARY_PATH:-/usr/lib/titan:/usr/ttcn3/lib}"
else
	# below is for Arch Linux packages
	# https://aur.archlinux.org/packages/eclipse-titan
	# https://aur.archlinux.org/packages/titan-git
	# ... and non-installed custom (e.g. git master) builds
	TTCN3_BIN_DIR="${TTCN3_DIR}/bin"
	TITAN_LIBRARY_PATH="${TTCN3_DIR}/lib"
fi

# Run ttcn3_start with LD_LIBRARY_PATH. Do not put $TEST in quotes as it can be
# empty and must be omitted in that case. Otherwise ttcn3_start tries to stop
# the MTC after the first test, which fails with "MTC cannot be terminated" and
# then ttcn3_start keeps running even after the testsuite has stopped.
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$BUILDDIR/$SUITE_DIR:$TITAN_LIBRARY_PATH" \
	"$TTCN3_BIN_DIR/ttcn3_start" \
	"$BUILDDIR/$SUITE_DIR/$SUITE_NAME" \
	"$CFG" \
	$TEST

expected="$TOPDIR/$SUITE_DIR/expected-results.xml"
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

compare="$TOPDIR/compare-results.py"
if [ ! -x "$compare" ]; then
  echo "ERROR: cannot find $compare"
  exit 1
fi

"$compare" "$expected" "$last_log" $OSMO_TTCN3_COMPARE_ARGS
