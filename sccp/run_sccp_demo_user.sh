#!/bin/sh -ex

if [ -n "$OSMO_DEV_MAKE_DIR" ]; then
	# Using osmo-dev: either with or without --podman
	exec "$OSMO_DEV_MAKE_DIR"/libosmo-sigtran/examples/sccp_demo_user "$@"
fi

# Using binary packages: as sccp_demo_user is not packaged it gets
# built by testenv/podman_install.py:from_source_sccp_demo_user()
# and symlinked to /usr/local/bin.
exec sccp_demo_user "$@"
