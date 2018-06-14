== MGW_Test.ttcn

* external interfaces
    * MGCP (emulates call agent)
    * RTP (stream source/sink)

{% dot mgw_tests.svg
digraph G {
  rankdir=LR;
  MGW [label="IUT\nosmo-mgw",shape="box"];
  ATS [label="ATS\nMGCP_Test.ttcn"];

  ATS -> MGW [label="RTP"];
  ATS -> MGW [label="MGCP"];
  MGW -> ATS [label="RTP"];
}
%}
