# EIR_Tests.ttcn

* external interfaces
    * N17: SBI (emulates AMF)

{% dot eir_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nEIR_Tests.ttcn"];
  UDM [label="AMF\open5gs-eird",shape="box"];

  ATS -> UDM [label="N17"];
}
%}
