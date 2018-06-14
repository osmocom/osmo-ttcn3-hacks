# GGSN_Tests.ttcn

* external interfaces
    * Gp: GTP (emulates SGSN)
    * Gi: IP (emulates Internet)

{% dot ggsn_tests.svg
digraph G {
  rankdir=LR;
  GGSN [label="GGSN\nosmo-ggsn",shape="box"];
  ATS [label="ATS\nGGSN_Tests.ttcn"];

  ATS -> GGSN [label="Gp (GTP)"];
  GGSN -> ATS [label="Gi (IP)"];
  ATS -> GGSN [label="VTY"];
}
%}
