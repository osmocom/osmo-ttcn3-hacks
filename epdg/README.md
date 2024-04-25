# EPDG_Tests.ttcn

* external interfaces
    * CEAI: GSUP (emulates strongswan)
    * SWx: Diameter (emulates HSS)
    * S6b: Diameter (emulates SMF)
    * S2b: GTPv2C (emulates SMF)
    * Emulates userplane IPsec decapsulated IP traffic at strongswan
    * Emulates userplane GTPv1U encapsulated IP traffic at UPF

{% dot epdg_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nEPDG_Tests.ttcn"];
  EPDG [label="EPDG\nosmo-epdg",shape="box", color=red];
  GTP_KERN [label="gtp\nkernel-module", shape="box"];

  ATS -> EPDG [label="CEAI (GSUP)"];
  ATS -> EPDG [label="S6b (Diameter)"];
  EPDG -> ATS [label="SWx (Diameter)"]
  EPDG -> ATS [label="S2b (GTPv2C)"];
  ATS -> EPDG [label="IP traffic (to/from strongswan ipsec)", style=dashed];
  EPDG -> GTP_KERN [label="netlink"]
  GTP_KERN -> ATS [label="IP traffic (GTPv1U to/from UPF)", style=dashed]
}
%}
