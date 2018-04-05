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
#ifndef SummaryLogger_HH2
#define SummaryLogger_HH2

namespace TitanLoggerApi { class TitanLogEvent; }

#ifndef TITAN_RUNTIME_2
#include "RT1/TitanLoggerApi.hh"
#else
#include "RT2/TitanLoggerApi.hh"
#endif

#include "ILoggerPlugin.hh"
#include <stdio.h>
#include <string>
#include <vector>

struct TestCase {
	static constexpr const char* Pass = "pass";
	static constexpr const char* Inconc = "INCONCLUSIVE";
	static constexpr const char* Fail = "FAIL";
	static constexpr const char* XFail = "xfail";
	static constexpr const char* Error = "ERROR";
	static constexpr const char* Unbound = "UNBOUND";
	static constexpr const char* Skipped = "skipped";

	const char *expected_verdict;
	const char *verdict;
	std::string tc_name;
	std::string module_name;
	
	TestCase();
	
	void writeTestCase(FILE* file_stream_) const;
	void setTCVerdict(const TitanLoggerApi::TitanLogEvent& event);
	void reset();
	bool evaluateExpectedVerdict(const char *suite_name, const char *test_name,
				     const char *verdict_name);
};


struct TestSuite {
	typedef std::vector<TestCase*> TestCases;
	
	std::string ts_name;
	int all;
	int skipped;
	int failed;
	int error;
	int inconc;
	TestCases testcases;

	TestSuite():ts_name(""), all(0), skipped(0), failed(0), error(0), inconc(0) {}
	~TestSuite();
	
	void addTestCase(const TestCase& element);
	void write(FILE* file_stream_);
	void readExpectedVerdicts(const char *path=NULL);
	void evaluateExpectedVerdict(const char *suite_name, const char *test_name,
				     const char *verdict_name);
};

class SummaryLogger: public ILoggerPlugin
{
	public:
		SummaryLogger();
		virtual ~SummaryLogger();
		inline bool is_static() { return false; }
		void init(const char *options = 0);
		void fini();

		void log(const TitanLoggerApi::TitanLogEvent& event, bool log_buffered,
			 bool separate_file, bool use_emergency_mask);
		void set_parameter(const char *parameter_name, const char *parameter_value);

		virtual void open_file(bool is_first);
		virtual void close_file();

	private:
		// parameters
		char *filename_stem_;
		char *testsuite_name_;
		// working values
		char *out_filename_;
		char *expect_filename_;
		TestSuite testsuite;
		TestCase testcase;

		FILE *file_stream_;
};

#endif  // SummaryLogger_HH2
