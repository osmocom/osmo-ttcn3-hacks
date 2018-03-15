#!dont_run_this
# This file is sourced by */gen_links.sh

rm -f .gitignore

gen_link() {
	src="$1"
	f="$2"
	echo "Linking $f"
	ln -sf "$src" "$f"
	echo "$f" >> .gitignore
}

gen_links() {
	DIR=$1
	shift
	FILES=$*
	for f in $FILES; do
		gen_link "$DIR/$f" "$f"
	done
}

ignore_pp_results() {
	for pp in *.ttcnpp; do
		ttcn_file="$(echo $pp | sed 's/pp$//')"
		echo "$ttcn_file" >> .gitignore
	done
}
