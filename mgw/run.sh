#!/bin/sh
set -xe
make compile || { set +x; echo; make compile 2>&1 | grep error: ; exit 1; }
make -j 5
../start-testsuite.sh MGCP_Test MGCP_Test.cfg $@
../log_merge.sh MGCP_Test --rm
