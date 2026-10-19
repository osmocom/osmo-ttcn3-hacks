# UDM_Tests.ttcn

* external interfaces
    * N8: SBI (emulates AMF)
    * N10: SBI (emulates SMF)
    * N13: SBI (emulates AUSF)
    * N35: SBI (emulates UDR)

{% dot udm_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nUDM_Tests.ttcn"];
  UDM [label="AMF\open5gs-udmd",shape="box"];

  ATS -> UDM [label="N8"];
  ATS -> UDM [label="N10"];
  ATS -> UDM [label="N13"];
  ATS -> UDM [label="N35"];
}
%}
