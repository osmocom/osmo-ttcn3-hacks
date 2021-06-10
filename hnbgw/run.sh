#!/bin/sh
set -xe
make compile || { set +x; echo; make compile 2>&1 | grep error: ; exit 1; }
make -j 5
../start-testsuite.sh HNBGW_Tests HNBGW_Tests.cfg $@
../log_merge.sh HNBGW_Tests --rm
