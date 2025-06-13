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

**⚠️ Important implementation Notes**:
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