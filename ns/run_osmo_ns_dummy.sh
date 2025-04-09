#!/bin/sh -ex

if [ -n "$OSMO_DEV_MAKE_DIR" ]; then
	# Using osmo-dev: either with or without --podman
	exec "$OSMO_DEV_MAKE_DIR"/libosmocore/utils/osmo-ns-dummy "$@"
fi

# Using binary packages: as osmo-ns-dummy is not packaged it gets built by
# testenv/podman_install.py:from_source_osmo_ns_dummy() and symlinked to
# /usr/local/bin.
exec osmo-ns-dummy "$@"
