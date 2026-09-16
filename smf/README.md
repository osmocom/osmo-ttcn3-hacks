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

  ATS -> SMF [dir="both",label="N4 (PFCP)"];
  ATS -> SMF [dir="both",label="Gx (Diameter)"];
  ATS -> SMF [dir="both",label="Gy (Diameter)"];
  ATS -> SMF [dir="both",label="S6b (Diameter)"];
  #4G:
  ATS -> SMF [label="S5/S8 (GTPv2C)"];
  ATS -> SMF [label="S2b (GTPv2C)"];
  # 5G:
  ATS -> SMF [label="Nsmf (SBI)"];
  SMF -> ATS [label="Namf (SBI, over SCP)"];
  SMF -> ATS [label="Nudm (SBI, over SCP)"];
  SMF -> ATS [label="Npcf (SBI, over SCP)"];
}
%}
