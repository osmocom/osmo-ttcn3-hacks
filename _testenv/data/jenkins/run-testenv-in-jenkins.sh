#!/bin/sh -ex
# Script for the Osmocom jenkins to run testenv:
# https://jenkins.osmocom.org/jenkins/view/TTCN3/
test -n "$1"
test -n "$WORKSPACE"

TESTSUITE="$1"
DISTRO="debian:trixie"
CACHE_TMPFS=/tmp/jenkins/"$(basename "$WORKSPACE")"

pull_podman_image() {
	local image="registry.osmocom.org/osmocom-build/"$(echo "$DISTRO" | tr : -)"-osmo-ttcn3-testenv"
	podman pull "$image"
}

# Clean up leftover apt partial directories, which the host system user can't
# remove directly
fix_cache_tmpfs_permissions() {
	local i

	for i in "$CACHE_TMPFS"/podman/var-lib-apt-debian-*/*/partial; do
		if [ -e "$i" ]; then
			podman run \
				--rm \
				-v "$CACHE_TMPFS:$CACHE_TMPFS" \
				osmocom-build/debian-trixie-osmo-ttcn3-testenv \
				rm -rvf "$i"
		fi
	done
}

show_load() {
	uptime | grep --color=always -o "load.*"
}

run_testenv() {
	set +x	# Don't output the color codes we set here
	export TESTENV_COLOR_DEBUG="$(printf '\e[0;94m')"	# light blue
	export TESTENV_COLOR_INFO="$(printf '\e[1;34m')"	# bold, blue
	export TESTENV_COLOR_WARNING="$(printf '\e[1;35m')"	# bold, purple
	export TESTENV_COLOR_ERROR="$(printf '\e[1;91m')"	# bold, red
	export TESTENV_COLOR_CRITICAL="$(printf '\e[1;91m')"	# bold, red
	export TESTENV_SOURCE_HIGHLIGHT_COLORS="esc"
	export TESTENV_NO_IMAGE_UP_TO_DATE_CHECK=1
	export TESTENV_NO_KVM=1
	set -x

	./testenv.py run \
		"$TESTSUITE" \
		--podman \
		--cache "$CACHE_TMPFS" \
		--ccache ~/ccache/testenv \
		--log-dir "$PWD"/logs \
		$TESTENV_ARGS
}

main() {
	# Clean up from previous job (possibly aborted)
	fix_cache_tmpfs_permissions
	rm -rf logs .linux _cache "$CACHE_TMPFS"

	mkdir -p "$CACHE_TMPFS"
	pull_podman_image
	show_load

	if run_testenv; then
		show_load
		rm -rf .linux "$CACHE_TMPFS"
	else
		show_load
		fix_cache_tmpfs_permissions

		rm -f .linux
		mv "$CACHE_TMPFS" _cache

		exit 1
	fi
}

main
