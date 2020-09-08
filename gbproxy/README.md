# GBProxy_Tests.ttcn

* external interfaces
    * Gb (emulates PCU side NS/BSSGP)
    * Gb (emulates SGSN side NS/BSSGP)
    * VTY

{% dot gbproxy_tests.svg
digraph G {
  rankdir=LR;
  GBP [label="IUT\nosmo-gbproxy",shape="box"];
  ATS [label="ATS\nGBProxy_Tests.ttcn"];

  ATS -> GBP [label="Gb (from SGSN)"];
  GBP -> ATS [label="Gb (from PCU)"];
  ATS -> SGSN [label="VTY"];
}
%}
