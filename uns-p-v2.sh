#!/bin/bash

# Improved parallel extension to uns.sh with proper namespace isolation
# Usage:
#   ./uns-p-v2.sh                 # Run all tests sequentially
#   ./uns-p-v2.sh <TEST_NAME>     # Run single test
#   ./uns-p-v2.sh -p <N>          # Run all tests with N parallel workers
#   ./uns-p-v2.sh -p <N> <FILTER> # Run filtered tests with N parallel workers

set -e

WHCIHT="smdpp"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PYSRVPATH=$(realpath ${SCRIPT_DIR}/../pysim)
TESTP=$(realpath ${SCRIPT_DIR}/${WHCIHT})

# Parse command line arguments
PARALLEL_MODE=0
MAX_WORKERS=1
TEST_FILTER=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--parallel)
            PARALLEL_MODE=1
            MAX_WORKERS="${2:-4}"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS] [TEST_FILTER]"
            echo "Options:"
            echo "  -p, --parallel <N>  Run tests with N parallel workers (default: 4)"
            echo "  -h, --help          Show this help message"
            echo "Examples:"
            echo "  $0                  # Run all tests sequentially"
            echo "  $0 TC_SM_DP_       # Run tests matching pattern"
            echo "  $0 -p 8            # Run all tests with 8 workers"
            echo "  $0 -p 4 Cancel     # Run Cancel tests with 4 workers"
            exit 0
            ;;
        *)
            TEST_FILTER="$1"
            shift
            ;;
    esac
done

cd ${SCRIPT_DIR}

# Function to get test list (runs outside namespace)
get_test_list() {
    cd ${TESTP}
    export TTCN3_DIR=/app/titan
    export TITAN_LIBRARY_PATH=/app/titan/lib
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:.:$TITAN_LIBRARY_PATH"
    
    local all_tests=$(./${WHCIHT}_Tests -l 2>/dev/null | grep "^${WHCIHT}_Tests\.TC_" | sed "s/^${WHCIHT}_Tests\.//")
    
    if [ -n "$TEST_FILTER" ]; then
        echo "$all_tests" | grep "$TEST_FILTER"
    else
        echo "$all_tests"
    fi
}

# Get test list before entering namespace
TESTS=$(get_test_list)
NUM_TESTS=$(echo "$TESTS" | wc -l)

if [ -z "$TESTS" ]; then
    echo "No tests found matching filter: $TEST_FILTER"
    exit 1
fi

# Create a script that will run inside the namespace
cat > ${TESTP}/.namespace_runner.sh << 'EOF'
#!/bin/bash
set -e

# Arguments passed from parent
WHCIHT="$1"
TESTP="$2"
PYSRVPATH="$3"
PARALLEL_MODE="$4"
MAX_WORKERS="$5"
shift 5
TESTS="$@"

# Setup environment
export TTCN3_DIR=/app/titan
export TITAN_LIBRARY_PATH=/app/titan/lib
export TTCN3_BIN_DIR=/app/titan/bin
export PATH=/app/titan/bin:$PATH
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:.:$TITAN_LIBRARY_PATH"

# Bring up loopback interface
ip l s up lo

# Clean up old logs
rm -f ${TESTP}/*log ${TESTP}/.*pid

# Start Python servers
echo "Starting SM-DP+ servers in namespace: NIST on port 8000, BRP on port 8001"
( cd ${PYSRVPATH} && python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8000 2>&1 > ${TESTP}/_pyserver_nist.log & echo $! > ${TESTP}/.nist_pid )
( cd ${PYSRVPATH} && python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8001 --brainpool 2>&1 > ${TESTP}/_pyserver_brp.log & echo $! > ${TESTP}/.brp_pid )

# Wait for servers to start
sleep 2

# Verify servers are running
if ! kill -0 $(cat ${TESTP}/.nist_pid 2>/dev/null) 2>/dev/null; then
    echo "ERROR: NIST server failed to start"
    exit 1
fi
if ! kill -0 $(cat ${TESTP}/.brp_pid 2>/dev/null) 2>/dev/null; then
    echo "ERROR: BRP server failed to start"
    exit 1
fi

# Function to run a single test
run_single_test() {
    local test_name="$1"
    local log_suffix="$2"
    
    cd ${TESTP}
    
    # Clean test-specific logs
    rm -f ${WHCIHT}_Tests*.log
    
    # Run the test
    ../start-testsuite.sh ${WHCIHT}_Tests ${WHCIHT}_Tests.cfg ${test_name} 2>&1 | grep -v ' DEBUG   ' || true
    
    # Process logs
    local QLIST="DEBUG_ENCDEC DEBUG_UNQUALIFIED PORTEVENT_MMRECV PORTEVENT_MMSEND PORTEVENT_MQUEUE PARALLEL_PTC PORTEVENT_STATE EXECUTOR_ WARNING_UNQUALIFIED PORTEVENT_UNQUALIFIED PARALLEL_UNQUALIFIED"
    local QVARS=$(for file in $QLIST; do printf " -ve %s" "$file"; done)
    
    if ls ${WHCIHT}_Tests*.log 1> /dev/null 2>&1; then
        ttcn3_logmerge ${WHCIHT}_Tests*.log | grep ${QVARS} > _merged_${log_suffix}.log
        ttcn3_logformat _merged_${log_suffix}.log > merged_${log_suffix}.log
        rm -f ${WHCIHT}_Tests*.log _merged_${log_suffix}.log
    else
        echo "No test logs generated for $test_name" > merged_${log_suffix}.log
    fi
}

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    
    # Kill servers
    [ -f ${TESTP}/.nist_pid ] && kill $(cat ${TESTP}/.nist_pid) 2>/dev/null || true
    [ -f ${TESTP}/.brp_pid ] && kill $(cat ${TESTP}/.brp_pid) 2>/dev/null || true
    
    # Wait a moment for servers to shut down
    sleep 1
    
    # Process server logs
    if [ -f ${TESTP}/_pyserver_nist.log ]; then
        grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/_pyserver_nist.log > ${TESTP}/pyserver_nist.log || true
    fi
    if [ -f ${TESTP}/_pyserver_brp.log ]; then
        grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/_pyserver_brp.log > ${TESTP}/pyserver_brp.log || true
    fi
    
    # Create combined pyserver.log
    {
        echo '=== NIST Server Log (port 8000) ==='
        cat ${TESTP}/pyserver_nist.log 2>/dev/null || echo "No NIST server log found"
        echo ''
        echo '=== BRP Server Log (port 8001) ==='
        cat ${TESTP}/pyserver_brp.log 2>/dev/null || echo "No BRP server log found"
    } > ${TESTP}/pyserver.log
    
    # Clean up temporary files
    rm -f ${TESTP}/_pyserver_*.log ${TESTP}/.*pid ${TESTP}/.namespace_runner.sh
}
trap cleanup EXIT

# Export function for parallel execution
export -f run_single_test
export TESTP WHCIHT

# Run tests based on mode
if [ "$PARALLEL_MODE" -eq 1 ]; then
    echo "Running tests with $MAX_WORKERS parallel workers"
    
    # Run tests in parallel
    printf '%s\n' $TESTS | xargs -P "$MAX_WORKERS" -I {} bash -c '
        echo "[Worker $$] Starting: {}"
        run_single_test "{}" "parallel_$$_{}"
        echo "[Worker $$] Completed: {}"
    '
    
    # Merge parallel logs
    echo "Merging test logs..."
    cat ${TESTP}/merged_parallel_*.log > ${TESTP}/merged.log 2>/dev/null || echo "No test logs to merge" > ${TESTP}/merged.log
    rm -f ${TESTP}/merged_parallel_*.log
    
else
    # Sequential mode
    echo "Running tests sequentially"
    
    for test in $TESTS; do
        echo "Running: $test"
        run_single_test "$test" "seq_${test}"
    done
    
    # Merge sequential logs
    cat ${TESTP}/merged_seq_*.log > ${TESTP}/merged.log 2>/dev/null || echo "No test logs to merge" > ${TESTP}/merged.log
    rm -f ${TESTP}/merged_seq_*.log
fi

echo "All tests completed inside namespace"
EOF

chmod +x ${TESTP}/.namespace_runner.sh

# Display execution plan
echo "============================================"
echo "Test Execution Plan:"
echo "  Test suite: ${WHCIHT}"
echo "  Number of tests: ${NUM_TESTS}"
echo "  Execution mode: $([ $PARALLEL_MODE -eq 1 ] && echo "Parallel with $MAX_WORKERS workers" || echo "Sequential")"
echo "  Network isolation: Enabled (unshare)"
echo "============================================"

# Run everything inside namespace
unshare -UpmniCTfr -w ${WHCIHT} -- bash ${TESTP}/.namespace_runner.sh "$WHCIHT" "$TESTP" "$PYSRVPATH" "$PARALLEL_MODE" "$MAX_WORKERS" $TESTS

echo "Test execution completed. Results in ${TESTP}/merged.log"