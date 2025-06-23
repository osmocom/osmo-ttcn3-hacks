#!/bin/bash

# Minimal parallel extension to uns.sh
# Usage:
#   ./uns-p.sh              # Run all tests sequentially (original behavior)
#   ./uns-p.sh <TEST_NAME>  # Run single test (original behavior)
#   ./uns-p.sh -p <N>       # Run all tests with N parallel workers

WHCIHT="smdpp"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PYSRVPATH=$(realpath ${SCRIPT_DIR}/../pysim)
TESTP=$(realpath ${SCRIPT_DIR}/${WHCIHT})

# Check for parallel mode
PARALLEL_MODE=0
MAX_WORKERS=1
TEST_FILTER=""

if [ "$1" = "-p" ] || [ "$1" = "--parallel" ]; then
    PARALLEL_MODE=1
    MAX_WORKERS="${2:-4}"
    shift 2
    TEST_FILTER="$@"
elif [ $# -gt 0 ]; then
    # Single test mode
    TEST_FILTER="$1"
fi

cd ${SCRIPT_DIR}

# Function to get test list
get_test_list() {
    cd ${TESTP}
    export TTCN3_DIR=/app/titan
    export TITAN_LIBRARY_PATH=/app/titan/lib
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:.:$TITAN_LIBRARY_PATH"
    ./${WHCIHT}_Tests -l 2>/dev/null | grep "^${WHCIHT}_Tests\.TC_" | sed "s/^${WHCIHT}_Tests\.//"
}

# Function to run a single test in namespace
run_test_in_namespace() {
    local test_name="$1"
    local log_suffix="${2:-$$}"
    
    QLIST="DEBUG_ENCDEC DEBUG_UNQUALIFIED PORTEVENT_MMRECV PORTEVENT_MMSEND PORTEVENT_MQUEUE PARALLEL_PTC PORTEVENT_STATE EXECUTOR_ WARNING_UNQUALIFIED PORTEVENT_UNQUALIFIED PARALLEL_UNQUALIFIED"
    QVARS=$(for file in $QLIST; do printf " -ve %s" "$file"; done)
    
    CMDSTR="export TTCN3_DIR=/app/titan; export TITAN_LIBRARY_PATH=/app/titan/lib; export TTCN3_BIN_DIR=/app/titan/bin; export PATH=/app/titan/bin:\$PATH; ip l s up lo;"
    CMDSTR+="cd ${TESTP}; rm -f ${WHCIHT}*log;"
    CMDSTR+="../start-testsuite.sh ${WHCIHT}_Tests ${WHCIHT}_Tests.cfg ${test_name} 2>&1 | grep -v ' DEBUG   ';"
    CMDSTR+="ttcn3_logmerge ${WHCIHT}*log | grep "${QVARS}" > _merged_${log_suffix}.log;"
    CMDSTR+="ttcn3_logformat _merged_${log_suffix}.log > merged_${log_suffix}.log;"
    CMDSTR+="rm -f ${WHCIHT}*log _merged_${log_suffix}.log;"
    
    # Run in same namespace as servers (no unshare for network)
    bash -c "${CMDSTR}"
    return $?
}

# Start Python servers
echo "Starting SM-DP+ servers: NIST on port 8000, BRP on port 8001"
rm -f ${TESTP}/*log ${TESTP}/.*pid
( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8000 2>&1 > ${TESTP}/_pyserver_nist.log & echo $! > ${TESTP}/.nist_pid )
( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8001 --brainpool 2>&1 > ${TESTP}/_pyserver_brp.log & echo $! > ${TESTP}/.brp_pid )
sleep 2

# Cleanup function
cleanup() {
    echo "Stopping servers..."
    [ -f ${TESTP}/.nist_pid ] && kill $(cat ${TESTP}/.nist_pid) 2>/dev/null
    [ -f ${TESTP}/.brp_pid ] && kill $(cat ${TESTP}/.brp_pid) 2>/dev/null
    
    # Process server logs
    grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/_pyserver_nist.log > ${TESTP}/pyserver_nist.log 2>/dev/null || true
    grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/_pyserver_brp.log > ${TESTP}/pyserver_brp.log 2>/dev/null || true
    
    # Create combined pyserver.log
    echo '=== NIST Server Log (port 8000) ===' > ${TESTP}/pyserver.log
    cat ${TESTP}/pyserver_nist.log >> ${TESTP}/pyserver.log 2>/dev/null || true
    echo '' >> ${TESTP}/pyserver.log
    echo '=== BRP Server Log (port 8001) ===' >> ${TESTP}/pyserver.log
    cat ${TESTP}/pyserver_brp.log >> ${TESTP}/pyserver.log 2>/dev/null || true
    
    rm -f ${TESTP}/_pyserver_*.log ${TESTP}/.*pid
}
trap cleanup EXIT

# Main execution
if [ -n "$TEST_FILTER" ] && [ $PARALLEL_MODE -eq 0 ]; then
    # Single test mode
    echo "Running single test: $TEST_FILTER"
    run_test_in_namespace "$TEST_FILTER" "single"
    cp ${TESTP}/merged_single.log ${TESTP}/merged.log 2>/dev/null || true
    
elif [ $PARALLEL_MODE -eq 1 ]; then
    # Parallel mode
    if [ -n "$TEST_FILTER" ]; then
        TESTS=$(get_test_list | grep "$TEST_FILTER")
    else
        TESTS=$(get_test_list)
    fi
    
    NUM_TESTS=$(echo "$TESTS" | wc -l)
    echo "Running $NUM_TESTS tests with $MAX_WORKERS parallel workers"
    
    # Export function and variables for xargs
    export -f run_test_in_namespace get_test_list
    export TESTP WHCIHT SCRIPT_DIR
    
    # Run tests in parallel
    echo "$TESTS" | xargs -P "$MAX_WORKERS" -I {} bash -c 'echo "[$$] Starting: {}"; run_test_in_namespace "{}" "parallel_$$"; echo "[$$] Completed: {}"'
    
    # Merge all parallel logs
    echo "Merging test logs..."
    cat ${TESTP}/merged_parallel_*.log > ${TESTP}/merged.log 2>/dev/null || true
    rm -f ${TESTP}/merged_parallel_*.log
    
else
    # Sequential mode (all tests)
    echo "Running all tests sequentially..."
    TESTS=$(get_test_list)
    
    for test in $TESTS; do
        echo "Running: $test"
        run_test_in_namespace "$test" "seq_$$"
    done
    
    # Merge sequential logs
    cat ${TESTP}/merged_seq_*.log > ${TESTP}/merged.log 2>/dev/null || true
    rm -f ${TESTP}/merged_seq_*.log
fi

echo "Test execution completed. Check merged.log for results."