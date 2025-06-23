#!/bin/bash

WHCIHT="smdpp"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PYSRVPATH=$(realpath ${SCRIPT_DIR}/../pysim)
TESTP=$(realpath ${SCRIPT_DIR}/${WHCIHT})

# Check if a specific test case was provided as an argument
TEST_CASE=""
if [ $# -gt 0 ]; then
    TEST_CASE="$1"
    echo "Running specific test case: $TEST_CASE"
fi

cd ${SCRIPT_DIR}

QLIST="DEBUG_ENCDEC DEBUG_UNQUALIFIED PORTEVENT_MMRECV PORTEVENT_MMSEND PORTEVENT_MQUEUE PARALLEL_PTC PORTEVENT_STATE EXECUTOR_ WARNING_UNQUALIFIED PORTEVENT_UNQUALIFIED PARALLEL_UNQUALIFIED"
QVARS=$(for file in $QLIST; do printf " -ve %s" "$file"; done)


CMDSTR="export TTCN3_DIR=/app/titan; export TITAN_LIBRARY_PATH=/app/titan/lib; export TTCN3_BIN_DIR=/app/titan/bin; export PATH=/app/titan/bin:\$PATH; ip l s up lo; rm ${TESTP}/*log;"
# Start NIST server on port 8000
CMDSTR+=" echo 'Starting SM-DP+ servers: NIST on port 8000, BRP on port 8001 (with in-memory session storage)';"
CMDSTR+=" ( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8000 --in-memory 2>&1 > ${TESTP}/_pyserver_nist.log & echo \$! > ${TESTP}/_nist_pid ) ;"
# Start BRP server on port 8001
CMDSTR+=" ( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8001 --brainpool --in-memory 2>&1 > ${TESTP}/_pyserver_brp.log & echo \$! > ${TESTP}/_brp_pid ) ;"
CMDSTR+=" sleep 1;"
# Set up cleanup trap to kill both servers
CMDSTR+=" trap 'kill \$(cat ${TESTP}/_nist_pid 2>/dev/null) 2>/dev/null; kill \$(cat ${TESTP}/_brp_pid 2>/dev/null) 2>/dev/null' EXIT;"
CMDSTR+="../start-testsuite.sh ${WHCIHT}_Tests ${WHCIHT}_Tests.cfg ${TEST_CASE} | grep -v ' DEBUG   ';"
# Kill both servers after tests complete
CMDSTR+="kill \$(cat ${TESTP}/_nist_pid 2>/dev/null) 2>/dev/null; kill \$(cat ${TESTP}/_brp_pid 2>/dev/null) 2>/dev/null;"
CMDSTR+="ttcn3_logmerge smdp*log | grep "${QVARS}" > ${TESTP}/_merged.log;"
#CMDSTR+="ttcn3_logformat ${TESTP}/_merged.log > ${TESTP}/merged.log; rm ${TESTP}/_merged.log; sleep 2"
CMDSTR+="ttcn3_logformat ${TESTP}/_merged.log > ${TESTP}/merged.log; find ${TESTP} -iname '*log' -not -name 'merged.log' -and -not -iname '*pyserver*.log' | xargs -n1 rm; rm ./*stderr; sleep 1;"
# Merge both server logs
CMDSTR+="grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/_pyserver_nist.log > ${TESTP}/pyserver_nist.log 2>/dev/null || true;"
CMDSTR+="grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/_pyserver_brp.log > ${TESTP}/pyserver_brp.log 2>/dev/null || true;"
# Create combined pyserver.log for backward compatibility
CMDSTR+="echo '=== NIST Server Log (port 8000) ===' > ${TESTP}/pyserver.log;"
CMDSTR+="cat ${TESTP}/pyserver_nist.log >> ${TESTP}/pyserver.log 2>/dev/null || true;"
CMDSTR+="echo '' >> ${TESTP}/pyserver.log;"
CMDSTR+="echo '=== BRP Server Log (port 8001) ===' >> ${TESTP}/pyserver.log;"
CMDSTR+="cat ${TESTP}/pyserver_brp.log >> ${TESTP}/pyserver.log 2>/dev/null || true;"
CMDSTR+="rm -f ${TESTP}/_pyserver_*.log ${TESTP}/_*_pid;"


# echo ${CMDSTR}
unshare -UpmniCTfr -w ${WHCIHT} -- sh -c "${CMDSTR}"
exit 0
