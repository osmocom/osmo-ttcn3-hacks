# SEPP_Tests.ttcn

* external interfaces
    * N32-c: SBI (emulates peer SEPP)
    * NN32-f: SBI (emulates peer SEPP or RI)

{% dot sepp_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nSEPP_Tests.ttcn"];
  SEPP [label="AMF\open5gs-seppd",shape="box"];

  ATS -> SEPP [label="N32-c"];
  ATS -> SEPP [label="N32-f"];
}
%}
