#!/bin/sh
SO="/usr/lib/pcsc/drivers/serial/libifdvpcd.so"

if ! [ -e "$SO" ]; then
	echo
	echo "ERROR: $SO not found!"
	echo
	echo "This testsuite requires vsmartcard-vpcd."
	echo
	exit 1
fi
