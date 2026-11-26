# SCP_Tests.ttcn

* external interfaces
    * SBI (emulates NRF)
    * SBI (emulates NFs)
    * N116 (SBI, emulates NWDAF)
    * N117 (SBI, emulates DCCF)

{% dot scp_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nSCP_Tests.ttcn"];
  SCP [label="AMF\open5gs-scpd",shape="box"];

  ATS <- SCP [label="NRF"];
  ATS -> SCP [label="N116 (NWDAF)"];
  ATS -> SCP [label="N117 (DCCF)"];
  ATS -> SCP [label="NF producers/consumers"];
}
%}
