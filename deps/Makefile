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

ECLIPSEGIT:=https://github.com/eclipse
ECLIPSEGIT2:=git://git.eclipse.org/gitroot/titan
OSMOGITHUB:=https://github.com/osmocom
OSMOGIT:=git://git.osmocom.org

ECLIPSEGIT_REPOS=	titan.Libraries.TCCUsefulFunctions \
			titan.ProtocolModules.ICMP \
			titan.ProtocolModules.ICMPv6 \
			titan.ProtocolModules.IP \
			titan.ProtocolModules.JSON_v07_2006 \
			titan.ProtocolModules.L2TP \
			titan.ProtocolModules.SMPP \
			titan.ProtocolModules.TCP \
			titan.ProtocolModules.UDP \
			titan.ProtocolModules.RTP \
			titan.ProtocolModules.DIAMETER_ProtocolModule_Generator \
			titan.TestPorts.Common_Components.Socket-API \
			titan.TestPorts.Common_Components.Abstract_Socket \
			titan.TestPorts.HTTPmsg \
			titan.TestPorts.IPL4asp \
			titan.TestPorts.LANL2asp \
			titan.TestPorts.PCAPasp \
			titan.TestPorts.SIPmsg \
			titan.TestPorts.TCPasp \
			titan.TestPorts.TELNETasp \
			titan.TestPorts.UDPasp \
			titan.TestPorts.UNIX_DOMAIN_SOCKETasp

ECLIPSEGIT2_REPOS=	titan.ProtocolModules.GRE \
			titan.ProtocolModules.M2PA \
			titan.ProtocolModules.ROSE \
			titan.ProtocolModules.SCTP \
			titan.ProtocolModules.SUA \
			titan.ProtocolModules.SDP \
			titan.TestPorts.GPIO \
			titan.ProtocolModules.BSSAPP_v7.3.0 \
			titan.ProtocolModules.BSSGP_v13.0.0 \
			titan.ProtocolModules.GTP_v13.5.0 \
			titan.ProtocolModules.GTPv2_v13.7.0 \
			titan.ProtocolModules.ISUP_Q.762 \
			titan.ProtocolModules.MobileL3_v13.4.0 \
			titan.ProtocolModules.NAS_EPS_15.2.0.1 \
			titan.ProtocolModules.NS_v7.3.0 \
			titan.ProtocolModules.SNDCP_v7.0.0 \
			titan.ProtocolEmulations.SCCP

OSMOGITHUB_REPOS=	titan.TestPorts.SCTPasp \
			titan.ProtocolModules.LLC_v7.1.0 \
			titan.ProtocolModules.SGsAP_13.2.0 \
			titan.TestPorts.MTP3asp \
			titan.ProtocolModules.M3UA \
			titan.ProtocolEmulations.M3UA

OSMOGIT_REPOS=		titan.ProtocolModules.MAP \
			titan.ProtocolModules.BSSMAP \
			titan.TestPorts.USB

ALL_REPOS=$(ECLIPSEGIT_REPOS) $(ECLIPSEGIT2_REPOS) $(OSMOGITHUB_REPOS) $(OSMOGIT_REPOS)

# Tag names from 'git-describe --tags'; if not available, a commit hash may be used instead.
# In order to keep local changes in the repository of a dependency, set its commit to the
# name of a local branch here (e.g. 'master').
titan.Libraries.TCCUsefulFunctions_commit=	R.30.A
titan.ProtocolEmulations.M3UA_commit=		06b0f71a7d13fafc6f778974f6b4379ad87511ea
titan.ProtocolEmulations.SCCP_commit=	 	R.7.A-2-g724c83f
titan.ProtocolModules.BSSAPP_v7.3.0_commit=	R.2.A
titan.ProtocolModules.BSSGP_v13.0.0_commit=	R.2.A
titan.ProtocolModules.BSSMAP_commit=		master
titan.ProtocolModules.GRE_commit=		R.2.A
titan.ProtocolModules.GTP_v13.5.0_commit=	R.2.A
titan.ProtocolModules.GTPv2_v13.7.0_commit=	R.2.A
titan.ProtocolModules.ICMP_commit=		R.2.A
titan.ProtocolModules.ICMPv6_commit=		R.2.A
titan.ProtocolModules.IP_commit=		R.10.B-1-g99d0ec9
titan.ProtocolModules.ISUP_Q.762_commit=	R.8.A
titan.ProtocolModules.JSON_v07_2006_commit=	R.1.A
titan.ProtocolModules.L2TP_commit=		R.2.A
titan.ProtocolModules.LLC_v7.1.0_commit=	2a3c09fbf7bae22f802aa88689800f38a1f3732d
titan.ProtocolModules.MAP_commit=		R.2.A-1-g79c6a3d
titan.ProtocolModules.M2PA_commit=		R.2.A
titan.ProtocolModules.M3UA_commit=		2544b6bdc76d18313a235fbfdaf0cac04ebdd603
titan.ProtocolModules.MobileL3_v13.4.0_commit=	R.2.A
titan.ProtocolModules.NAS_EPS_15.2.0.1_commit=	2ba853c7d197f7f39d97b76fca4726a7143fb4ad
titan.ProtocolModules.NS_v7.3.0_commit=		R.2.A
titan.ProtocolModules.RTP_commit=		R.5.A
titan.ProtocolModules.DIAMETER_ProtocolModule_Generator_commit=	ffd939595a08da1b8c8176aaa1f8578bfe02a912
titan.ProtocolModules.ROSE_commit=		R.1.C
titan.ProtocolModules.SCTP_commit=		R.2.A
titan.ProtocolModules.SGsAP_13.2.0_commit=	520dc48dd41ee15c9fd77e71d2ea7a05c58770b9
titan.ProtocolModules.SDP_commit=		R.14.A
titan.ProtocolModules.SMPP_commit=		R.2.A-2-gb7aee69
titan.ProtocolModules.SNDCP_v7.0.0_commit=	R.2.A
titan.ProtocolModules.SUA_commit=		R.5.A
titan.ProtocolModules.TCP_commit=		R.3.A
titan.ProtocolModules.UDP_commit=		R.4.A
titan.TestPorts.Common_Components.Socket-API_commit=	R.6.A
titan.TestPorts.Common_Components.Abstract_Socket_commit=	R.9.B
titan.TestPorts.GPIO_commit=			R.3.A
titan.TestPorts.HTTPmsg_commit=			R.9.B
titan.TestPorts.IPL4asp_commit=			R.29.A
titan.TestPorts.LANL2asp_commit=		R.8.B
titan.TestPorts.MTP3asp_commit=	 		1cecdad6f3641a5f19b3833703bff6e5005eff11
titan.TestPorts.PCAPasp_commit=			R.7.A
titan.TestPorts.SCTPasp_commit=			R.11.A-2-g2faa9cf
titan.TestPorts.SIPmsg_commit=			R.15.A-1-ge4f9dd0
titan.TestPorts.TCPasp_commit=			R.9.A
titan.TestPorts.TELNETasp_commit=		R.9.A
titan.TestPorts.UDPasp_commit=			R.8.A
titan.TestPorts.UNIX_DOMAIN_SOCKETasp_commit=	R.2.A-4-gbacd73d
titan.TestPorts.USB_commit=			master

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
	cd $(1) && git fetch && git checkout -q -f "$($(1)_commit)"
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

$(foreach dir,$(ECLIPSEGIT_REPOS), \
	$(eval $(call GIT_template,$(dir),$(ECLIPSEGIT))))

$(foreach dir,$(ECLIPSEGIT2_REPOS), \
	$(eval $(call GIT_template,$(dir),$(ECLIPSEGIT2))))

$(foreach dir,$(OSMOGITHUB_REPOS), \
	$(eval $(call GIT_template,$(dir),$(OSMOGITHUB))))

$(foreach dir,$(OSMOGIT_REPOS), \
	$(eval $(call GIT_template,$(dir),$(OSMOGIT))))
