# NRF_Tests.ttcn

* external interfaces
    * N27: SBI (emulates NFs or other NRF)

{% dot nrf_tests.svg
digraph G {
  rankdir=LR;
  ATS [label="ATS\nNRF_Tests.ttcn"];
  NRF [label="AMF\open5gs-nrfd",shape="box"];

  ATS -> NRF [label="N27"];
}
%}
