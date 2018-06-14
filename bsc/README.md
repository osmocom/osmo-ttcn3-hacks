# BSC_Tests.ttcn

* external interfaces
    * A-bis side: RSL (emulates BTS-side client)
    * A-side: BSSAP/SCCP/M3UA (emulates MSC-side)
    * MGW side: MGCP (emulates MGW side)

{% dot bsc_tests.svg
digraph G {
  rankdir=LR;
  { rank=same; BTS; STP; };
  BSC [label="IUT\nosmo-bsc",shape="box"];
  ATS [label="ATS\nBSC_Tests.ttcn"];
  BTS [label="osmo-bts-omldummy\nOML only"];

  BTS -> BSC [label="A-bis OML"];
  ATS -> BSC [label="A-bis RSL"];
  ATS -> BSC [label="CTRL"];
  ATS -> BSC [label="VTY"];
  ATS -> STP [label="A BSSAP\nSCCP/M3UA"];
  BSC -> STP [label="A BSSAP\nSCCP/M3UA"];
}
%}
