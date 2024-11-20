#!/bin/sh -ex
osmo-config-merge osmo-bts.cfg osmo-bts-virtphy.confmerge > osmo-bts-virtphy.cfg

# Can't delete lines with osmo-config-merge
sed -i '/^ *osmotrx .*/d' osmo-bts-virtphy.cfg
