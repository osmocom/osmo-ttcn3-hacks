# SGSN_Tests.ttcn

* external interfaces
    * Gb (emulates PCU side NS/BSSGP + MS)
    * GSUP (emulates HLR)
    * VTY

{% dot sgsn_tests.svg
digraph G {
  rankdir=LR;
  SGSN [label="SGSN\nosmo-sgsn",shape="box"];
  ATS [label="ATS\nSGSN_Tests.ttcn"];

  ATS -> SGSN [label="Gb"];
  SGSN-> ATS [label="Gp (GTP)"];
  ATS -> SGSN [label="VTY"];
}
%}
