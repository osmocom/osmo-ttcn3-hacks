# AUSF_Tests.ttcn

* external interfaces
    * N12: SBI (emulates AMF)
    * N13: SBI (emulates UDM)

{% dot ausf_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nAUSF_Tests.ttcn"];
  UDM [label="AMF\open5gs-ausfd",shape="box"];

  ATS -> UDM [label="N12"];
  ATS -> UDM [label="N13"];
}
%}
