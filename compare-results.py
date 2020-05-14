#!/usr/bin/env python3
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
import argparse
import re

doc = "Compare TTCN3 test run results with expected results by junit logs."

# The nicest would be to use an XML library, but I don't want to introduce dependencies on the build slaves.
re_testcase = re.compile(r'''<testcase classname=['"]([^'"]+)['"].* name=['"]([^'"]+)['"].*>''')
re_testcase_end = re.compile(r'''(</testcase>|<testcase [^>]*/>)''')
re_failure = re.compile(r'''(<failure\b|<error\b)''')

RED = "\033[1;31m"
GREEN = "\033[1;32m"
YELLOW = "\033[1;33m"
BLUE = "\033[1;34m"
NOCOLOR = "\033[0;m"

def col(color, text):
	return color + text + NOCOLOR

RESULT_PASS = col(GREEN, 'pass')
RESULT_FAIL = col(RED, 'pass->FAIL')
RESULT_SKIP = col(BLUE, 'skip')
RESULT_XFAIL = col(YELLOW, 'xfail')
RESULT_FIXED = col(GREEN, 'xfail->PASS')
RESULT_NEW_PASS = col(GREEN, 'NEW: PASS')
RESULT_NEW_FAIL = col(RED, 'NEW: FAIL')

RESULTS = (
	RESULT_FAIL,
	RESULT_NEW_FAIL,
	RESULT_XFAIL,
	RESULT_FIXED,
	RESULT_PASS,
	RESULT_NEW_PASS,
	RESULT_SKIP,
	)

def count(counter, name, result):
	v = counter.get(result) or 0
	v += 1
	counter[result] = v
	if result != RESULT_SKIP:
		print('%s %s' % (result, name))

def compare_one(name, expect, result, counter):
	if result is None:
		count(counter, name, RESULT_SKIP)
	elif result == RESULT_PASS:
		if expect == RESULT_PASS:
			count(counter, name, RESULT_PASS)
		elif expect == RESULT_FAIL:
			count(counter, name, RESULT_FIXED)
		elif expect is None:
			count(counter, name, RESULT_NEW_PASS)
	elif result == RESULT_FAIL:
		if expect == RESULT_PASS:
			count(counter, name, RESULT_FAIL)
		elif expect == RESULT_FAIL:
			count(counter, name, RESULT_XFAIL)
		elif expect is None:
			count(counter, name, RESULT_NEW_FAIL)

def compare(cmdline, f_expected, f_current):
	expected_list = parse_results(f_expected)
	current_list = parse_results(f_current)

	expected_dict = dict(expected_list)
	current_dict = dict(current_list)

	counter = {}

	for expected_name, expected_result in expected_list:
		compare_one(expected_name, expected_result, current_dict.get(expected_name), counter)

	# Also count new tests
	for current_name, current_result in current_list:
		if current_name in expected_dict:
			continue
		compare_one(current_name, None, current_result, counter)


	print('\nSummary:')
	for r in RESULTS:
		v = counter.get(r)
		if not v:
			continue
		print('  %s: %d' % (r, v))
	print('\n')

def parse_results(f):
	tests = []
	name = None
	result = None
	for line in f:
		m = re_testcase.search(line)
		if m:
			class_name, test_name = m.groups()
			name = '%s.%s' % (class_name, test_name)

		m = re_failure.search(line)
		if m:
			result = RESULT_FAIL

		m = re_testcase_end.search(line)
		if m:
			if not name:
				continue
			if result is None:
				result = RESULT_PASS
			tests.append((name, result))

			name = None
			result = None

	return tests

def main(cmdline):
	with open(cmdline.expected_results, 'r') as f_expected:
		with open(cmdline.current_results, 'r') as f_current:
			print('\nComparing expected results %r against results in %r\n--------------------'
				% (cmdline.expected_results, cmdline.current_results))
			compare(cmdline, f_expected, f_current)

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description=doc)
	parser.add_argument('expected_results', metavar='expected.junit-xml',
			help='junit XML file listing the expected test results.')
	parser.add_argument('current_results', metavar='current.junit-xml',
			help='junit XML file listing the current test results.')

	cmdline = parser.parse_args()
	main(cmdline)
