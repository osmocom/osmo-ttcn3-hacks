# SMF_Tests.ttcn

* external interfaces
    * N11: HTTP2/SBI (emulates AMF side)
    * N4: UDP/PFCP (emulates UPF)
    * Gx: Diameter (emulates PCRF)
    * Gy: Diameter (emulates OCS)
    * S6b: Diameter (emulates AAA)
    * Internet: emulates the Internet side from/to UPF

{% dot smf_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nSMF_Tests.ttcn"];
  SMF [label="AMF\open5gs-smfd",shape="box"];
  NRF [label="SMF\nopen5gs-nrfd"];

  ATS -> SMF [label="N11"];
  ATS -> SMF [label="N4"];
  ATS -> SMF [label="Gx"];
  ATS -> SMF [label="Gy"];
  ATS -> SMF [label="S6b"];
  SMF -> NRF [label="SBI"];
}
%}
