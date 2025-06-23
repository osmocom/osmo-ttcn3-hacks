#!/bin/bash

# Extended uns.sh with parallel test execution support
# Usage:
#   ./uns-parallel.sh                    # Run all tests sequentially
#   ./uns-parallel.sh <TEST_NAME>        # Run single test
#   ./uns-parallel.sh --parallel <N>     # Run all tests with N workers
#   ./uns-parallel.sh --parallel <N> <PATTERN>  # Run matching tests with N workers

WHCIHT="smdpp"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PYSRVPATH=$(realpath ${SCRIPT_DIR}/../pysim)
TESTP=$(realpath ${SCRIPT_DIR}/${WHCIHT})
PARALLEL_LOGS_DIR="${TESTP}/parallel_logs"
SERVER_LOCK="${TESTP}/.server_lock"
MAX_WORKERS=1
PARALLEL_MODE=0
TEST_PATTERN=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --parallel|-p)
            PARALLEL_MODE=1
            MAX_WORKERS="$2"
            shift 2
            ;;
        *)
            if [ $PARALLEL_MODE -eq 1 ]; then
                TEST_PATTERN="$1"
            else
                # Single test mode
                TEST_PATTERN="$1"
            fi
            shift
            ;;
    esac
done

# Function to start Python servers
start_servers() {
    if [ -f "$SERVER_LOCK" ]; then
        echo "Servers already running (lock file exists)"
        return 1
    fi
    
    echo "Starting SM-DP+ servers: NIST on port 8000, BRP on port 8001"
    
    # Start NIST server
    ( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8000 2>&1 > ${TESTP}/pyserver_nist_main.log & echo $! > ${TESTP}/.nist_pid )
    
    # Start BRP server  
    ( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8001 --brainpool 2>&1 > ${TESTP}/pyserver_brp_main.log & echo $! > ${TESTP}/.brp_pid )
    
    # Create lock file
    touch "$SERVER_LOCK"
    
    # Wait for servers to start
    sleep 2
    
    # Verify servers are running
    if ! ps -p $(cat ${TESTP}/.nist_pid 2>/dev/null) > /dev/null 2>&1; then
        echo "Failed to start NIST server"
        cleanup_servers
        return 1
    fi
    
    if ! ps -p $(cat ${TESTP}/.brp_pid 2>/dev/null) > /dev/null 2>&1; then
        echo "Failed to start BRP server"
        cleanup_servers
        return 1
    fi
    
    echo "Servers started successfully"
    return 0
}

# Function to stop Python servers
cleanup_servers() {
    echo "Cleaning up servers..."
    
    # Kill servers
    if [ -f "${TESTP}/.nist_pid" ]; then
        kill $(cat ${TESTP}/.nist_pid 2>/dev/null) 2>/dev/null || true
        rm -f ${TESTP}/.nist_pid
    fi
    
    if [ -f "${TESTP}/.brp_pid" ]; then
        kill $(cat ${TESTP}/.brp_pid 2>/dev/null) 2>/dev/null || true
        rm -f ${TESTP}/.brp_pid
    fi
    
    # Remove lock file
    rm -f "$SERVER_LOCK"
    
    # Process server logs
    if [ -f "${TESTP}/pyserver_nist_main.log" ]; then
        grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/pyserver_nist_main.log > ${TESTP}/pyserver_nist.log 2>/dev/null || true
        rm -f ${TESTP}/pyserver_nist_main.log
    fi
    
    if [ -f "${TESTP}/pyserver_brp_main.log" ]; then
        grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/pyserver_brp_main.log > ${TESTP}/pyserver_brp.log 2>/dev/null || true
        rm -f ${TESTP}/pyserver_brp_main.log
    fi
    
    # Create combined pyserver.log
    echo '=== NIST Server Log (port 8000) ===' > ${TESTP}/pyserver.log
    cat ${TESTP}/pyserver_nist.log >> ${TESTP}/pyserver.log 2>/dev/null || true
    echo '' >> ${TESTP}/pyserver.log
    echo '=== BRP Server Log (port 8001) ===' >> ${TESTP}/pyserver.log
    cat ${TESTP}/pyserver_brp.log >> ${TESTP}/pyserver.log 2>/dev/null || true
}

# Function to run a single test
run_single_test() {
    local test_name="$1"
    local log_prefix="${2:-test}"
    local pid=$$
    
    echo "[$$] Running test: $test_name"
    
    # Create unique log file names
    local test_log="${PARALLEL_LOGS_DIR}/${log_prefix}_${test_name}_${pid}.log"
    local test_merged="${PARALLEL_LOGS_DIR}/${log_prefix}_${test_name}_${pid}_merged.log"
    
    # Prepare environment variables
    local CMDSTR="export TTCN3_DIR=/app/titan; export TITAN_LIBRARY_PATH=/app/titan/lib; export TTCN3_BIN_DIR=/app/titan/bin; export PATH=/app/titan/bin:\$PATH;"
    CMDSTR+=" ip l s up lo;"
    
    # Run the test
    CMDSTR+=" cd ${TESTP};"
    CMDSTR+=" ../start-testsuite.sh ${WHCIHT}_Tests ${WHCIHT}_Tests.cfg ${test_name} 2>&1 | tee ${test_log};"
    
    # Process logs for this test
    CMDSTR+=" ttcn3_logmerge ${WHCIHT}*log | grep -v 'DEBUG_ENCDEC\|DEBUG_UNQUALIFIED\|PORTEVENT_\|PARALLEL_\|EXECUTOR_\|WARNING_UNQUALIFIED' > ${test_merged};"
    CMDSTR+=" rm -f ${WHCIHT}*log;"
    
    # Execute in namespace
    unshare -UpmniCTfr -w ${WHCIHT}_${test_name}_${pid} -- sh -c "${CMDSTR}"
    local result=$?
    
    if [ $result -eq 0 ]; then
        echo "[$$] Test $test_name: PASSED"
    else
        echo "[$$] Test $test_name: FAILED (exit code: $result)"
    fi
    
    return $result
}

# Function to get all test names
get_all_tests() {
    cd ${TESTP}
    # Extract test case names from the TTCN file
    grep "^testcase TC_" ${WHCIHT}_Tests.ttcn | sed 's/testcase \([^ ]*\).*/\1/' | sort
}

# Function to run tests in parallel
run_parallel_tests() {
    local pattern="$1"
    local max_workers="$2"
    
    # Create parallel logs directory
    mkdir -p "$PARALLEL_LOGS_DIR"
    rm -f ${PARALLEL_LOGS_DIR}/*
    
    # Get list of tests
    local tests
    if [ -z "$pattern" ]; then
        tests=$(get_all_tests)
    else
        tests=$(get_all_tests | grep "$pattern")
    fi
    
    local num_tests=$(echo "$tests" | wc -l)
    echo "Found $num_tests tests to run with $max_workers workers"
    
    # Check if GNU parallel is available
    if command -v parallel >/dev/null 2>&1; then
        echo "Using GNU parallel for test execution"
        echo "$tests" | parallel -j "$max_workers" --line-buffer --tagstring "[{#}]" \
            "run_single_test {} parallel_{#}"
        local result=$?
    else
        echo "GNU parallel not found, using bash job control"
        
        # Run tests with bash job control
        local job_count=0
        local failed_tests=""
        
        for test in $tests; do
            # Wait if we've reached max workers
            while [ $(jobs -r | wc -l) -ge $max_workers ]; do
                sleep 0.1
            done
            
            # Start test in background
            run_single_test "$test" "parallel" &
            ((job_count++))
            
            # Show progress
            echo "Started test $job_count/$num_tests: $test"
        done
        
        # Wait for all jobs to complete
        echo "Waiting for all tests to complete..."
        wait
        local result=$?
    fi
    
    # Merge all test logs
    echo "Merging test logs..."
    cat ${PARALLEL_LOGS_DIR}/*_merged.log > ${TESTP}/merged_parallel.log 2>/dev/null || true
    
    # Format the final log
    ttcn3_logformat ${TESTP}/merged_parallel.log > ${TESTP}/merged.log
    rm -f ${TESTP}/merged_parallel.log
    
    # Count results
    local passed=$(grep -c "Test .* PASSED" ${PARALLEL_LOGS_DIR}/*.log 2>/dev/null || echo 0)
    local failed=$(grep -c "Test .* FAILED" ${PARALLEL_LOGS_DIR}/*.log 2>/dev/null || echo 0)
    
    echo "===================="
    echo "Test Results Summary"
    echo "===================="
    echo "Total tests: $num_tests"
    echo "Passed: $passed"
    echo "Failed: $failed"
    echo "===================="
    
    return $result
}

# Main execution
cd ${SCRIPT_DIR}

# Set up cleanup trap
trap 'cleanup_servers' EXIT

# Start servers
if ! start_servers; then
    echo "Failed to start servers"
    exit 1
fi

# Export functions for parallel execution
export -f run_single_test
export SCRIPT_DIR PYSRVPATH TESTP WHCIHT PARALLEL_LOGS_DIR

if [ $PARALLEL_MODE -eq 1 ]; then
    # Parallel mode
    run_parallel_tests "$TEST_PATTERN" "$MAX_WORKERS"
    result=$?
else
    # Sequential mode
    if [ -z "$TEST_PATTERN" ]; then
        # Run all tests sequentially
        echo "Running all tests sequentially..."
        for test in $(get_all_tests); do
            run_single_test "$test" "seq"
            if [ $? -ne 0 ]; then
                result=1
            fi
        done
        
        # Merge logs
        cat ${PARALLEL_LOGS_DIR}/seq_*_merged.log > ${TESTP}/merged.log 2>/dev/null || true
    else
        # Run single test
        run_single_test "$TEST_PATTERN" "single"
        result=$?
        
        # Copy log
        cp ${PARALLEL_LOGS_DIR}/single_*_merged.log ${TESTP}/merged.log 2>/dev/null || true
    fi
fi

# Cleanup is handled by trap
exit ${result:-0}