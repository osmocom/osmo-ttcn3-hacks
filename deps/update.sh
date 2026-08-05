#!/bin/sh -e
DIR="$1"
COMMIT="$2"
URL_PREFIX="$3"

get_full_url() {
	case "$URL_PREFIX" in
		*gitlab*)
			echo "$URL_PREFIX"/"$DIR".git
			;;
		*)
			echo "$URL_PREFIX"/"$DIR"
			;;
	esac
}

# Eclipse GitLab has rate limiting and sometimes too many concurrent
# connections fail. If that happens, sleep and try again in a few (random)
# seconds, to give less concurrent load to the server.
retry_with_backoff_time() {
	local max=5
	local sec
	local i

	for i in $(seq 1 $max); do
		if "$@"; then
			return
		fi

		if [ $i -lt $max ]; then
			sec=$(($i * $(shuf -i 1-10 -n1)))
			echo "[$DIR] Failed ($i/$max), retrying in ${sec}s..."
			sleep $sec
		else
			echo "[$DIR] Failed ($i/$max), giving up!"
			exit 1
		fi
		echo "[$DIR] Retrying: $@"
	done
}

update_url() {
	local current="$(git -C "$DIR" remote get-url origin)"
	local full_url="$(get_full_url)"

	if [ "$current" != "$full_url" ]; then
		echo "[$DIR] Updating URL to $full_url"
		git -C "$DIR" remote set-url origin "$full_url"
		retry_with_backoff_time git -C "$DIR" fetch
	fi
}

if [ -d "$DIR" ]; then
	update_url
else
	echo "[$DIR] Initial git clone"
	retry_with_backoff_time git clone -q "$(get_full_url)"
fi

cd "$DIR"

if [ "$(git rev-parse HEAD)" = "$COMMIT" ]; then
	# Commit is already checked out, nothing to do!
	exit 0
fi

if ! git cat-file -e "$COMMIT"; then
	echo "[$DIR] Missing $COMMIT, fetching git repository"
	retry_with_backoff_time git fetch
fi

if git rev-parse -q "origin/$COMMIT" 1>/dev/null 2>&1; then
	echo "[$DIR] Checking out origin/$COMMIT"
	git checkout -q -f "origin/$COMMIT"
else
	echo "[$DIR] Checking out $COMMIT"
	git checkout -q -f "$COMMIT"
fi
