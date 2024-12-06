#!/bin/sh -e
# Wrapper for the ttcn3 compiler used by meson.build to ensure the generated
# cc files don't conflict.
target="$1"
shift 1

DIR="_$target"
mkdir -p "$DIR"

export TTCN3_DIR=/usr
"$@" -o "$DIR"

for i in "$DIR"/*.cc; do
	sed -i "s/^#include \"/#include \"$DIR\//g" "$i" > "$target"_"$(basename "$i")" &
done

wait
