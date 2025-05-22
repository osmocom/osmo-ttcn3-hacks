# C5G_Tests.ttcn

* external interfaces
    * NG-C: SCTP/NGAP(emulates gNB or/and g-eNB side) towards AMF
    * NG-U: UDP/GTPv1U (emulates gNB or/and g-eNB side) towards UPF
    * Internet: emulates the Internet side from/to UPF

{% dot 5gc_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nC5G_Tests.ttcn"];
  AMF [label="AMF\open5gs-amfd",shape="box"];
  SMF [label="SMF\nopen5gs-smfd"];
  UPF [label="UPF\nopen5gs-upfd"];

  ATS -> AMF [label="NGAP"];
  ATS -> UPF [label="GTPv1U",style="dashed"];
  AMF -> SMF [label="GTPv2C"];
  SMF -> UPF [label="PFCP"];
}
%}
