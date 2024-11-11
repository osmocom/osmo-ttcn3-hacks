#!/bin/sh -ex

adjust_testsuite_config() {
	sed -i 's/^UPF_Tests.mp_upf_impl := .*/UPF_Tests.mp_upf_impl := UPF_IMPL_EUPF/' \
		UPF_Tests.cfg
}

# * CAP_BPF and CAP_NET_ADMIN: as mentioned here:
#   https://github.com/edgecomllc/eupf/blob/55ce219c09776470f9a8f66bea020466a61e4e87/.deploy/helm/eupf/values.yaml#L82-L85
# * CAP_SYS_RESOURCE: fails with "Can't increase resource limits: operation not
#   permitted" otherwise
# * CAP_SYS_ADMIN: fails with "attempt to corrupt spilled pointer on stack"
#   otherwise, like here: https://github.com/PowerDNS/pdns/issues/14279
setcap_eupf() {
	SETCAP_ARGS="
		CAP_BPF=+eip
		CAP_NET_ADMIN=+eip
		CAP_SYS_ADMIN=+eip
		CAP_SYS_RESOURCE=+eip
	"
	sudo setcap "$SETCAP_ARGS" "$(which eupf)"
}

adjust_testsuite_config
setcap_eupf
