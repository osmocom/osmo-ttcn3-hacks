# SGSN_Tests.ttcn

* external interfaces
    * Gb (emulates PCU side NS/BSSGP + MS)
    * Iu (emulates HNBGW side IuPS)
    * Gp: GTP (emulates GGSN)
    * GSUP (emulates HLR)
    * VTY

{% dot sgsn_tests.svg
digraph G {
  rankdir=LR;
  SGSN [label="IUT\nosmo-sgsn",shape="box"];
  ATS [label="ATS\nSGSN_Tests.ttcn"];
  STP [label="STP\nosmo-stp"];

  ATS -> SGSN [label="Gb"];
  SGSN-> ATS [label="Gp (GTP)"];
  SGSN -> STP [label="Iu (IuPS)"];
  ATS -> STP [label="Iu (IuPS)"];
  SGSN -> ATS [label="GSUP"];
  ATS -> SGSN [label="VTY"];
}
%}
