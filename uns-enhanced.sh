#!/bin/bash

# Enhanced uns.sh with parallel test execution support
# Usage:
#   ./uns-enhanced.sh                        # Run all tests sequentially  
#   ./uns-enhanced.sh <TEST_NAME>            # Run single test
#   ./uns-enhanced.sh --list                 # List all available tests
#   ./uns-enhanced.sh --parallel <N>         # Run all tests with N parallel workers
#   ./uns-enhanced.sh --parallel <N> <GREP>  # Run tests matching pattern with N workers

set -e

WHCIHT="smdpp"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PYSRVPATH=$(realpath ${SCRIPT_DIR}/../pysim)
TESTP=$(realpath ${SCRIPT_DIR}/${WHCIHT})

# Parse command line arguments
PARALLEL_MODE=0
MAX_WORKERS=1
TEST_FILTER=""
LIST_TESTS=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --parallel|-p)
            PARALLEL_MODE=1
            MAX_WORKERS="$2"
            shift 2
            ;;
        --list|-l)
            LIST_TESTS=1
            shift
            ;;
        *)
            TEST_FILTER="$1"
            shift
            ;;
    esac
done

# Function to get all test names using TTCN-3 binary
get_test_list() {
    cd ${TESTP}
    export TTCN3_DIR=/app/titan
    export TITAN_LIBRARY_PATH=/app/titan/lib
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:.:$TITAN_LIBRARY_PATH"
    
    # Get test list from the compiled test suite
    # Filter out control and non-test entries
    ./${WHCIHT}_Tests -l 2>/dev/null | grep "^${WHCIHT}_Tests\.TC_" | sed "s/^${WHCIHT}_Tests\.//" | sort
}

# List tests if requested
if [ $LIST_TESTS -eq 1 ]; then
    echo "Available tests in ${WHCIHT}_Tests:"
    get_test_list | nl
    exit 0
fi

# Function to run tests in worker mode (called by xargs)
run_test_worker() {
    local test_name="$1"
    local worker_id="${2:-1}"
    local timestamp=$(date +%s%N)
    local test_id="${test_name}_${worker_id}_${timestamp}"
    
    echo "[Worker $worker_id] Starting: $test_name"
    
    # Create test-specific directory
    local test_dir="${TESTP}/parallel_run/${test_id}"
    mkdir -p "$test_dir"
    cd "$test_dir"
    
    # Set up environment
    export TTCN3_DIR=/app/titan
    export TITAN_LIBRARY_PATH=/app/titan/lib
    export TTCN3_BIN_DIR=/app/titan/bin
    export PATH=/app/titan/bin:$PATH
    
    # Run test in isolated namespace
    local CMDSTR="ip l s up lo;"
    CMDSTR+=" cd ${TESTP};"
    CMDSTR+=" ../start-testsuite.sh ${WHCIHT}_Tests ${WHCIHT}_Tests.cfg ${test_name} 2>&1;"
    
    unshare -UpmniCTfr -w ${test_id} -- sh -c "${CMDSTR}" > "${test_dir}/output.log" 2>&1
    local result=$?
    
    # Process test logs
    cd ${TESTP}
    if [ -f "${WHCIHT}_Tests.log" ]; then
        mv "${WHCIHT}_Tests.log" "${test_dir}/"
        mv ${WHCIHT}_Tests_*.log "${test_dir}/" 2>/dev/null || true
    fi
    
    # Create summary
    if [ $result -eq 0 ]; then
        echo "[Worker $worker_id] PASSED: $test_name"
        echo "PASSED" > "${test_dir}/result.txt"
    else
        echo "[Worker $worker_id] FAILED: $test_name (exit code: $result)"
        echo "FAILED:$result" > "${test_dir}/result.txt"
    fi
    
    return $result
}

# Export function for xargs
export -f run_test_worker
export WHCIHT SCRIPT_DIR PYSRVPATH TESTP

# Main execution
cd ${SCRIPT_DIR}

# Clean up old parallel run directory
rm -rf ${TESTP}/parallel_run
mkdir -p ${TESTP}/parallel_run

# Start Python servers
echo "Starting SM-DP+ servers: NIST on port 8000, BRP on port 8001"
( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8000 > ${TESTP}/pyserver_nist.log 2>&1 & echo $! > ${TESTP}/.nist_pid )
( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8001 --brainpool > ${TESTP}/pyserver_brp.log 2>&1 & echo $! > ${TESTP}/.brp_pid )

# Set up cleanup
cleanup() {
    echo "Cleaning up..."
    [ -f ${TESTP}/.nist_pid ] && kill $(cat ${TESTP}/.nist_pid) 2>/dev/null || true
    [ -f ${TESTP}/.brp_pid ] && kill $(cat ${TESTP}/.brp_pid) 2>/dev/null || true
    rm -f ${TESTP}/.nist_pid ${TESTP}/.brp_pid
    
    # Process Python server logs
    grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/pyserver_nist.log > ${TESTP}/pyserver_nist_clean.log 2>/dev/null || true
    grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/pyserver_brp.log > ${TESTP}/pyserver_brp_clean.log 2>/dev/null || true
    
    # Create combined pyserver.log
    echo '=== NIST Server Log (port 8000) ===' > ${TESTP}/pyserver.log
    cat ${TESTP}/pyserver_nist_clean.log >> ${TESTP}/pyserver.log 2>/dev/null || true
    echo '' >> ${TESTP}/pyserver.log
    echo '=== BRP Server Log (port 8001) ===' >> ${TESTP}/pyserver.log
    cat ${TESTP}/pyserver_brp_clean.log >> ${TESTP}/pyserver.log 2>/dev/null || true
    
    rm -f ${TESTP}/pyserver_*_clean.log
}
trap cleanup EXIT

# Wait for servers to start
sleep 2

# Get test list
if [ -n "$TEST_FILTER" ] && [ $PARALLEL_MODE -eq 0 ]; then
    # Single test mode
    TEST_LIST="$TEST_FILTER"
else
    # Get all tests, optionally filtered
    TEST_LIST=$(get_test_list)
    if [ -n "$TEST_FILTER" ]; then
        TEST_LIST=$(echo "$TEST_LIST" | grep "$TEST_FILTER" || true)
    fi
fi

# Count tests
NUM_TESTS=$(echo "$TEST_LIST" | grep -c . || echo 0)

if [ $NUM_TESTS -eq 0 ]; then
    echo "No tests found matching filter: $TEST_FILTER"
    exit 1
fi

echo "Running $NUM_TESTS tests with $MAX_WORKERS workers"
echo "============================================"

# Run tests
if [ $PARALLEL_MODE -eq 1 ] && [ $NUM_TESTS -gt 1 ]; then
    # Parallel execution using xargs
    echo "$TEST_LIST" | nl | xargs -P "$MAX_WORKERS" -n 2 bash -c 'run_test_worker "$2" "$1"'
else
    # Sequential execution
    worker_id=1
    for test in $TEST_LIST; do
        run_test_worker "$test" "$worker_id"
        ((worker_id++)) || true
    done
fi

# Merge logs
echo ""
echo "Merging test logs..."

# First merge all TTCN3 logs
cd ${TESTP}
find parallel_run -name "${WHCIHT}_Tests*.log" -type f | while read log; do
    echo "Processing: $log"
    cat "$log" >> all_tests_raw.log
done

# Apply ttcn3_logmerge if logs exist
if [ -f all_tests_raw.log ]; then
    ttcn3_logmerge all_tests_raw.log > merged_raw.log 2>/dev/null || cp all_tests_raw.log merged_raw.log
    
    # Filter out noise
    QLIST="DEBUG_ENCDEC DEBUG_UNQUALIFIED PORTEVENT_MMRECV PORTEVENT_MMSEND PORTEVENT_MQUEUE PARALLEL_PTC PORTEVENT_STATE EXECUTOR_ WARNING_UNQUALIFIED PORTEVENT_UNQUALIFIED PARALLEL_UNQUALIFIED"
    QVARS=$(for item in $QLIST; do printf " -ve %s" "$item"; done)
    grep $QVARS merged_raw.log > merged_filtered.log 2>/dev/null || cp merged_raw.log merged_filtered.log
    
    # Format final log
    ttcn3_logformat merged_filtered.log > ${TESTP}/merged.log 2>/dev/null || cp merged_filtered.log ${TESTP}/merged.log
    
    # Clean up temp files
    rm -f all_tests_raw.log merged_raw.log merged_filtered.log
fi

# Generate summary report
echo ""
echo "Test Execution Summary"
echo "======================"
PASSED=0
FAILED=0
FAILED_TESTS=""

for result_file in ${TESTP}/parallel_run/*/result.txt; do
    if [ -f "$result_file" ]; then
        test_dir=$(dirname "$result_file")
        test_name=$(basename "$test_dir" | sed 's/_[0-9]*_[0-9]*$//')
        result=$(cat "$result_file")
        
        if [[ "$result" == "PASSED" ]]; then
            ((PASSED++)) || true
        else
            ((FAILED++)) || true
            FAILED_TESTS="${FAILED_TESTS}  - ${test_name}: ${result}\n"
        fi
    fi
done

echo "Total tests: $NUM_TESTS"
echo "Passed: $PASSED"
echo "Failed: $FAILED"

if [ $FAILED -gt 0 ]; then
    echo ""
    echo "Failed tests:"
    echo -e "$FAILED_TESTS"
    exit_code=1
else
    echo ""
    echo "All tests passed!"
    exit_code=0
fi

# Keep parallel_run directory for debugging if tests failed
if [ $FAILED -eq 0 ]; then
    echo ""
    echo "Cleaning up test artifacts..."
    rm -rf ${TESTP}/parallel_run
fi

exit $exit_code