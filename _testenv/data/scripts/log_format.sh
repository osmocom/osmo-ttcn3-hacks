#!/bin/sh -e
# Run ttcn3_logformat on all merged log files.

for i in *.merged; do
	temp="$i.temp"
	ttcn3_logformat -o "$temp" "$i"
	mv "$temp" "$i"
done
