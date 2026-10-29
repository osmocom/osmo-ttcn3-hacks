# BSF_Tests.ttcn

* external interfaces
    * N7: SBI (emulates SMF)
    * N15: SBI (emulates AMF)
    * N36: SBI (emulates UDR)

{% dot pcf_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nBSF_Tests.ttcn"];
  UDM [label="AMF\open5gs-pcfd",shape="box"];

  ATS -> UDM [label="N7"];
  ATS -> UDM [label="N15"];
  ATS -> UDM [label="N36"];
}
%}
