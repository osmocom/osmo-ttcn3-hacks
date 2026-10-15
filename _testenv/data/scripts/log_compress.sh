#!/bin/sh -e
# Compress big log files (OS#7105)
min=1000000  # ~ 1 MiB

for i in *.merged; do
	size="$(stat -c"%s" "$i")"

	if [ $size -lt $min ]; then
		continue
	fi

	pigz -9 "$i"
done
