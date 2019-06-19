#!/usr/bin/env bash
expected_file="$1"
results_file="$2"

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

usage() {
  echo "
Usage:

  $(basename "$0") expected_results.junit-log current_results.junit-log [--allow-* [...]]

Return 0 if the expected results match the current results exactly.

  --allow-skip   Allow runnning less tests than are listed in the expected file.
                 Default is to return failure on any skipped tests.
  --allow-new    Allow more test results than found in the expected file.
                 Default is to return failure on any unknown tests.
  --allow-xpass  If a test was expected to fail but passed, return success.
                 Default is to return failure on any mismatch.
"
}

if [ ! -f "$expected_file" ]; then
  usage
  echo "Expected file not found: '$expected_file'"
  exit 1
fi

if [ ! -f "$results_file" ]; then
  usage
  echo "Current results file not found: '$results_file'"
  exit 1
fi

shift
shift

allow_xpass=0
allow_skip=0
allow_new=0

while test -n "$1"; do
  arg="$1"
  if [ "x$arg" = "x--allow-xpass" ]; then
    allow_xpass=1
  elif [ "x$arg" = "x--allow-skip" ]; then
    allow_skip=1
  elif [ "x$arg" = "x--allow-new" ]; then
    allow_new=1
  else
    usage
    echo "Unknown argument: '$arg'"
    exit 1
  fi
  shift
done

echo "Comparing expected results $expected_file against results in $results_file
--------------------"

parse_testcase() {
  line="$1"
  suite_name="$(echo "$line" | sed 's,.*classname='"'"'\([^'"'"']*\)'"'"'.*,\1,')"
  test_name="$(echo "$line" | sed 's,.*\<name='"'"'\([^'"'"']*\)'"'"'.*,\1,')"
  if [ -n "$(echo "$line" | grep '/>$')" ]; then
    test_result="pass"
  else
    test_result="FAIL"
  fi
}

pass=0
xfail=0
more_failures=0
more_successes=0
skipped=0
new=0

while read line; do
  parse_testcase "$line"
  exp_suite_name="$suite_name"
  exp_test_name="$test_name"
  exp_test_result="$test_result"
  matched="0"

  while read line; do
    parse_testcase "$line"
    if [ "x$exp_suite_name" != "x$suite_name" ]; then
      continue
    fi
    if [ "x$exp_test_name" != "x$test_name" ]; then
      continue
    fi

    if [ "x$exp_test_result" = "x$test_result" ]; then
      if [ "x$exp_test_result" = "xFAIL" ]; then
        exp_test_result="xfail"
	(( xfail += 1 ))
      else
        (( pass += 1 ))
      fi
      echo "$exp_test_result $suite_name.$test_name"
    else
      if [ "x$exp_test_result" = "xFAIL" ]; then
        exp_test_result="xfail"
      fi
      echo "$exp_test_result->$test_result $suite_name.$test_name"
      if [ "x$test_result" = "xFAIL" ]; then
        (( more_failures += 1 ))
      else
	(( more_successes += 1 ))
      fi
    fi
    matched="1"
    break
  done <<< "$(grep "<testcase.*$exp_test_name" "$results_file")"

  if [ "x$matched" = "x0" ]; then
    echo "skipped $exp_suite_name.$exp_test_name"
    (( skipped += 1 ))
  fi

done <<< "$(grep "<testcase" "$expected_file")"

# Also catch all new tests that aren't covered in the expected results
while read line; do
  parse_testcase "$line"
  got_suite_name="$suite_name"
  got_test_name="$test_name"
  got_test_result="$test_result"
  matched="0"

  while read line; do
    parse_testcase "$line"
    if [ "x$got_suite_name" != "x$suite_name" ]; then
      continue
    fi
    if [ "x$got_test_name" != "x$test_name" ]; then
      continue
    fi

    matched="1"
    break
  done <<< "$(grep "<testcase.*$test_name" "$expected_file")"

  if [ "x$matched" = "x0" ]; then
    echo "NEW-$got_test_result $got_suite_name.$got_test_name"
    (( new += 1 ))
  fi

done <<< "$(grep "<testcase" "$results_file")"

echo "--------------------"
overall_verdict=0

ask_update=""

if [ "x$pass" != x0 ]; then
  echo "$pass pass"
fi

if [ "x$xfail" != x0 ]; then
  echo "$xfail xfail"
fi

if [ "x$skipped" != x0 ]; then
  echo "$skipped skipped"
  ask_update="$ask_update removed=$skipped"
  if [ "x$allow_skip" = x0 ]; then
    overall_verdict=4
  fi
fi

if [ "x$new" != x0 ]; then
  echo "$new new"
  ask_update="$ask_update new=$new"
  if [ "x$allow_new" = x0 ]; then
    overall_verdict=3
  fi
fi

if [ "x$more_successes" != x0 ]; then
  echo "$more_successes pass unexpectedly"
  ask_update="$ask_update xpass=$more_successes"
  if [ "x$allow_xpass" = x0 ]; then
    overall_verdict=2
  fi
fi

if [ "x$more_failures" != x0 ]; then
  echo "$more_failures FAIL"
  overall_verdict=1
fi

if [ -n "$ask_update" ]; then
  echo
  echo "(Please update the expected results:$ask_update)"
fi

exit $overall_verdict
