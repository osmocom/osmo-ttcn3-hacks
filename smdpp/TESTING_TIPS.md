# Testing Tips for TTCN-3 Test Development

## Running Individual Tests

Instead of running all tests (which takes longer), you can run specific tests:

```bash
# Run a single test by name
cd /app/tt-smdpp
./uns.sh smdpp.TC_SM_DP_ES9_AuthenticateClientNIST_02_ConfirmationCode

# Or multiple specific tests
./uns.sh smdpp.TC_SM_DP_ES9_AuthenticateClientNIST_01_Nominal smdpp.TC_SM_DP_ES9_AuthenticateClientNIST_02_ConfirmationCode
```

## Benefits of Running Individual Tests

1. **Faster feedback loop** - Single test runs in ~2-3 seconds vs ~10+ seconds for all tests
2. **Focused debugging** - Easier to see specific error messages
3. **Iterative development** - Fix one issue at a time
4. **Less log pollution** - Smaller log files to analyze

## Test Naming Convention

Tests follow the pattern: `smdpp.TC_<TestName>`

Examples:
- `smdpp.TC_rsp_complete_flow`
- `smdpp.TC_SM_DP_ES9_InitiateAuthenticationNIST_01_Nominal`
- `smdpp.TC_SM_DP_ES9_AuthenticateClientNIST_03_Mismatched_Transaction_ID`

## Workflow for Fixing Tests

1. Run the specific failing test
2. Check logs (merged.log and pyserver.log)
3. Make fixes
4. Recompile if needed: `make -j` (no need for clean if only minor changes)
5. Run the same test again
6. Repeat until test passes
7. Run all tests once at the end to ensure no regressions

## Quick Commands

```bash
# Run one test
./uns.sh smdpp.TC_<TestName>

# Check last test result quickly
tail -50 smdpp/merged.log

# Check server errors
tail -50 smdpp/pyserver.log

# Quick recompile (no clean)
cd smdpp && make -j
```