# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the Osmocom TTCN-3 Test Suite repository containing test suites for cellular network elements (2G/3G/4G/5G). The tests are written in TTCN-3 (Testing and Test Control Notation) and compiled with the Eclipse TITAN compiler.

The SM-DP+ (smdpp) test suite specifically validates RSP (Remote SIM Provisioning) protocol implementations according to GSMA SGP.22 specifications.

## Current Status

**Recent Work Completed (December 2024)**:
- ✅ Fixed critical certificate chain validation issue in RSP protocol implementation
- ✅ Implemented proper certificate retrieval using actual certificates instead of PKIDs
- ✅ Enhanced certificate chain validation with intermediate certificate support
- ✅ Test now successfully completes major RSP protocol steps:
  - InitiateAuthentication ✅
  - Authentication response validation ✅ 
  - GetBoundProfilePackage ✅
  - Certificate roles validation ✅
  - EUM certificate permissions ✅
  - Certificate chain discovery ✅

**Known Issues**:
- Certificate name constraints validation may fail in test environments due to strict OpenSSL validation
- This is expected behavior with test certificates and does not indicate RSP protocol issues

## Key Commands

### Building the Test Suite

```bash
# For a specific test suite (e.g., smdpp)
cd smdpp/
./gen_links.sh      # Create symbolic links to dependencies
./regen_makefile.sh # Generate Makefile with proper dependencies
make compile        # Compile TTCN-3 to C++
make all           # Build the executable test suite
```

### Running Tests

```bash
# Run the smdpp test using uns.sh (recommended)
cd smdpp/
../uns.sh  # Runs in isolated namespace with Python server

# Run individual test case manually
cd smdpp/
../start-testsuite.sh smdpp_Tests smdpp_Tests.cfg

# Run specific test case
cd smdpp/
../start-testsuite.sh smdpp_Tests smdpp_Tests.cfg TC_rsp_complete_flow
```

**Note**: The `uns.sh` script runs the test in an isolated network namespace with the Python SM-DP+ server implementation.

### Cleaning

```bash
make clean  # Clean generated files
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

Tests use `testenv.py` which:
- Manages test environment setup
- Handles daemon lifecycle
- Captures logs and packet traces
- Generates JUnit XML results
- Supports podman containerization

## Important Configuration

- SSL/TLS certificates configured in test `.cfg` files
- HTTP debugging controlled via `smdpp_Tests.default`
- Common logging configuration in `Common.cfg`
- Module parameters for specific test behavior

## Development Notes

When modifying tests:
1. TTCN-3 handles protocol encoding/decoding
2. C++ handles cryptographic operations only
3. Always regenerate Makefile after adding new files
4. Test certificates are pre-generated in `sgp26/` directory
5. Use `./uns.sh` for smdpp test execution (preferred over testenv.py)

## Recent Implementation Details

**Certificate Chain Validation Fix**:
- **Problem**: Code was passing PKID (Public Key Identifier) instead of actual certificate for chain validation
- **Solution**: Implemented `ext_RSPClient_getCICertificate()` function to retrieve actual CI certificate
- **Implementation**: Added `ext_CertificateUtil_verifyCertificateChainWithIntermediate()` for proper chain validation with intermediate certificates
- **Files Modified**:
  - `smdpp_Tests.ttcn`: Added external function declarations and updated validation calls
  - `smdpp_Tests_Functions.cc`: Implemented new C++ functions
  - `smdvalcpp2.cpp`: Added `getCICertificate()` method to RSPClient class

**Key Functions Added**:
- `ext_RSPClient_getCICertificate()`: Retrieves actual CI certificate instead of PKID
- `ext_CertificateUtil_verifyCertificateChainWithIntermediate()`: Validates certificate chains with specific intermediate certificates

This ensures proper RSP protocol compliance with certificate chain validation as per GSMA SGP.22 specifications.