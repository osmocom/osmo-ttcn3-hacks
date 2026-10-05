#!/bin/sh -ex

# osmo-ns-dummy is always built with osmo-dev, even if using --binary-packages
exec "$OSMO_DEV_MAKE_DIR"/libosmocore/utils/osmo-ns-dummy "$@"
