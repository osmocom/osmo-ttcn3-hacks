# UDR_Tests.ttcn

* external interfaces
    * N35: SBI (emulates UDM)
    * N36: SBI (emulates PCF)

{% dot udr_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nUDR_Tests.ttcn"];
  UDR [label="AMF\open5gs-udrd",shape="box"];

  ATS -> UDR [label="N35"];
  ATS -> UDR [label="N36"];
}
%}
