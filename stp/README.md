# STP_Tests.ttcn

* external interfaces
    * M3UA/SCTP (can emulate both client and server side)
    * IPA (can emulate both client and server side)
    * VTY

{% dot sip_tests.svg
digraph G {
  rankdir=LR;
  STP [label="IUT\nosmo-stp",shape="box"];
  ATS [label="ATS\nSTP_Tests.ttcn"];

  ATS -> STP [label="M3UA", dir="both"];
  ATS -> STP [label="IPA", dir="both"];
  ATS -> STP [label="VTY"];
}
%}
