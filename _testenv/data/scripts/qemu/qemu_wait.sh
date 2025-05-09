#!/bin/sh -e
if [ -z "$TESTENV_QEMU_KERNEL" ]; then
	exit 0
fi

LOGFILE="$(basename "$PWD").log"

i=0
for i in $(seq 1 600); do
	sleep 0.1

	# Check for the marker from qemu_initrd_exit_error
	if [ -e build_initrd_failed ]; then
		exit 1
	fi

	if grep -q KERNEL_TEST_VM_IS_READY "$LOGFILE"; then
		# Wait some more for SUT to become ready
		sleep 1
		exit 0
	fi
done

echo
echo "ERROR: Timeout while waiting for QEMU to become ready"
echo

exit 1
