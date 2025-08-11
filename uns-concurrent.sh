#!/bin/bash
# Concurrent test runner for SM-DP+ and ES2+ test suites using separate network namespaces

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
: "${PYSRVPATH:=$(realpath ${SCRIPT_DIR}/../pysim)}"
TESTP=$(realpath ${SCRIPT_DIR}/smdpp)

if [ ! -f ${PYSRVPATH}/osmo-smdpp.py ]; then
    echo "fix your PYSRVPATH!"
    exit 1
fi

if [ -f /app/env.sh ]; then
    . /app/env.sh
fi

CONCURRENCY=4  # Default concurrent tests
RUN_ALL=0
TEST_PATTERN=""
TEST_LIST_ARG=""
CONCISE_MODE=0
TEST_SUITE="both"  # Default to running both suites

while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs)
            CONCURRENCY="$2"
            shift 2
            ;;
        -a|--all)
            RUN_ALL=1
            shift
            ;;
        -p|--pattern)
            TEST_PATTERN="$2"
            shift 2
            ;;
        -c|--concise)
            CONCISE_MODE=1
            shift
            ;;
        -s|--suite)
            TEST_SUITE="$2"
            if [[ ! "$TEST_SUITE" =~ ^(smdpp|es2plus|both)$ ]]; then
                echo "Error: Invalid suite '$TEST_SUITE'. Must be 'smdpp', 'es2plus', or 'both'"
                exit 1
            fi
            shift 2
            ;;
        -t|--tests)
            # Allow space-separated list of specific tests
            shift
            # All remaining arguments are test names
            TEST_LIST_ARG="$@"
            break  # Stop processing args
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "  -j, --jobs N      Run N tests concurrently (default: 4)"
            echo "  -a, --all         Run all tests"
            echo "  -p, --pattern PAT Run tests matching pattern"
            echo "  -c, --concise     Only show failed/unknown tests (hide passing)"
            echo "  -s, --suite SUITE Select test suite: smdpp, es2plus, or both (default: both)"
            echo "  -t, --tests       Run specific tests (space-separated list)"
            echo "  -h, --help        Show this help"
            echo ""
            echo "Examples:"
            echo "  $0 -a -j 8                                      # Run all tests from both suites with 8 concurrent jobs"
            echo "  $0 -a -j 8 -s smdpp                            # Run all smdpp tests with 8 concurrent jobs"
            echo "  $0 -a -j 8 -s es2plus                          # Run all ES2+ tests with 8 concurrent jobs"
            echo "  $0 -p AuthenticateClient -j 4                  # Run all matching tests with concurrency 4"
            echo "  $0 -j 2 -t TEST1 TEST2                          # Run specific tests with concurrency 2"
            echo "  $0 -a -c                                        # Run all tests, only show failures"
            echo ""
            echo "Notes:"
            echo "  - Using -p automatically runs matching tests (no need for -a)"
            echo "  - Options like -j must come before -t"
            echo "  - Test names for -t should include the suite prefix (e.g., smdpp_Tests.TC_...)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

cd ${SCRIPT_DIR}

# Function to get test list from an executable
get_test_list() {
    local exe_name=$1
    local exe_path="${TESTP}/${exe_name}"
    
    if [ ! -x "$exe_path" ]; then
        return
    fi
    
    # Get test list and filter to only tests from this module
    # The executable already outputs fully qualified names like "ES2Plus_Tests.TC_..."
    cd ${TESTP} && LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PWD:$TITAN_LIBRARY_PATH" ./${exe_name} -l 2>/dev/null | grep "^${exe_name}\." | grep -v ".control$"
}

# Get list of tests to run
if [ -n "$TEST_LIST_ARG" ]; then
    TEST_LIST="$TEST_LIST_ARG"
else
    TEST_LIST=""
    
    # Discover tests from selected suites
    if [ "$TEST_SUITE" = "smdpp" ] || [ "$TEST_SUITE" = "both" ]; then
        SMDPP_TESTS=$(get_test_list "smdpp_Tests")
        TEST_LIST="$TEST_LIST $SMDPP_TESTS"
    fi
    
    if [ "$TEST_SUITE" = "es2plus" ] || [ "$TEST_SUITE" = "both" ]; then
        ES2PLUS_TESTS=$(get_test_list "ES2Plus_Tests")
        TEST_LIST="$TEST_LIST $ES2PLUS_TESTS"
    fi
    
    # Trim leading/trailing spaces
    TEST_LIST=$(echo "$TEST_LIST" | xargs)
    
    if [ -n "$TEST_PATTERN" ]; then
        TEST_LIST=$(echo "$TEST_LIST" | tr ' ' '\n' | grep "$TEST_PATTERN" | tr '\n' ' ')
    fi
fi

if [ "$RUN_ALL" -eq 0 ] && [ -z "$TEST_LIST_ARG" ] && [ -z "$TEST_PATTERN" ]; then
    echo "Available tests:"
    echo "$TEST_LIST" | tr ' ' '\n' | nl
    echo ""
    echo "Use -a to run all tests, -p PATTERN to run matching tests, or -t to specify tests"
    echo "Use -s to select test suite: smdpp, es2plus, or both (default)"
    exit 0
fi

# If we have a pattern but no explicit run flag, assume user wants to run those
if [ -n "$TEST_PATTERN" ] && [ "$RUN_ALL" -eq 0 ] && [ -z "$TEST_LIST_ARG" ]; then
    RUN_ALL=1
fi

TOTAL_TESTS=$(echo "$TEST_LIST" | wc -w)
echo "Found $TOTAL_TESTS tests to run from suite(s): $TEST_SUITE with concurrency=$CONCURRENCY"

# Create temporary directory for this run
RUN_ID=$(date +%Y%m%d_%H%M%S)_$$
RUN_DIR="${TESTP}/concurrent_run_${RUN_ID}"
mkdir -p "$RUN_DIR"

run_test_in_namespace() {
    local full_test_name="$1"
    local test_num="$2"
    local ns_name="test_${test_num}_$$"
    local test_log_dir="${RUN_DIR}/test_${test_num}"

    # Parse suite and test name
    local suite_name="${full_test_name%%.*}"
    local test_name="${full_test_name#*.}"
    local config_file="${suite_name}.cfg"

    mkdir -p "$test_log_dir"

    # Create status file
    echo "RUNNING" > "${test_log_dir}/status"
    echo "$full_test_name" > "${test_log_dir}/test_name"
    echo "$suite_name" > "${test_log_dir}/suite_name"

    # In concise mode, don't show starting message for tests
    if [ "$CONCISE_MODE" -eq 0 ]; then
        echo "[Test $test_num] Starting: $full_test_name"
    fi

    local QLIST="DEBUG_ENCDEC DEBUG_UNQUALIFIED PORTEVENT_MMRECV PORTEVENT_MMSEND PORTEVENT_MQUEUE PARALLEL_PTC PORTEVENT_STATE EXECUTOR_ WARNING_UNQUALIFIED PORTEVENT_UNQUALIFIED PARALLEL_UNQUALIFIED"
    local QVARS=$(for file in $QLIST; do printf " -ve %s" "$file"; done)

    local CMDSTR="export TTCN3_DIR=${TTCN3_DIR}; export TITAN_LIBRARY_PATH=${TITAN_LIBRARY_PATH}; export TTCN3_BIN_DIR=${TTCN3_BIN_DIR}; export PATH=\$PATH;"

    # Mount a new tmpfs for this namespace to ensure isolation
    CMDSTR+=" mount -t tmpfs tmpfs /tmp;"

    # Add random startup delay to prevent race conditions
    CMDSTR+=" sleep 0.\$((RANDOM % 500 + 100));"  # 0.1-0.6 second random delay

    CMDSTR+=" cd ${TESTP}; ip l s up lo;"

    CMDSTR+=" rm -f ${suite_name}*.log *.stderr 2>/dev/null;"

    CMDSTR+=" ( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -t -H 127.0.0.1 -p 8000 --in-memory 2>&1 > ${test_log_dir}/pyserver_nist.log & echo \$! > ${test_log_dir}/nist_pid ) ;"

    CMDSTR+=" ( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -t -H 127.0.0.1 -p 8001 --brainpool --in-memory 2>&1 > ${test_log_dir}/pyserver_brp.log & echo \$! > ${test_log_dir}/brp_pid ) ;"

    CMDSTR+=" sleep 2;"

    # Set up cleanup trap
    CMDSTR+=" trap 'kill \$(cat ${test_log_dir}/nist_pid 2>/dev/null) 2>/dev/null; kill \$(cat ${test_log_dir}/brp_pid 2>/dev/null) 2>/dev/null' EXIT;"

    CMDSTR+=" ../start-testsuite.sh ${suite_name} ${config_file} ${test_name} > ${test_log_dir}/test_output.log 2>&1;"
    CMDSTR+=" TEST_RESULT=\$?;"

    CMDSTR+=" kill \$(cat ${test_log_dir}/nist_pid 2>/dev/null) 2>/dev/null;"
    CMDSTR+=" kill \$(cat ${test_log_dir}/brp_pid 2>/dev/null) 2>/dev/null;"

    CMDSTR+=" ttcn3_logmerge ${suite_name}*.log 2>/dev/null | grep "${QVARS}" > ${test_log_dir}/_merged.log;"
    CMDSTR+=" ttcn3_logformat ${test_log_dir}/_merged.log > ${test_log_dir}/merged.log 2>/dev/null;"

    CMDSTR+=" rm -f ${suite_name}*.log *.stderr ${test_log_dir}/_*.log ${test_log_dir}/*_pid;"

    CMDSTR+=" grep -v 'DEBUG:pySim.esim.saip' ${test_log_dir}/pyserver_nist.log > ${test_log_dir}/pyserver_nist_clean.log 2>/dev/null || true;"
    CMDSTR+=" grep -v 'DEBUG:pySim.esim.saip' ${test_log_dir}/pyserver_brp.log > ${test_log_dir}/pyserver_brp_clean.log 2>/dev/null || true;"
    CMDSTR+=" mv ${test_log_dir}/pyserver_nist_clean.log ${test_log_dir}/pyserver_nist.log 2>/dev/null || true;"
    CMDSTR+=" mv ${test_log_dir}/pyserver_brp_clean.log ${test_log_dir}/pyserver_brp.log 2>/dev/null || true;"

    # Write status file from within namespace
    CMDSTR+=" STATUS_FILE=\$(mktemp);"
    CMDSTR+=" if grep -q 'Test case.*finished.*Verdict: pass' ${test_log_dir}/test_output.log 2>/dev/null; then echo 'PASSED' > \$STATUS_FILE;"
    CMDSTR+=" elif grep -q 'Test case.*finished.*Verdict: fail' ${test_log_dir}/test_output.log 2>/dev/null; then echo 'FAILED' > \$STATUS_FILE;"
    CMDSTR+=" elif grep -q 'Test case.*finished.*Verdict: inconc' ${test_log_dir}/test_output.log 2>/dev/null; then echo 'INCONC' > \$STATUS_FILE;"
    CMDSTR+=" elif grep -q 'Test case.*finished.*Verdict: error' ${test_log_dir}/test_output.log 2>/dev/null; then echo 'ERROR' > \$STATUS_FILE;"
    CMDSTR+=" elif [ \$TEST_RESULT -eq 0 ]; then echo 'PASSED' > \$STATUS_FILE;"
    CMDSTR+=" else echo 'FAILED' > \$STATUS_FILE; fi;"
    CMDSTR+=" cat \$STATUS_FILE > ${test_log_dir}/status 2>/dev/null || echo 'Could not write status file';"
    CMDSTR+=" rm -f \$STATUS_FILE;"

    unshare -UpmniCTfr -- sh -c "${CMDSTR}" 2>&1 | tee "${test_log_dir}/namespace.log" > /dev/null

    local status=$(cat "${test_log_dir}/status" 2>/dev/null || echo "UNKNOWN")

    # In concise mode, only show non-passing tests
    if [ "$CONCISE_MODE" -eq 1 ] && [ "$status" = "PASSED" ]; then
        # Silent for passing tests in concise mode
        :
    else
        echo "[Test $test_num] Completed: $full_test_name - $status"
    fi
}

# Export function and variables needed for running in subshells
export -f run_test_in_namespace
export CONCISE_MODE SCRIPT_DIR PYSRVPATH TESTP RUN_DIR

echo "Starting parallel test execution..."
if [ "$CONCISE_MODE" -eq 1 ]; then
    echo "(Concise mode: only showing failed/unknown tests)"
fi
echo ""

# Track jobs
declare -a job_pids
declare -a job_tests
declare -a job_nums

test_num=0
active_jobs=0

wait_for_job_slot() {
    while [ $active_jobs -ge $CONCURRENCY ]; do
        for i in "${!job_pids[@]}"; do
            if [ -n "${job_pids[$i]}" ]; then
                if ! kill -0 "${job_pids[$i]}" 2>/dev/null; then
                    # completed
                    wait "${job_pids[$i]}"
                    unset job_pids[$i]
                    ((active_jobs--))
                fi
            fi
        done
        sleep 0.1
    done
}

for test_name in $TEST_LIST; do
    wait_for_job_slot

    ((test_num++))
    run_test_in_namespace "$test_name" "$test_num" &

    job_pids+=($!)
    job_tests+=("$test_name")
    job_nums+=($test_num)
    ((active_jobs++))
done

echo "Waiting for all tests to complete..."
for pid in "${job_pids[@]}"; do
    if [ -n "$pid" ]; then
        wait "$pid" 2>/dev/null
    fi
done

echo "All tests completed. Generating summary..."

SUMMARY_FILE="${RUN_DIR}/summary.txt"
{
    echo "Test Run Summary - $RUN_ID"
    echo "================================"
    echo "Total tests: $TOTAL_TESTS"
    echo "Test suite(s): $TEST_SUITE"
    echo "Concurrency: $CONCURRENCY"
    if [ "$CONCISE_MODE" -eq 1 ]; then
        echo "Mode: Concise (showing only failed/unknown tests)"
    fi
    echo ""
    echo "Results:"
    echo "--------"
} > "$SUMMARY_FILE"

PASSED=0
FAILED=0
UNKNOWN=0
ERROR=0
INCONC=0

# Count by suite
SMDPP_PASSED=0
SMDPP_FAILED=0
ES2PLUS_PASSED=0
ES2PLUS_FAILED=0

for test_dir in "${RUN_DIR}"/test_*; do
    if [ -d "$test_dir" ]; then
        test_num=$(basename "$test_dir" | cut -d_ -f2)
        test_name=$(cat "$test_dir/test_name" 2>/dev/null || echo "UNKNOWN")
        suite_name=$(cat "$test_dir/suite_name" 2>/dev/null || echo "UNKNOWN")
        status=$(cat "$test_dir/status" 2>/dev/null || echo "UNKNOWN")

        # In concise mode, only add non-passing tests to summary
        if [ "$CONCISE_MODE" -eq 0 ] || [ "$status" != "PASSED" ]; then
            printf "%-80s %s\n" "$test_name" "$status" >> "$SUMMARY_FILE"
        fi

        case "$status" in
            PASSED) 
                ((PASSED++))
                if [ "$suite_name" = "smdpp_Tests" ]; then
                    ((SMDPP_PASSED++))
                elif [ "$suite_name" = "ES2Plus_Tests" ]; then
                    ((ES2PLUS_PASSED++))
                fi
                ;;
            FAILED) 
                ((FAILED++))
                if [ "$suite_name" = "smdpp_Tests" ]; then
                    ((SMDPP_FAILED++))
                elif [ "$suite_name" = "ES2Plus_Tests" ]; then
                    ((ES2PLUS_FAILED++))
                fi
                ;;
            INCONC) ((INCONC++)) ;;
            ERROR) ((ERROR++)) ;;
            *) ((UNKNOWN++)) ;;
        esac
    fi
done

{
    echo ""
    echo "Summary:"
    echo "  Passed:       $PASSED"
    echo "  Failed:       $FAILED"
    echo "  Error:        $ERROR"
    echo "  Inconclusive: $INCONC"
    echo "  Unknown:      $UNKNOWN"
    
    if [ "$TEST_SUITE" = "both" ]; then
        echo ""
        echo "By Suite:"
        echo "  smdpp_Tests:   $SMDPP_PASSED passed, $SMDPP_FAILED failed"
        echo "  ES2Plus_Tests: $ES2PLUS_PASSED passed, $ES2PLUS_FAILED failed"
    fi
    
    echo ""
    echo "Detailed logs available in: $RUN_DIR"
} >> "$SUMMARY_FILE"

cat "$SUMMARY_FILE"

# Convenience symlink
ln -sfn "$RUN_DIR" "${TESTP}/latest_concurrent_run"

echo ""
echo "To view logs for a specific test:"
echo "  ls -la $RUN_DIR/test_*/"
echo ""
echo "To view a test's merged log (replace 1 with test number):"
echo "  less $RUN_DIR/test_1/merged.log"
echo ""
echo "To view a test's Python server logs (replace 1 with test number):"
echo "  less $RUN_DIR/test_1/pyserver_nist.log"
echo "  less $RUN_DIR/test_1/pyserver_brp.log"
echo ""
echo "Or use the latest symlink:"
echo "  less smdpp/latest_concurrent_run/test_1/merged.log"