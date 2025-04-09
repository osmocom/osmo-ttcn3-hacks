#!/bin/sh -ex
# Script used by testenv/podman_install:from_source_osmo_ns_dummy() to build
# osmo-ns-dummy in libosmocore from git when running with "--binary-repo". This
# is needed, because osmo-ns-dummy is not packaged.
MAKE="make -j$1"

$MAKE -C include/osmocom/core all
$MAKE -C src/core libosmocore.la
$MAKE -C src/vty libosmovty.la
$MAKE -C src/isdn libosmoisdn.la
$MAKE -C src/gsm libosmogsm.la
$MAKE -C src/gb libosmogb.la
$MAKE -C src/ctrl libosmoctrl.la
$MAKE -C utils osmo-ns-dummy
