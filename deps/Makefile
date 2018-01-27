
ECLIPSEGIT:=https://github.com/eclipse
ECLIPSEGIT2:=git://git.eclipse.org/gitroot/titan
OSMOGITHUB:=https://github.com/osmocom

ECLIPSEGIT_REPOS=	titan.Libraries.TCCUsefulFunctions \
			titan.ProtocolModules.ICMP \
			titan.ProtocolModules.ICMPv6 \
			titan.ProtocolModules.IP \
			titan.ProtocolModules.L2TP \
			titan.ProtocolModules.M3UA \
			titan.ProtocolModules.SMPP \
			titan.ProtocolModules.TCP \
			titan.ProtocolModules.UDP \
			titan.ProtocolModules.RTP \
			titan.TestPorts.Common_Components.Socket-API \
			titan.TestPorts.IPL4asp \
			titan.TestPorts.LANL2asp \
			titan.TestPorts.PCAPasp \
			titan.TestPorts.SCTPasp \
			titan.TestPorts.SIPmsg \
			titan.TestPorts.TCPasp \
			titan.TestPorts.TELNETasp \
			titan.TestPorts.UDPasp \
			titan.TestPorts.UNIX_DOMAIN_SOCKETasp

ECLIPSEGIT2_REPOS=	titan.ProtocolModules.GRE \
			titan.ProtocolModules.M2PA \
			titan.ProtocolModules.SCTP \
			titan.ProtocolModules.SUA \
			titan.ProtocolModules.SDP \
			titan.TestPorts.GPIO \
			titan.ProtocolModules.BSSAPP_v7.3.0 \
			titan.ProtocolModules.BSSGP_v13.0.0 \
			titan.ProtocolModules.BSSMAP_v11.2.0 \
			titan.ProtocolModules.GTP_v13.5.0 \
			titan.ProtocolModules.GTPv2_v13.7.0 \
			titan.ProtocolModules.ISUP_Q.762 \
			titan.ProtocolModules.LLC_v7.1.0 \
			titan.ProtocolModules.MobileL3_v13.4.0 \
			titan.ProtocolModules.NS_v7.3.0 \
			titan.ProtocolModules.SNDCP_v7.0.0

OSMOGITHUB_REPOS=	

ALL_REPOS=$(ECLIPSEGIT_REPOS) $(ECLIPSEGIT2_REPOS) $(OSMOGITHUB_REPOS)

all: $(ALL_REPOS)

update: $(foreach dir,$(ALL_REPOS),$(dir)/update)
clean: $(foreach dir,$(ALL_REPOS),$(dir)/clean)

define GIT_template
$(1):
	git clone $(2)/$(1)

.PHONY:$(1)/update
$(1)/update: $(1)
	(cd $(1) && git fetch && git checkout -f -B master)

.PHONY: $(1)/clean
$(1)/clean:
	@rm -rf $(1)
endef

$(foreach dir,$(ECLIPSEGIT_REPOS), \
	$(eval $(call GIT_template,$(dir),$(ECLIPSEGIT))))

$(foreach dir,$(ECLIPSEGIT2_REPOS), \
	$(eval $(call GIT_template,$(dir),$(ECLIPSEGIT2))))

$(foreach dir,$(OSMOGITHUB_REPOS), \
	$(eval $(call GIT_template,$(dir),$(OSMOGITHUB))))
