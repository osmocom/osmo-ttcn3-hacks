#!/bin/sh -e
export PYHSS_CONFIG=config.yaml

if [ "$TESTENV_INSTALL_DIR" = "/" ]; then
	# Installed via debian package
	. /opt/venvs/pyhss/bin/activate
else
	# Built with osmo-dev
	. "$TESTENV_INSTALL_DIR"/venv/bin/activate
fi

"$@"
