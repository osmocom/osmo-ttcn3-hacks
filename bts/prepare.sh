#!/bin/sh -ex
CONFIG="$1"

check_usage()  {
	local valid="generic|hopping|oml"

	case "|$valid|" in
		*"|$CONFIG|"*)
			;;
		*)
			set +x
			echo "usage: prepare.sh $valid"
			exit 1
			;;
	esac
}

setcap_osmo_bts_trx() {
	sudo setcap CAP_SYS_NICE=+eip "$(which osmo-bts-trx)"
}

adjust_osmo_bts_config() {
	if [ "$CONFIG" != "oml" ]; then
		return
	fi

	osmo-config-merge \
		"osmo-bts.cfg" \
		"osmo-bts-$CONFIG.confmerge" \
		> "osmo-bts-$CONFIG.cfg"
}

check_usage
setcap_osmo_bts_trx
adjust_osmo_bts_config
