#!/bin/bash

export TTCN3_DIR="/usr"

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


CMDSTR="ip l s up lo; rm ${TESTP}/*log; ( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 2>&1 > ${TESTP}/_pyserver.log &) ; sleep 1;"
CMDSTR+="../start-testsuite.sh ${WHCIHT}_Tests ${WHCIHT}_Tests.cfg ${TEST_CASE} | grep -v ' DEBUG   ';"
CMDSTR+="ttcn3_logmerge smdp*log | grep "${QVARS}" > ${TESTP}/_merged.log;"
#CMDSTR+="ttcn3_logformat ${TESTP}/_merged.log > ${TESTP}/merged.log; rm ${TESTP}/_merged.log; sleep 2"
CMDSTR+="ttcn3_logformat ${TESTP}/_merged.log > ${TESTP}/merged.log; find ${TESTP} -iname '*log' -not -name 'merged.log' -and -not -iname '*pyserver.log' | xargs -n1 rm; sleep 1;"
CMDSTR+="grep -v 'DEBUG:pySim.esim.saip' ${TESTP}/_pyserver.log  > ${TESTP}/pyserver.log;"
CMDSTR+="rm ${TESTP}/_pyserver.log;"


# echo ${CMDSTR}
unshare -UpmniCTfr -w ${WHCIHT} -- sh -c "${CMDSTR}"
exit 0
