#!/bin/sh
set -xe
make compile
make -j 5
../start-testsuite.sh BSC_Tests BSC_Tests_VAMOS.cfg $@
../log_merge.sh BSC_Tests --rm
