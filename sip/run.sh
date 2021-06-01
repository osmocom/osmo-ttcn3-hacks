#!/bin/sh
set -xe
make compile
make -j 5
../start-testsuite.sh SIP_Tests
../log_merge.sh SIP_Tests --rm
