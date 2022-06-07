# UPF_Tests.ttcn

* external interfaces
    * PFCP
    * VTY
    * CTRL
    * StatsD

{% dot upf_tests.svg
digraph G {
  graph [label="UPF_Tests", labelloc=t, fontsize=30];
  rankdir=LR;
  UPF [label="IUT\nosmo-upf",shape="box"];
  ATS [label="ATS\nUPF_Tests.ttcn"];

  UPF <- ATS [label="PFCP"];
  UPF <- ATS [label="CTRL"];
  UPF <- ATS [label="VTY"];
  UPF -> ATS [label="StatsD"];
}
%}
