#!dont_run_this
# This file is sourced by */gen_links.sh

# Copyright 2017 Harald Welte
# Copyright 2018 sysmocom - s.f.m.c. GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
	# Avoid using the pattern itself if no file is found in globbing below:
	shopt -s nullglob
	for pp in *.ttcnpp; do
		ttcn_file="$(echo $pp | sed 's/pp$//')"
		echo "$ttcn_file" >> .gitignore
	done
}
