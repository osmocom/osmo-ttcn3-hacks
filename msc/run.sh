#!/bin/sh
set -xe
make compile
make -j 5
../start-testsuite.sh MSC_Tests MSC_Tests.cfg $@
../log_merge.sh MSC_Tests --rm
