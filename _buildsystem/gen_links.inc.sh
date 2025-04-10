# Copyright 2017 Harald Welte
# Copyright 2018-2025 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: Apache-2.0
# This file is sourced by */gen_links.sh

if [ -e Makefile ]; then
	echo
	echo "ERROR: found old build artefacts in source tree"
	echo
	echo "osmo-ttcn3-hacks does out-of-tree builds now. Make sure that you"
	echo "have comitted all code changes and run 'make clean-old' once"
	echo "to clean up your source tree, then try again."
	echo
	exit 1
fi

TOPDIR="$(realpath "$(dirname "$0")/..")"
PROJECTDIR=$(realpath . --relative-to "$TOPDIR")  # e.g. "msc", "library/rua"
BUILDDIR="${BUILDDIR:-$TOPDIR/_build}"

mkdir -p "$BUILDDIR/$PROJECTDIR"

gen_links() {
	local f
	local dir="$1"
	shift
	local files="$*"

	for f in $files; do
		(ln -sf \
			"$(realpath "$TOPDIR/$PROJECTDIR/$dir/$f")" \
			"$BUILDDIR/$PROJECTDIR/$f") &
	done
}

gen_links_finish() {
	local f
	local patterns="
		*.asn
		*.c
		*.cc
		*.h
		*.hh
		*.ttcn
		*.ttcnpp
	"
	for f in $patterns; do
		if ! [ -e "$f" ]; then
			continue
		fi

		ln -sf \
			"$TOPDIR/$PROJECTDIR/$f" \
			"$BUILDDIR/$PROJECTDIR/$f" &
	done
	wait
}
