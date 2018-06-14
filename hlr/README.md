# HLR_Tests.ttcn

* external interfaces
    * GSUP (emulates VLR/SGSN side)
    * VTY

{% dot hlr_tests.svg
digraph G {
  rankdir=LR;
  HLR [label="IUT\nosmo-hlr",shape="box"];
  ATS [label="ATS\nHLR_Tests.ttcn"];

  ATS -> HLR [label="GSUP"];
  ATS -> HLR [label="VTY"];
}
%}
