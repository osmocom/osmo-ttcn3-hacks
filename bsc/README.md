# BSC_Tests.ttcn

* external interfaces
    * A-bis side: RSL (emulates BTS-side client) (OML handled by osmo-bts-omldummy)
    * A-side (emulates MSC-side)
        * BSSAP/SCCP/M3UA (AoIP)
        * BSSAP/SCCP/IPA (SCCPLite)
    * MGW side: MGCP (emulates MGW side)
    * VTY
    * CTRL
    * StatsD

{% dot bsc_tests.svg
digraph G {
  graph [label="AoIP", labelloc=t, fontsize=30];
  rankdir=LR;
  { rank=same; BTS; STP; };
  BSC [label="IUT\nosmo-bsc",shape="box"];
  ATS [label="ATS\nBSC_Tests.ttcn"];
  BTS [label="osmo-bts-omldummy\nOML only"];

  BTS -> BSC [label="A-bis OML"];
  ATS -> BSC [label="A-bis RSL"];
  ATS -> BSC [label="CTRL"];
  ATS -> BSC [label="VTY"];
  BSC -> ATS [label="StatsD"];
  ATS -> STP [label="A BSSAP\nSCCP/M3UA"];
  BSC -> STP [label="A BSSAP\nSCCP/M3UA"];
}
%}

{% dot bsc_tests_sccplite.svg
digraph G {
  graph [label="SCCPLite", labelloc=t, fontsize=30];
  rankdir=LR;
  BSC [label="IUT\nosmo-bsc",shape="box"];
  ATS [label="ATS\nBSC_Tests.ttcn"];
  BTS [label="osmo-bts-omldummy\nOML only"];

  BTS -> BSC [label="A-bis OML"];
  ATS -> BSC [label="A-bis RSL"];
  ATS -> BSC [label="CTRL"];
  ATS -> BSC [label="VTY"];
  BSC -> ATS [label="StatsD"];
  ATS -> BSC [label="A BSSAP\nSCCP/IPA"];
}
%}
