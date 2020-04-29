# PCU_Tests_RAW.ttcn

* external interfaces
    * Gb (emulates SGSN side NS/BSSGP)
    * unix pcu socket (emulates BTS)
    * VTY

{% dot msc_tests.svg
digraph G {
  rankdir=LR;
  PCU [label="IUT\nosmo-pcu",shape="box"];
  ATS [label="ATS\nPCU_Tests.ttcn"];

  PCU -> ATS [label="Gb"];
  PCU -> ATS [label="pcu_sock"];
  ATS -> PCU [label="VTY"];
}
%}
