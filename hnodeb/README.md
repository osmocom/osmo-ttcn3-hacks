# HNB_Tests.ttcn

* external interfaces
    * Iuh side (emulates HNBGW-side)
        * SCTP/HNBAP
        * SCTP/RUA/RANAP
    * RTP side: emulates MGW
    * GTP-U side: emulates GGSN
    * UE side
    * VTY
    * CTRL
    * StatsD

{% dot hnb_tests.svg
digraph G {
  graph [label="HNB_Tests", labelloc=t, fontsize=30];
  rankdir=LR;
  { rank=same; ATS; HNB; };
  HNB [label="IUT\nosmo-bsc",shape="box"];
  ATS [label="ATS\nHNB_Tests.ttcn"];

  HNB <- ATS [label="Uu (or some intermediate lower layer)"];
  HNB -> ATS [label="Iuh"];
  HNB -> ATS [label="RTP"];
  HNB -> ATS [label="GTP-U"];
  HNB <- ATS [label="CTRL"];
  HNB <- ATS [label="VTY"];
  HNB -> ATS [label="StatsD"];
}
%}
