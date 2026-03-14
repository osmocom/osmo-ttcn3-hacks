# What is this?

This is a TTCN-3 testsuite for osmo-s1gw.

## What is osmo-s1gw?

**OsmoS1GW** is an S1AP gateway (proxy) between eNBs and an MME.
It terminates S1AP SCTP connections from eNBs on one side, forwards
S1AP to/from the MMEs on the other, and coordinates user-plane bearer
setup with a UPF via PFCP.  It is implemented in Erlang.

```
eNB --[S1AP/SCTP]--> S1GW --[S1AP/SCTP]--> MME
                      |
                   [PFCP/UDP]
                      |
                     UPF
```

Network address layout in tests:
- eNB-facing: S1GW=127.0.1.1, eNB(s)=127.0.1.10+
- MME-facing: S1GW=127.0.2.1, MME=127.0.2.10+
- UPF-facing: S1GW=127.0.3.1, UPF=127.0.3.10
- StatsD: 127.0.4.10:8125, REST: 127.0.0.1:8080

## Running the testsuite

```bash
# From repo root:
./testenv.py run s1gw

# Build only:
make s1gw

# Run manually (osmo-s1gw must already be running):
cd s1gw
../start-testsuite.sh s1gw/S1GW_Tests S1GW_Tests.cfg

# Single test case:
../start-testsuite.sh s1gw/S1GW_Tests S1GW_Tests.cfg S1GW_Tests.TC_e_rab_setup
```

## Module Overview

| File | Purpose |
|---|---|
| `S1GW_Tests.ttcn` | Top-level test cases, `f_init_*`, control block |
| `S1GW_ConnHdlr.ttcn` | Per-eNB connection handler; all protocol helper functions |
| `S1AP_Server.ttcn` | MME emulator; routes S1AP PDUs to ConnHdlr subscribers |
| `S1GW_UEMux.ttcn` | UE multiplexer for multi-UE tests (128 concurrent UEs, by default) |
| `S1GW_REST_Types.ttcn` | JSON/REST data types for management API |
| `S1GW_REST_Functions.ttcn` | REST helper functions (metrics, PFCP, MME/E-RAB mgmt) |

## Architecture & Key Patterns

### Component Hierarchy

```
test_CT  (extends StatsD_Checker_CT, http_CT)
  ├── S1AP_Server_CT    (MME emulator, one instance)
  ├── PFCP_Emulation_CT (UPF emulator, one instance)
  └── ConnHdlr [0..N]   (one per emulated eNB)
        └── UEMux_CT / UEMuxUE_CT  (only in TC_uemux_* tests)
```

`test_CT` starts in `f_init()`, which brings up the REST interface,
S1AP server, PFCP emulation, and StatsD checker before spawning any
ConnHdlr instances.

### ConnHdlr Lifecycle

Every test case follows this lifecycle via `f_TC_exec()`:

```
f_ConnHdlr_spawn()
  → f_ConnHdlr_s1ap_connect()     # TCP/SCTP connect to S1GW eNB-facing port
  → f_ConnHdlr_s1ap_register()    # register with S1AP_Server
  → f_ConnHdlr_s1ap_setup()       # S1AP SetupRequest/Response exchange
  → [test body runs]
  → f_ConnHdlr_s1ap_disconnect()
  → f_ConnHdlr_s1ap_unregister()
```

Multi-eNB tests spawn N ConnHdlr instances, run them in parallel
via `interleave`, then join.

### E-RAB Parameter Tracking (`ERab` / `ERabList`)

Each bearer is tracked with transport endpoints for all four forwarding
directions (access↔core in both send/receive), plus UPF PFCP SEIDs and
TEID/IP combos.  These are populated during setup and then asserted
during modification/release to verify S1GW correctly translates addresses.

```ttcn
type record ERab {
    ERabParams u2c,   -- UPF→Core side (what S1GW programs into UPF)
    ERabParams c2u,   -- Core→UPF side
    ERabParams a2u,   -- Access→UPF side
    ERabParams u2a,   -- UPF→Access side
    ERabParams u2cm,  -- modified variant
    ERabParams u2am,  -- modified variant
    OCT8 pfcp_loc_seid,
    OCT8 pfcp_rem_seid
}
```

### PFCP Session Mutex

`f_ConnHdlr_erab_setup_req()` acquires a mutex before establishing the
PFCP session.  This serialises concurrent E-RAB setups across eNBs
because the initial PFCP trigger uses SEID=0, which cannot be
unambiguously routed without locking.

### Bidirectional PDU Helpers

All S1AP send/receive functions come in two directional variants:

- `f_ConnHdlr_tx_s1ap_from_enb()` / `f_ConnHdlr_rx_s1ap_from_enb()` — eNB -> S1GW -> MME direction
- `f_ConnHdlr_tx_s1ap_from_mme()` / `f_ConnHdlr_rx_s1ap_from_mme()` — MME -> S1GW -> eNB direction

Similarly for PFCP: `f_ConnHdlr_pfcp_expect()`, `f_ConnHdlr_pfcp_reply()`.

### Test Case Parameterisation

Test bodies are passed as function references to `f_TC_exec()`, which handles ConnHdlr
spawn/teardown. This lets the same test body run with varying eNB count or E-RAB count
just by changing parameters — e.g. `TC_e_rab_setup` and `TC_e_rab_setup_multi` share
`f_TC_e_rab_setup`.

### StatsD Validation

Many tests call `f_statsd_expect()` at the end to assert S1GW exported the right metrics
(connection counts, forwarded packet counters, PFCP association state). These expectations
are declared with `StatsDExpect` records before the test body runs and checked after.

### REST Interface

`S1GW_REST_Functions.ttcn` wraps HTTP GET/POST calls for inspecting and controlling live
S1GW state (PFCP association, MME pool, E-RAB list).  Used both for supplementary assertions
(e.g. verifying StatsD metrics are consistent with REST state) and as the primary mechanism
in REST-focused test cases.

## Test Case Groups

| Group | What's Covered |
|---|---|
| Connection | eNB connect/disconnect, MME-initiated teardown, MME unavailable |
| E-RAB | Bearer setup/release/modify in both directions, PFCP rejection |
| Initial Context | Attach bearer setup, UE context lifecycle |
| UE Mux | Concurrent multi-UE operation (128 UEs) via UEMux |
| Handover | X2/S1 handover preparation, resource allocation, partial failure |
| PFCP | UPF heartbeat exchange |
| MME Pool | MME fallback on rejection/timeout, impatient eNB disconnect during pool selection |
| REST | MME pool listing, runtime add/delete, fallback after runtime delete |
