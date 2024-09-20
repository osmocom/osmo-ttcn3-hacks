#!/bin/sh -e
DIR="$1"
COMMIT="$2"

cd "$DIR"
git fetch

if git rev-parse "origin/$COMMIT" 2>/dev/null; then
	set -x
	git checkout -q -f "origin/$COMMIT"
else
	set -x
	git checkout -q -f "$COMMIT"
fi
