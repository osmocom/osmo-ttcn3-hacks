# BRP (Brainpool) Curve Support Issue in TLS 1.3

## Problem Summary

The SM-DP+ test suite's Brainpool (BRP) test cases are failing due to an incompatibility between TLS 1.3 and Brainpool curves in OpenSSL 3.0.x.

## Technical Details

### Current Environment
- OpenSSL Version: 3.0.13
- TLS Version Required: >= 1.3 (per project requirements)
- Affected Tests: All BRP variants (4 tests)

### Root Cause

1. **TLS 1.3 Specification (RFC 8446)**: Only defines support for NIST curves (P-256, P-384, P-521) and X25519/X448
2. **OpenSSL 3.0.x**: Implements TLS 1.3 strictly per RFC 8446, without Brainpool support
3. **OpenSSL 3.2+**: Adds RFC 8734 support which enables Brainpool curves in TLS 1.3

### Verification

Using `openssl s_client` confirmed the issue:

```bash
# TLS 1.3 with Brainpool fails
openssl s_client -connect localhost:8443 \
    -curves brainpoolP256r1 \
    -tls1_3 \
    -cert sgp26/DPauth/CERT_S_SM_DPauth_ECDSA_BRP.der \
    -key sgp26/DPauth/SK_S_SM_DPauth_ECDSA_BRP.pem

# Result: SSL routines::sslv3 alert handshake failure
```

## Current Test Status

Total Tests: 27
- Passing: 21 (77.78%)
- Failing: 6 (22.22%)
  - 4 BRP tests (SSL handshake failure)
  - 2 other failures

## Solution Options

1. **Upgrade to OpenSSL 3.2+** (Recommended)
   - Provides native RFC 8734 support
   - Enables Brainpool curves in TLS 1.3
   - No code changes required

2. **Use TLS 1.2** (Not viable)
   - Project requirement specifies TLS >= 1.3
   - Would be a security downgrade

3. **Wait for Distribution Update**
   - Ubuntu 24.04 LTS ships with OpenSSL 3.0.13
   - Future updates may include OpenSSL 3.2+

## Impact

- NIST curve tests work perfectly (21/23 passing)
- Only Brainpool tests are affected
- Core functionality is validated with NIST tests