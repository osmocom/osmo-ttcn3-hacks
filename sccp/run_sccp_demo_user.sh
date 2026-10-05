#!/bin/sh -ex

# sccp_demo_user is always built with osmo-dev, even if using --binary-packages
exec "$OSMO_DEV_MAKE_DIR"/libosmo-sigtran/examples/sccp_demo_user "$@"
