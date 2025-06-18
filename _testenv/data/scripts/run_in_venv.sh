#!/bin/sh -e
if [ -n "$TESTENV_INSTALL_DIR" ]; then
	. "$TESTENV_INSTALL_DIR"/venv/bin/activate
fi

"$@"
