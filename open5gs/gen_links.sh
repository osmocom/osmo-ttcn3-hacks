#!/bin/bash -e

BASEDIR=../deps

. ../_buildsystem/gen_links.inc.sh

DIR=$BASEDIR/titan.ProtocolModules.JSON_v07_2006/src
FILES="JSON_Generic.ttcn "
gen_links $DIR $FILES

gen_links_finish
