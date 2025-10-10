# HLR_Tests.ttcn

External interfaces:
* GSUP (emulates VLR/SGSN side)
* VTY / PyHSS REST API client

PyHSS notes:
* Temporary branch that has all necessary patches:
  https://gitea.osmocom.org/osmith/pyhss/src/branch/osmith/ttcn3
* Patches are being upstreamed to:
  https://github.com/nickvsnetworking/pyhss

{% dot hlr_tests.svg
digraph G {
  rankdir=LR;
  HLR [label="IUT\nosmo-hlr",shape="box"];
  ATS [label="ATS\nHLR_Tests.ttcn"];

  ATS -> HLR [label="GSUP"];
  ATS -> HLR [label="VTY"];
}
%}
