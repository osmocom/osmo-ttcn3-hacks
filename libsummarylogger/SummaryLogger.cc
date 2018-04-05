/******************************************************************************
 * Copyright (c) 2018 sysmocom - s.f.m.c. GmbH
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *   Neels Hofmeyr
 *
 ******************************************************************************/
#include "SummaryLogger.hh"

#include <unistd.h>
#include <sys/types.h>

#include <sys/time.h>

extern "C" {
	ILoggerPlugin *create_plugin() { return new SummaryLogger(); }
	void destroy_plugin(ILoggerPlugin *plugin) { delete plugin; }
}


TestCase::TestCase()
{
	reset();
}

void TestCase::reset()
{
	verdict = Unbound;
	expected_verdict = NULL;
	tc_name = "";
	module_name = "";
}

TestSuite::~TestSuite()
{
	for (TestCases::const_iterator it = testcases.begin(); it != testcases.end(); ++it) {
		delete (*it);
	}
}

SummaryLogger::SummaryLogger()
	: filename_stem_(NULL), testsuite_name_(mcopystr("Titan")), out_filename_(NULL), file_stream_(NULL)
{
	major_version_ = 1;
	minor_version_ = 0;
	name_ = mcopystr("SummaryLogger");
	help_ = mcopystr("SummaryLogger writes a brief summary, keeping track of expected-to-fail items");
}

SummaryLogger::~SummaryLogger()
{
	close_file();

	Free(name_);
	Free(help_);
	Free(out_filename_);
	Free(testsuite_name_);
	Free(filename_stem_);
	name_ = help_ = out_filename_ = filename_stem_ = NULL;
	file_stream_ = NULL;
}

void SummaryLogger::init(const char *)
{
	fprintf(stderr, "Initializing `%s' (v%u.%u): %s\n", name_, major_version_, minor_version_, help_);
}

void SummaryLogger::fini()
{
}

void SummaryLogger::set_parameter(const char *parameter_name, const char *parameter_value)
{
	if (!strcmp("filename_stem", parameter_name)) {
		if (filename_stem_ != NULL)
			Free(filename_stem_);
		filename_stem_ = mcopystr(parameter_value);
	} else if (!strcmp("testsuite_name", parameter_name)) {
		if (filename_stem_ != NULL)
			Free(testsuite_name_);
		testsuite_name_ = mcopystr(parameter_value);
	} else {
		fprintf(stderr, "Unsupported parameter: `%s' with value: `%s'\n",
			parameter_name, parameter_value);
	}
}

void SummaryLogger::open_file(bool is_first) {
	if (is_first) {
		if (filename_stem_ == NULL) {
			filename_stem_ = mcopystr("summary");
		}
	}

	if (file_stream_ != NULL)
		return;

	if (!TTCN_Runtime::is_single() && !TTCN_Runtime::is_mtc())
		return; // don't bother, only MTC has testcases

	out_filename_ = mprintf("%s-%lu.log", filename_stem_, (unsigned long)getpid());

	file_stream_ = fopen(out_filename_, "w");
	if (!file_stream_) {
		fprintf(stderr, "%s was unable to open log file `%s', reinitialization "
			"may help\n", plugin_name(), out_filename_);
		return;
	}

	is_configured_ = true;
	testsuite.ts_name = testsuite_name_;
}

void SummaryLogger::close_file() {
	if (file_stream_ != NULL) {
		testsuite.write(file_stream_);
		fclose(file_stream_);
		file_stream_ = NULL;
	}
	if (out_filename_) {
		Free(out_filename_);
		out_filename_ = NULL;
	}
}

void SummaryLogger::log(const TitanLoggerApi::TitanLogEvent& event,
				bool /*log_buffered*/, bool /*separate_file*/,
				bool /*use_emergency_mask*/)
{
	if (file_stream_ == NULL) return;

	const TitanLoggerApi::LogEventType_choice& choice = event.logEvent().choice();

	switch (choice.get_selection()) {
	case TitanLoggerApi::LogEventType_choice::ALT_testcaseOp:
		{
			const TitanLoggerApi::TestcaseEvent_choice& tcev = choice.testcaseOp().choice();

			switch (tcev.get_selection()) {
			case TitanLoggerApi::TestcaseEvent_choice::ALT_testcaseStarted:
				testcase.tc_name = tcev.testcaseStarted().testcase__name();
				// remember the start time
				testcase.verdict = TestCase::Unbound;
				break;

			case TitanLoggerApi::TestcaseEvent_choice::ALT_testcaseFinished:
				{
					const TitanLoggerApi::TestcaseType& tct = tcev.testcaseFinished();
					testcase.module_name = tct.name().module__name();

					testcase.setTCVerdict(event);
					testsuite.addTestCase(testcase);
					testcase.reset();
					break;
				}

			case TitanLoggerApi::TestcaseEvent_choice::UNBOUND_VALUE:
				testcase.verdict = TestCase::Unbound;
				break;
			}

			break;
		}

	default:
		break;
	}

	fflush(file_stream_);
}

void TestCase::writeTestCase(FILE* file_stream_) const{

	if (!expected_verdict || expected_verdict == verdict) {
		const char *verdict_str = verdict;
		if (expected_verdict == Fail)
			verdict_str = XFail;
		/* We don't know what verdict to expect. Just write out results. */
		fprintf(file_stream_, "%-5s", verdict_str);
	} else {
		/* There is an expected verdict read in from an expected-verdicts.txt.
		 * Clearly indicate verdict changes. */
		const char *expected_verdict_str = expected_verdict;
		if (expected_verdict == Fail)
			expected_verdict_str = XFail;

		fprintf(file_stream_, "%s->%s", expected_verdict_str, verdict);
	}
	fprintf(file_stream_, " %s.%s\n", module_name.data(), tc_name.data());
	fflush(file_stream_);
}

void TestSuite::addTestCase(const TestCase& testcase) {
	testcases.push_back(new TestCase(testcase));
	all++;

	if (testcase.verdict == TestCase::Fail) failed++;
	else if (testcase.verdict == TestCase::Skipped) skipped++;
	else if (testcase.verdict == TestCase::Error) error++;
	else if (testcase.verdict == TestCase::Inconc) inconc++;
}

void TestSuite::write(FILE* file_stream_) {
	readExpectedVerdicts();
	for (TestCases::const_iterator it = testcases.begin(); it != testcases.end(); ++it) {
		(*it)->writeTestCase(file_stream_);
	}
	fflush(file_stream_);
}

void TestCase::setTCVerdict(const TitanLoggerApi::TitanLogEvent& event){
	TitanLoggerApi::Verdict tc_verdict = event.logEvent().choice().testcaseOp().choice().testcaseFinished().verdict();
	switch (tc_verdict) {
	case TitanLoggerApi::Verdict::UNBOUND_VALUE:
	case TitanLoggerApi::Verdict::UNKNOWN_VALUE:
		verdict = TestCase::Unbound;
		break;

	case TitanLoggerApi::Verdict::v0none:
		verdict = TestCase::Skipped;
		break;

	case TitanLoggerApi::Verdict::v1pass:
		verdict = TestCase::Pass;
		break;

	case TitanLoggerApi::Verdict::v2inconc:
		verdict = TestCase::Inconc;
		break;

	case TitanLoggerApi::Verdict::v3fail:
		verdict = TestCase::Fail;
		break;

	case TitanLoggerApi::Verdict::v4error:
		verdict = TestCase::Error;
		break;
	}
}

void TestSuite::readExpectedVerdicts(const char *path)
{
	char *line = NULL;
	char *saveptr = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *f;
	if (!path)
		path = "expected-verdicts.txt";
	f = fopen(path, "r");
	if (!f)
		return;

	while ((read = getline(&line, &len, f)) != -1) {
		char *tokens[3] = {};
		int i;

		/* Expecting a line like written above:
		 *   pass MSC_Tests.TC_foo
		 *   [0]  [1]       [2]
		 */
		for (i = 0; i < 3; i++) {
			tokens[i] = strtok_r(i ? NULL : line, " .\n", &saveptr);
			if (!tokens[i]) {
				i = -1;
				break;
			}
		}

		/* This line didn't have enough tokens. */
		if (i != 3)
			continue;

		evaluateExpectedVerdict(tokens[1], tokens[2], tokens[0]);
	}

	fclose(f);

}

void TestSuite::evaluateExpectedVerdict(const char *suite_name, const char *test_name,
					const char *verdict_name)
{
	for (TestCases::const_iterator it = testcases.begin(); it != testcases.end(); ++it) {
		if ((*it)->evaluateExpectedVerdict(suite_name, test_name, verdict_name))
			return;
	}
}

bool TestCase::evaluateExpectedVerdict(const char *suite_name, const char *test_name,
				       const char *verdict_name)
{
	if (strcmp(suite_name, this->module_name.data()))
		return false;
	if (strcmp(test_name, this->tc_name.data()))
		return false;

	if (!strcmp(verdict_name, Pass))
		this->expected_verdict = Pass;
	else if (!strcmp(verdict_name, Inconc))
		this->expected_verdict = Inconc;
	else if (!strcmp(verdict_name, Fail)
		 || !strcmp(verdict_name, XFail))
		this->expected_verdict = Fail;
	else if (!strcmp(verdict_name, Error))
		this->expected_verdict = Error;
	else if (!strcmp(verdict_name, Unbound))
		this->expected_verdict = Unbound;
	else if (!strcmp(verdict_name, Skipped))
		this->expected_verdict = Skipped;
	else
		return false;
	return true;
}
