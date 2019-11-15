# MGW_Test.ttcn

* external interfaces
    * MGCP (emulates call agent)
    * RTP (stream source/sink)
    * Osmux (stream source/sink)
    * VTY

{% dot mgw_tests.svg
digraph G {
  rankdir=LR;
  MGW [label="IUT\nosmo-mgw",shape="box"];
  ATS [label="ATS\nMGCP_Test.ttcn"];

  ATS -> MGW [label="RTP", dir="both"];
  ATS -> MGW [label="Osmux", dir="both"];
  ATS -> MGW [label="MGCP"];
  ATS -> MGW [label="VTY"];
}
%}
