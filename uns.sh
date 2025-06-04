#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PYSRVPATH=$(realpath ${SCRIPT_DIR}/../pysim/)

cd ${SCRIPT_DIR}

QLIST="DEBUG_ENCDEC DEBUG_UNQUALIFIED PORTEVENT_MMRECV PORTEVENT_MMSEND PORTEVENT_MQUEUE PARALLEL_PTC PORTEVENT_STATE EXECUTOR_ WARNING_UNQUALIFIED PORTEVENT_UNQUALIFIED PARALLEL_UNQUALIFIED"
QVARS=$(for file in $QLIST; do printf " -ve %s" "$file"; done)

WHCIHT="smdpp"
CMDSTR="ip l s up lo; ( cd ${PYSRVPATH}; python3 -u ./osmo-smdpp.py -H 127.0.0.1 2>&1 > pyserver.log &) ; sleep 2;"
CMDSTR+="rm *log; ../start-testsuite.sh ${WHCIHT}_Tests ${WHCIHT}_Tests.cfg;"
CMDSTR+="ttcn3_logmerge smdp*log | grep "${QVARS}" > _merged.log;"
CMDSTR+="ttcn3_logformat _merged.log > merged.log; rm _merged.log; sleep 5"

echo ${CMDSTR}
unshare -UpmniCTfr -w ${WHCIHT} -- sh -c "${CMDSTR}"
exit 0
