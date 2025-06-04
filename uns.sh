#!/bin/bash
WHCIHT="smdpp"
CMDSTR="ip l s up lo; ip a;  ( cd ../../pysim/; ./osmo-smdpp.py -H 127.0.0.1 2>&1 > /app/tt-smdpp/smdp/pyserver.log &) ; sleep 2;"
CMDSTR+="ss -anlp4;"
CMDSTR+="rm *log; ../start-testsuite.sh ${WHCIHT}_Tests ${WHCIHT}_Tests.cfg;"
CMDSTR+="ttcn3_logmerge smdp*log > _merged.log;"
CMDSTR+="ttcn3_logformat _merged.log > merged.log; rm _merged.log;"

echo ${CMDSTR}
unshare -UpmniCTfr -w ${WHCIHT} -- sh -c "${CMDSTR}"
exit 0
