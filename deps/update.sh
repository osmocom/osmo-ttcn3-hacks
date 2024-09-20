#!/bin/sh -e
DIR="$1"
COMMIT="$2"

cd "$DIR"

if [ "$(git rev-parse HEAD)" = "$COMMIT" ]; then
	# Commit is already checked out, nothing to do!
	exit 0
fi

if ! git cat-file -e "$COMMIT"; then
	echo "[$DIR] Missing $COMMIT, fetching git repository"
	git fetch
fi

if git rev-parse -q "origin/$COMMIT" 1>/dev/null 2>&1; then
	echo "[$DIR] Checking out origin/$COMMIT"
	git checkout -q -f "origin/$COMMIT"
else
	echo "[$DIR] Checking out $COMMIT"
	git checkout -q -f "$COMMIT"
fi
