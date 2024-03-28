* Asterisk_Tests.ttcn

* external interfaces
    * SIP (emulates SIP UAs)
    * VoLTE (emulates IMS server)

{% dot sip_tests.svg
digraph G {
  rankdir=LR;
  Asterisk [label="IUT\nAsterisk",shape="box"];
  ATS [label="ATS\nAsterisk_Tests.ttcn"];

  ATS -> Asterisk [label="SIP"];
  ATS -> Asterisk [label="VoLTE (IMS)"];
}
%}
