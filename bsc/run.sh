#!/bin/sh
set -xe
make compile || { set +x; echo; make compile 2>&1 | grep error: ; exit 1; }
make -j 5
../start-testsuite.sh BSC_Tests BSC_Tests.cfg $@
../log_merge.sh BSC_Tests --rm
