#!/bin/sh
set -xe
make compile
make -j 5
../start-testsuite.sh PCU_Tests PCU_Tests.cfg $@
../log_merge.sh PCU_Tests --rm
