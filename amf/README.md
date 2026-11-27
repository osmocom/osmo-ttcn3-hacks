# AMF_Tests.ttcn

* external interfaces
    * NG-C: SCTP/NGAP(emulates gNB or/and g-eNB side) towards AMF
    * SBI: emulates SCP, NRF, UDM, UDR, etc.

{% dot amf_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nAMF_Tests.ttcn"];
  AMF [label="AMF\open5gs-amfd",shape="box"];

  ATS -> AMF [label="NGAP"];
  ATS <-> AMF [label="SBI"];
}
%}
