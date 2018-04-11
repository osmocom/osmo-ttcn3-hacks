#!/bin/sh
# to prevent diffs due to timing and source file lines, mask some numbers

for target in */expected-results.xml; do
  sed -i "s/time='[^']*'/time='MASKED'/g" "$target"
  sed -i 's/ttcn:[0-9]\+/ttcn:MASKED/g' "$target"
done
