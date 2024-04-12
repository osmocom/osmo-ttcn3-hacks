# PCU_Tests.ttcn

* external interfaces
    * Gb (emulates SGSN side NS/BSSGP)
    * PCUIF: unix pcu socket (emulates BTS)
    * VTY
    * CTRL
    * StatsD

{% dot pcu_tests.svg
digraph G {
  PCU [label="IUT\nosmo-pcu",shape="box", color=red];

  subgraph cluster_ats {
    label = "ATS\n(TTCN-3)";

    system_PCU_PT [label="port system:PCU"];

    subgraph cluster_RAW_PCU_Test_CT {
        label = "RAW_PCU_Test_CT\n(PCU_Tests.ttcn)";

        RAW_PCU_MSG_PT_PCUIF_A [label="port RAW_PCU_MSG_PT PCUIF\n(unused)"]
        PCUVTY [label="PCUVTY"];
        StatsD_ConnHdlr [label="StatsD_ConnHdlr"];
        CTRL_Adapter_CT [label="CTRL_Adapter_CT"];

        test [label="testcasename()\n(PCU_Tests.ttcn)", color=red];

        subgraph cluster_bssgp_CT {
            label="bssgp_CT\nSGSN_Components.ttcn";
            subgraph cluster_BSSGP_Client_CT {
                label="BSSGP_Client_CT\nBSSGP_Emulation.ttcnpp";
                BSSGP_PT [label="port BSSGP_PT BSSGP"];
            }
        }

        subgraph cluster_MS_BTS_IFACE_CT {
            label = "MS_BTS_IFACE_CT\n(GPRS_Components.ttcn)";

            RAW_PCU_MSG_PT_BTS_IFACE [label="port RAW_PCU_MSG_PT BTS"];
        }

    }

    subgraph cluster_RAW_PCUIF_CT {
        label = "RAW_PCUIF_CT\n(PCUIF_Components.ttcn)";

        PCUIF_CODEC_PT_PCU [label="port PCUIF_CODEC_PT PCU"];
        RAW_PCU_MSG_PT_BTS [label="port RAW_PCU_MSG_PT BTS"];
        RAW_PCU_MSG_PT_MTC [label="port RAW_PCU_MSG_PT MTC"];

        PCUIF_CODEC_PT_PCU -> RAW_PCU_MSG_PT_BTS  [dir=none];
        PCUIF_CODEC_PT_PCU -> RAW_PCU_MSG_PT_MTC  [dir=none];

    }

    subgraph cluster_RAW_PCU_BTS_CT {
        label = "RAW_PCU_BTS_CT\n(PCUIF_Components.ttcn)";

        RAW_PCU_MSG_PT_CLCK_A [label="port RAW_PCU_MSG_PT CLCK"];
        RAW_PCU_MSG_PT_PCUIF_B [label="port RAW_PCU_MSG_PT PCUIF"];
        RAW_PCU_MSG_PT_TC [label="port RAW_PCU_MSG_PT TC"];

        subgraph cluster_RAW_PCU_ClckGen_CT {
            label = "RAW_PCU_ClckGen_CT\n(PCUIF_Components.ttcn)";

            RAW_PCU_MSG_PT_CLCK_B [label="port RAW_PCU_MSG_PT CLCK", color=purple];
        }

        RAW_PCU_MSG_PT_CLCK_A -> RAW_PCU_MSG_PT_CLCK_B [dir=back];
        RAW_PCU_MSG_PT_PCUIF_B -> RAW_PCU_MSG_PT_TC [dir=none];
    }


    RAW_PCU_MSG_PT_TC -> RAW_PCU_MSG_PT_BTS_IFACE [dir=none];
    RAW_PCU_MSG_PT_PCUIF_A -> RAW_PCU_MSG_PT_MTC [dir=none];
    RAW_PCU_MSG_PT_PCUIF_B -> RAW_PCU_MSG_PT_BTS [dir=none];

    PCUIF_CODEC_PT_PCU -> system_PCU_PT [dir=none];

    test -> RAW_PCU_MSG_PT_BTS_IFACE  [dir=none];
    test -> RAW_PCU_MSG_PT_PCUIF_A  [label="(unused)", dir=none];
    test -> BSSGP_PT;
    test -> PCUVTY;
    test -> StatsD_ConnHdlr;
    test -> CTRL_Adapter_CT;
  }

  PCU -> BSSGP_PT [label="Gb"];
  PCU -> StatsD_ConnHdlr [label="statsd"];
  system_PCU_PT -> PCU [label="PCUIF"];
  PCUVTY -> PCU [label="VTY"];
  CTRL_Adapter_CT -> PCU [label="CTRL"];
}
%}
