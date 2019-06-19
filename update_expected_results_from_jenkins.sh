#!/bin/sh

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


# Download the latest junit xml results from the jenkins.osmocom.org workspaces.
# Usage:
# - have a clean git clone of osmo-ttcn3-hacks
# - have internet access to jenkins.osmocom.org
# - ./update_expected_results_from_jenkins.sh
# - git diff, make sure that you understand and approve of each and every change
# - git commit -a -m "update expected results"

not_found=""

if [ -n "$(git status | grep 'modified:')" ]; then
  echo "Your git clone contains modifications! This is not recommended."
  echo "Hit enter to continue anyway."
  read enter_to_continue
fi

for target in */expected-results.xml; do
  project="$(basename "$(dirname "$target")")"

  # shims for naming exceptions
  ws_path="ttcn3-${project}-test/ws/logs/${project}-tester"
  if [ "x$project" = "xggsn_tests" ]; then
    project="ggsn"
  elif [ "x$project" = "xsysinfo" ]; then
    ws_path="ttcn3-nitb-sysinfo/ws/logs/ttcn3-nitb-sysinfo"
  fi

  # find out the junit-NN.xml name
  dir_url="https://jenkins.osmocom.org/jenkins/job/$ws_path/"
  junit_file="$(wget -q -O - "$dir_url" | grep 'junit-xml-[0-9]*\.log' | tail -n 1 | sed 's/.*\(junit-xml-[0-9]*\.log\).*/\1/')"

  # update
  target_new="$target.new"
  if ! wget -O "$target_new" "${dir_url}$junit_file"; then
    not_found="$not_found $project"
    rm -f "$target_new"
  else
    mv "$target_new" "$target"
  fi
done

./mask_expected_results.sh

echo "

 MAKE SURE THE RESULTING CHANGES ARE SANE BEFORE COMMITTING!

"
if [ -n "$not_found" ]; then
	echo "Could not update: $not_found"
fi
