#!/bin/sh
set -xe
make compile || { set +x; echo; make compile 2>&1 | grep error: ; exit 1; }
make -j 5
../start-testsuite.sh MSC_Tests MSC_Tests.cfg $@
../log_merge.sh MSC_Tests --rm
