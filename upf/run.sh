#!/bin/sh
set -xe
make compile || { set +x; echo; make compile 2>&1 | grep error: ; exit 1; }
make -j 5
../start-testsuite.sh UPF_Tests UPF_Tests.cfg $@
../log_merge.sh UPF_Tests --rm
