#!/bin/sh -e
export PYHSS_CONFIG=config.yaml

if [ "$TESTENV_INSTALL_DIR" = "/" ]; then
	# Installed via debian package
	export PATH=/opt/venvs/pyhss/bin:"$PATH"
else
	# Built with osmo-dev
	. "$TESTENV_INSTALL_DIR"/venv/bin/activate
fi

"$@"
