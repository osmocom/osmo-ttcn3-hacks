# BSCNAT_Tests.ttcn

* external interfaces
    * BSSAP/SCCP/IPA (SCCPLite) (emulates BSC-side and MSC-side)
    * MGCP/UDP (emulates MSC-side MSC+MGW)
    * MGCP/IPA (emulates BSC-side BSC+MGW)
    * RTP (emulates BSC-side MGW and MSC-side MGW)
    * Osmux (emulates BSC-side MGW)
    * VTY
    * CTRL

{% dot bscnat_tests.svg
digraph G {
  rankdir=LR;
  BSCNAT [label="IUT\nosmo-bsc_nat",shape="box"];
  ATS [label="ATS\nBSC_Tests.ttcn"];

  ATS -> BSCNAT [label="SCCPLite BSC-side"];
  BSCNAT -> ATS [label="SCCPLite MSC-side"];
  BSCNAT -> ATS [label="MGCP/UDP MSC-side", dir="both"];
  BSCNAT -> ATS [label="MGCP/IPA BSC-side"];
  ATS -> BSCNAT [label="CTRL"];
  ATS -> BSCNAT [label="VTY"];
  ATS -> BSCNAT [label="RTP BSC-side", dir="both"];
  ATS -> BSCNAT [label="RTP MSC-side", dir="both"];
  ATS -> BSCNAT [label="Osmux BSC-side", dir="both"];
}
%}
