* SIP_Tests.ttcn

* external interfaces
    * MNCC (emulates MSC side)
    * SIP (emulates SIP switch)
    * VTY

{% dot sip_tests.svg
digraph G {
  rankdir=LR;
  SIP [label="IUT\nosmo-sip-connector",shape="box"];
  ATS [label="ATS\nSIP_Tests.ttcn"];

  ATS -> SIP [label="MNCC"];
  ATS -> SIP [label="SIP"];
  ATS -> SIP [label="VTY"];
}
%}
