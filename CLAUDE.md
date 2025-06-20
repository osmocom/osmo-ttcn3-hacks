# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the Osmocom TTCN-3 Test Suite repository containing test suites for cellular network elements (2G/3G/4G/5G). The tests are written in TTCN-3 (Testing and Test Control Notation) and compiled with the Eclipse TITAN compiler.

The SM-DP+ (smdpp) test suite specifically validates RSP (Remote SIM Provisioning) protocol implementations according to GSMA SGP.22 specifications.

## Key Commands

### Building the Test Suite
- YOU MUST NOT run make in /app/tt-smdp !
- YOU MUST run make in subdirs. never at the top!

```bash
# For a specific test suite (e.g., smdpp)
cd smdpp/

# Initial setup (only needed once or when dependencies change)
cd smdpp; ./gen_links.sh      # Create symbolic links to dependencies
cd smdpp; ./regen_makefile.sh # Generate Makefile with proper dependencies

# Full clean build (recommended)
cd smdpp; make clean; make compile; make -j

```

**⚠️ Important Build Notes**:
- **Always use `make clean; make compile; make -j` for clean builds**
- **Never use manual `rm` commands - always use `make clean`**
- **Don't run `make clean` frequently as it takes time - use incremental `make -j` for minor changes**
- Build process can take several minutes for full clean builds

**⚠️ Important Implementation Notes**:
- Always clean before make, because not doing that produces weird errors like smdpp_Tests_part_5.cc:364:15: error: 'init_ttcn3_debugger' is not a member of 'smdpp__Tests and those are a waste of time fixed by clean builds.
- ** c++ functions used by ttcn3 always have two underscores in the name, example:**
- ttcn3: external function ext_logInfo(charstring xmessage);
- c++: void ext__logInfo(const CHARSTRING& message)

### Running Tests

```bash
# From the root tt-smdpp directory
./uns.sh  # Runs in isolated namespace with Python server (MUST BE USED)
```

**Test Execution Notes**:
- **Always use `./uns.sh` for smdpp tests** - it provides isolated network namespace and Python server
- The Python SM-DP+ server is automatically started and configured
- Test typically takes 2-5 seconds to complete
- Final verdict should be: **pass** ✅

### Log Files and Debugging

```bash
# Main test log (after running ./uns.sh)
cat /app/tt-smdpp/smdpp/merged.log

# Python server log
cat /app/tt-smdpp/smdpp/pyserver.log

```

### Cleaning

```bash
cd smdpp/
make clean  # Clean generated files (takes time - use sparingly)
```

## Architecture

### Test Suite Structure

Each test suite follows this pattern:
- `<suite>_Tests.ttcn` - Main test module with test cases
- `<suite>_Tests.cfg` - Test configuration file
- `<suite>_Tests.default` - Default parameters
- `gen_links.sh` - Creates symbolic links to library dependencies
- `regen_makefile.sh` - Regenerates Makefile

### SM-DP+ Test Architecture

The smdpp test suite implements a complete RSP protocol flow:

1. **Component Architecture**:
   - `MTC_CT` - Main Test Component orchestrates the test
   - `smdpp_ConnHdlr` - Connection handler for individual test scenarios
   - HTTP server emulation for ES12 and ES2+ interfaces

2. **External C++ Functions**:
   - Cryptographic operations (ECDH, signing, verification)
   - Certificate handling and validation
   - Session key derivation
   - Located in `smdpp_Tests_Functions.cc`

3. **Protocol Flow**:
   - Certificate chain validation (now working with proper certificate chain: eUICC → EUM → CI)
   - ECDH key agreement
   - Profile package download and verification
   - Secure channel establishment

4. **Certificate Architecture**:
   - CI (Certificate Issuer) certificates in `sgp26/CertificateIssuer/`
   - EUM (eUICC Manufacturer) certificates in `sgp26/EUM/`
   - eUICC certificates in `sgp26/eUICC/`
   - SM-DP+ certificates in `sgp26/DPauth/` and `sgp26/DPpb/`
   - **Important**: Certificates remain in separate directories per their roles

### Key Dependencies

- OpenSSL for cryptographic operations
- libcjson/jansson for JSON parsing
- libcurl for HTTP client operations
- Test certificates in `sgp26/` directory

## Test Execution Environment
- **Always use `./uns.sh` for smdpp test execution** (required for proper operation)
- Test runs in isolated network namespace with Python SM-DP+ server

**Test Environment Components**:
1. **TTCN-3 Test Client**: Implements eUICC side of RSP protocol
2. **Python SM-DP+ Server**: Implements server side with proper BSP key derivation
3. **Network Isolation**: Uses `uns.sh` for clean network namespace
4. **Certificate Infrastructure**: Complete test certificate chain (CI → EUM → eUICC, DPauth, DPpb)
5. **Cryptographic Validation**: Full session key derivation and usage

## Important Configuration

- SSL/TLS certificates configured in test `.cfg` files
- HTTP debugging controlled via `smdpp_Tests.default`
- Common logging configuration in `Common.cfg`
- Module parameters for specific test behavior

## Development Notes

**Current Implementation Status**: **Complete and Working** ✅

When modifying tests:
1. **TTCN-3 handles protocol encoding/decoding** - ASN.1 structures and JSON
2. **C++ handles cryptographic operations** - ECDH, session keys, MAC verification, encryption
3. **Always regenerate Makefile after adding new files** - Use `./regen_makefile.sh`
4. **Test certificates are pre-generated** in `sgp26/` directory
5. **Always use `./uns.sh` for smdpp test execution** - Required for proper Python server integration
6. **Session keys are fully implemented** - Both GlobalPlatform SCP03 and BSP key derivation working
7. **BSP validation handles both test and encrypted data** - Intelligent detection of data types

**Debugging Tips**:
- Check session key derivation logs for S-ENC, S-MAC, and BSP key values
- Verify Python server logs show proper BSP key derivation
- Use `merged.log` for detailed test execution traces

## Additional Implementation Details

### BSP (Bound Session Protocol) Integration
- BSP crypto functionality extracted from `bsp_python_bindings.cpp` into separate C++ modules
- Key functions: `from_kdf` for session key derivation, `process_bound_profile_package` for decryption/verification
- Session keys derived using X9.63 KDF with shared secret from ECDH
- Supports both session keys (S-ENC, S-MAC) and Profile Protection Keys (PPK) via ReplaceSessionKeys

### PIN Code / Confirmation Code Handling
- Implements SGP.22 confirmation code verification: `SHA256(SHA256(CC) | TransactionID)`
- Confirmation codes treated as hex strings (like EID) - converted to bytes before hashing
- Test detects `ccRequiredFlag` in authenticateClient response and includes hash in PrepareDownloadRequest
- Both Python server and C++ client implement identical hash computation

### Data Encoding Conventions
- EID: Decimal string of digits interpreted as hex bytes (e.g., "89049032..." → [0x89, 0x04, 0x90, 0x32, ...])
- Confirmation Code: Hex string interpreted as bytes (e.g., "12345678" → [0x12, 0x34, 0x56, 0x78])
- Transaction ID: Already in binary format (octetstring in ASN.1)

## Test Specification Reference (SGP.23)
Test cases in `/app/testspec.md` follow SGP.23 v1.15 standard:
- **Naming**: `TC_Component_Interface.OperationVariant` (e.g., TC_SM-DP+_ES9+.InitiateAuthenticationNIST)
- **Structure**: Initial Conditions → Test Sequences → Step-by-step validation
- **Test Types**: Nominal (success), Error cases, Retry scenarios, Confirmation code tests
- **Message Notation**: `#` for constants, `<>` for dynamic values, MTD_HTTP_REQ/RESP for HTTP
- **Error Format**: "Subject Code X.X.X Reason Code Y.Y" (e.g., "8.2.1 3.7" for missing confirmation code)

## Implementation Documentation
- **Test Implementation Notes**: `/app/tt-smdpp/smdpp/TEST_IMPLEMENTATION_NOTES.md` - Patterns and guidelines for implementing new tests
- **JSON Error Handling**: `/app/tt-smdpp/smdpp/JSON_ERROR_HANDLING.md` - How JSON/ASN.1 dual-layer error handling works
- **Implementation Summary**: `/app/tt-smdpp/smdpp/IMPLEMENTATION_SUMMARY.md` - Summary of implemented InitiateAuthentication tests
- **ispresent() Guidelines**: `/app/tt-smdpp/smdpp/ISPRESENT_GUIDELINES.md` - When to use or omit ispresent() checks

## Memory Notes

### Testing and Execution Strategies
- Use tasks and workers to ensure your context does not get polluted by the expansive code and the massive log output these tests generate.