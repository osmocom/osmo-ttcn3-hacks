# MSC_Tests.ttcn

* external interfaces
    * A: BSSAP/SCCP/M3UA (emulates BSC-side)
    * IuCS: RANAP/SCCP/M3UA (emulates HNBGW-side)
    * MNCC: MNCC/unix-domain (emulates ext. MNCC side)
    * MGW: MGCP (emulates MGW side)
    * GSUP (emulates HLR side)
    * SMPP (emulates ESME side)
    * VTY
    * CTRL

{% dot msc_tests.svg
digraph G {
  rankdir=LR;
  MSC [label="IUT\nosmo-msc",shape="box"];
  ATS [label="ATS\nMSC_Tests.ttcn"];
  STP [label="STP\nosmo-stp"];

  ATS -> MSC [label="MNCC"];
  ATS -> MSC [label="SMPP",style="dashed"];
  ATS -> MSC [label="CTRL"];
  ATS -> MSC [label="VTY"];
  MSC -> ATS [label="GSUP"];
  MSC -> ATS [label="MGCP"];
  ATS -> STP [label="A BSSAP\nSCCP/M3UA"];
  MSC -> STP [label="A BSSAP\nSCCP/M3UA"];
  ATS -> STP [label="IuCS RANAP\nSCCP/M3UA"];
  MSC -> STP [label="IuCS RANAP\nSCCP/M3UA"];
}
%}
