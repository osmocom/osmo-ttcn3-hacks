# NSSF_Tests.ttcn

* external interfaces
    * N22: SBI (emulates AMF)
    * N31: SBI (emulates peer NSSF)
    * N34: SBI (emulates NWDAF)

{% dot nssf_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nNSSF_Tests.ttcn"];
  NSSF [label="AMF\open5gs-nssfd",shape="box"];

  ATS -> NSSF [label="N22"];
  ATS -> NSSF [label="N31"];
  ATS -> NSSF [label="N34"];
}
%}
