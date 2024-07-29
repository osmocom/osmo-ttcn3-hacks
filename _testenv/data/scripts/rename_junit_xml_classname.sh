#!/bin/sh -e
# Some jenkins jobs run the same tests multiple times with different configs.
# Then we have a conflict of test names ("classname") in the test result files.
# This script renames the classnames in the test result files to avoid the
# conflict. Jenkins will them show them separated in the test result analyzer.

SUFFIX="$1"

if [ -z "$SUFFIX" ]; then
	echo "usage: rename_junit_xml_classname.sh SUFFIX"
	echo "example: rename_junit_xml_classname ':hopping'"
	exit 1
fi

xmls="$(find -name 'junit-xml*.log')"

if [ -z "$xmls" ]; then
	case "$TESTENV_CLEAN_REASON" in
		prepare|crashed)
			# No xml files is expected
			exit 0
			;;
		finished)
			echo "ERROR: could not find any junit-xml*.log files!"
			exit 1
			;;
		*)
			echo "ERROR: invalid TESTENV_CLEAN_REASON: $TESTENV_CLEAN_REASON"
			exit 1
			;;
	esac
fi

for i in $xmls; do
	echo "Adding '$SUFFIX' to classnames in: $i"
	sed -i "s/classname='\([^']\+\)'/classname='\1$SUFFIX'/g" $i
done
