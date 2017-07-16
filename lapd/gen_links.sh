#!/bin/sh

BASEDIR=~/projects/git

gen_links() {
	DIR=$1
	FILES=$*
	for f in $FILES; do
		echo "Linking $f"
		ln -sf $DIR/$f $f
	done
}

DIR=../sysinfo
FILES="General_Types.ttcn GSM_Types.ttcn Osmocom_Types.ttcn"
gen_links $DIR $FILES
