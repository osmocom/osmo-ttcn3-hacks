# BTS_Tests.ttcn

* external interfaces
    * A-bis side: RSL (emulates BSC-side server)
    * Um side: L1CTL to control MS
    * PCU side: pcu_socket
    * VTY
    * CTRL

{% dot bts_tests.svg
digraph G {
  rankdir=LR;
  { rank=same; BTS, BSC};
  BTS [label="IUT\nosmo-bts-trx",shape="box"];
  ATS [label="ATS\nBTS_Tests.ttcn"];
  BSC [label="osmo-bsc\nOML only"];
  BTS -> fake_trx [label="bursts"];
  fake_trx -> trxcon [label="bursts"];
  trxcon -> ATS [label="GSM MAC blocks"];

  BTS -> BSC [label="A-bis OML"];
  BTS -> ATS [label="A-bis RSL"];

  ATS -> BTS [label="pcu_sock"];
  ATS -> BSC [label="VTY"];
  ATS -> BTS [label="CTRL"];
}
%}
