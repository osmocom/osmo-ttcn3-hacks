#!dont_run_this
# This file is sourced by */gen_links.sh

gen_links() {
	DIR=$1
	shift
	FILES=$*
	for f in $FILES; do
		echo "Linking $f"
		ln -sf $DIR/$f $f
	done
}
