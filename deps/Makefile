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

ECLIPSEGITLAB:=https://gitlab.eclipse.org/eclipse/titan
OSMOGITHUB:=https://github.com/osmocom
OSMOGIT:=git://git.osmocom.org

ECLIPSEGITLAB_REPOS=	titan.Libraries.TCCUsefulFunctions \
			titan.ProtocolEmulations.M3UA \
			titan.ProtocolModules.BSSAPP_v7.3.0 \
			titan.ProtocolModules.DIAMETER_ProtocolModule_Generator \
			titan.ProtocolModules.DNS \
			titan.ProtocolModules.GRE \
			titan.ProtocolModules.GTP_v13.5.0 \
			titan.ProtocolModules.GTPv2_v13.7.0 \
			titan.ProtocolModules.ICMP \
			titan.ProtocolModules.ICMPv6 \
			titan.ProtocolModules.IP \
			titan.ProtocolModules.ISUP_Q.762 \
			titan.ProtocolModules.JSON_v07_2006 \
			titan.ProtocolModules.L2TP \
			titan.ProtocolModules.LLC_v7.1.0 \
			titan.ProtocolModules.M3UA \
			titan.ProtocolModules.MobileL3_v13.4.0 \
			titan.ProtocolModules.SMPP \
			titan.ProtocolModules.TCP \
			titan.ProtocolModules.UDP \
			titan.ProtocolModules.RTP \
			titan.ProtocolModules.M2PA \
			titan.ProtocolModules.ROSE \
			titan.ProtocolModules.SCTP \
			titan.ProtocolModules.SDP \
			titan.ProtocolModules.SUA \
			titan.ProtocolModules.NAS_EPS_15.2.0.1 \
			titan.ProtocolModules.NS_v7.3.0 \
			titan.ProtocolModules.SGsAP_13.2.0 \
			titan.ProtocolModules.SNDCP_v7.0.0 \
			titan.TestPorts.Common_Components.Socket-API \
			titan.TestPorts.Common_Components.Abstract_Socket \
			titan.TestPorts.HTTPmsg \
			titan.TestPorts.IPL4asp \
			titan.TestPorts.LANL2asp \
			titan.TestPorts.PCAPasp \
			titan.TestPorts.PIPEasp \
			titan.TestPorts.SIPmsg \
			titan.TestPorts.TELNETasp \
			titan.TestPorts.TCPasp \
			titan.TestPorts.UNIX_DOMAIN_SOCKETasp

OSMOGITHUB_REPOS=	titan.TestPorts.SCTPasp \
			titan.TestPorts.MTP3asp \
			titan.TestPorts.UDPasp \
			titan.ProtocolEmulations.SCCP \
			titan.ProtocolModules.BSSGP_v13.0.0

OSMOGIT_REPOS=		titan.ProtocolModules.MAP \
			titan.ProtocolModules.BSSMAP \
			titan.TestPorts.USB \
			titan.TestPorts.AF_PACKET \
			osmo-uecups

ALL_REPOS=$(ECLIPSEGITLAB_REPOS) $(OSMOGITHUB_REPOS) $(OSMOGIT_REPOS) $(FORKEDGITLAB_REPOS)

# Tag names from 'git-describe --tags'; if not available, a commit hash may be used instead.
# Do not put references to branches here, except for local testing: this breaks the caching
# logic of docker containers, which only invalidate their cached ttcn3 dependencies if this
# file changed.
titan.Libraries.TCCUsefulFunctions_commit=	R.35.B-6-gb3687da
titan.ProtocolEmulations.M3UA_commit=		f086e78d74defa044d864f17adaad9433fedc961
titan.ProtocolEmulations.SCCP_commit=	 	R.7.A-11-gd4b7a6d
titan.ProtocolModules.BSSAPP_v7.3.0_commit=	R.2.A-4-g20cfaf8
titan.ProtocolModules.BSSGP_v13.0.0_commit=	e97d92a8b66bec399babea52f593771b76cb175a
titan.ProtocolModules.BSSMAP_commit=		24f967b2f77bfdd2e6eabf1e82fa76239095f772
titan.ProtocolModules.DNS_commit=		R.7.B
titan.ProtocolModules.GRE_commit=		R.2.A-5-g654c446
titan.ProtocolModules.GTP_v13.5.0_commit=	54219599037626c675610d3a44433d01af264fbb
titan.ProtocolModules.GTPv2_v13.7.0_commit=	R.2.A-5-g85cb124
titan.ProtocolModules.ICMP_commit=		e49d9fb9f7de637b4bf4803dc6b6e911a8661640
titan.ProtocolModules.ICMPv6_commit=		46f4d9b6e1e3c794294a92588401a81e4881dd27
titan.ProtocolModules.IP_commit=		1be86705f39ae38f3c04b2109806ee20d25e91d0
titan.ProtocolModules.ISUP_Q.762_commit=	R.8.A-5-g42d428b
titan.ProtocolModules.JSON_v07_2006_commit=	R.1.A
titan.ProtocolModules.L2TP_commit=		17e76d3662bd0bb815158e8a9de1ec413f21b530
titan.ProtocolModules.LLC_v7.1.0_commit=	09817f113255d7fb56f1d45d3dd629a093d9248d
titan.ProtocolModules.MAP_commit=		R.2.A-1-g79c6a3d
titan.ProtocolModules.M2PA_commit=		R.2.A-5-ga75b589
titan.ProtocolModules.M3UA_commit=		c496d298876fed55c2b730278b7ee77982555563
titan.ProtocolModules.MobileL3_v13.4.0_commit=	badbad680df216b3211260d56b14734eeb2c9028
titan.ProtocolModules.NAS_EPS_15.2.0.1_commit=	R.1.A-2-g2ba853c
titan.ProtocolModules.NS_v7.3.0_commit=		R.2.A-6-gf73f195
titan.ProtocolModules.RTP_commit=		R.5.A-4-g83ee83a
titan.ProtocolModules.DIAMETER_ProtocolModule_Generator_commit=	ffd939595a08da1b8c8176aaa1f8578bfe02a912
titan.ProtocolModules.ROSE_commit=		R.1.C-5-g0f6774c
titan.ProtocolModules.SCTP_commit=		R.2.A-5-g556ef3f
titan.ProtocolModules.SGsAP_13.2.0_commit=	R.1.B-3-g5ccaeab
titan.ProtocolModules.SDP_commit=		R.14.A-6-g7f00ea0
titan.ProtocolModules.SMPP_commit=		R.2.A-5-gf2c9017
titan.ProtocolModules.SNDCP_v7.0.0_commit=	R.2.A-5-gd0f0ce6
titan.ProtocolModules.SUA_commit=		R.5.A-5-gcf1137a
titan.ProtocolModules.TCP_commit=		R.3.A-5-g39e5f45
titan.ProtocolModules.UDP_commit=		R.4.A-5-geea8aa3
titan.TestPorts.AF_PACKET_commit=		0.1-5-g89ebea6
titan.TestPorts.Common_Components.Socket-API_commit=	R.6.A-6-gf4380d0
titan.TestPorts.Common_Components.Abstract_Socket_commit=	R.9.B-4-gbd41994
titan.TestPorts.HTTPmsg_commit=			R.9.B-4-g9a0dfde
titan.TestPorts.IPL4asp_commit=			R.30.E-7-g029bccb
titan.TestPorts.LANL2asp_commit=		R.8.C-3-gb07c265
titan.TestPorts.MTP3asp_commit=	 		1cecdad6f3641a5f19b3833703bff6e5005eff11
titan.TestPorts.PCAPasp_commit=			R.8.A-3-g9ad320f
titan.TestPorts.PIPEasp_commit=			R.7.D-3-g8b01154
titan.TestPorts.SCTPasp_commit=			R.11.A-8-g2e911c3
titan.TestPorts.SIPmsg_commit=			R.15.A-1-ge4f9dd0
titan.TestPorts.TCPasp_commit=			R.9.A-5-g2c91bc6
titan.TestPorts.TELNETasp_commit=		873fe539642542cd9a901c208f1ec11c6d2f5387
titan.TestPorts.UDPasp_commit=			c20d77a34f288dd70dd4aaa30e520778876e9336
titan.TestPorts.UNIX_DOMAIN_SOCKETasp_commit=	R.2.A-13-gd34ab71
titan.TestPorts.USB_commit=			4c742e931aae66c0046cebfe3c1de4618b6f9ffc
osmo-uecups_commit=				8362efef7c6fa341eb947a75786878e0685767b7

all: $(foreach dir,$(ALL_REPOS),$(dir)/update)
clean: $(foreach dir,$(ALL_REPOS),$(dir)/clean)
distclean: $(foreach dir,$(ALL_REPOS),$(dir)/distclean)

define GIT_template
$(1)_ORIGIN!=	if [ -d $(1) ]; then cd $(1) && git remote get-url origin; fi
$(1)_HEAD!=	if [ -d $(1) ]; then cd $(1) && git describe --tags 2>/dev/null || git rev-parse HEAD; fi
$(1)_MODIFIED!=	if [ -d $(1) ]; then cd $(1) && git diff --quiet --exit-code || echo -n "1"; fi

$(1):
	git clone $(2)/$(1)

.PHONY: $(1)/update
$(1)/update: $(1)
ifeq ($$($(1)_MODIFIED),1)
	@echo "WARNING: $(1) skipped because it contains uncommitted modifications!"
else
ifneq ($$($(1)_ORIGIN),$(2)/$(1))
	cd $(1) && git remote set-url origin $(2)/$(1) && git fetch
endif
ifneq ($$($(1)_HEAD),$($(1)_commit))
	cd $(1) && \
		git fetch && \
		if git rev-parse "origin/$($(1)_commit)" 2>/dev/null; then \
			set -x && \
			git checkout -q -f "origin/$($(1)_commit)"; \
		else \
			set -x && \
			git checkout -q -f "$($(1)_commit)"; \
		fi
endif
endif

.PHONY: $(1)/clean
$(1)/clean: $(1)
ifeq ($$($(1)_MODIFIED),1)
	@echo "WARNING: $(1) skipped because it contains uncommitted modifications!"
else
	cd $(1) && git fetch && git checkout -q -f "$($(1)_commit)" && git reset --hard
endif

.PHONY: $(1)/distclean
$(1)/distclean:
ifeq ($$($(1)_MODIFIED),1)
	@echo "WARNING: $(1) skipped because it contains uncommitted modifications!"
else
	@rm -rf $(1)
endif
endef

$(foreach dir,$(ECLIPSEGITLAB_REPOS), \
	$(eval $(call GIT_template,$(dir),$(ECLIPSEGITLAB))))

$(foreach dir,$(OSMOGITHUB_REPOS), \
	$(eval $(call GIT_template,$(dir),$(OSMOGITHUB))))

$(foreach dir,$(OSMOGIT_REPOS), \
	$(eval $(call GIT_template,$(dir),$(OSMOGIT))))

$(foreach dir,$(FORKEDGITLAB_REPOS), \
	$(eval $(call GIT_template,$(dir),$(FORKEDGITLAB))))
