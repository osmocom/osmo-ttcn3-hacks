# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the Osmocom TTCN-3 Test Suite repository containing test suites for cellular network elements (2G/3G/4G/5G). The tests are written in TTCN-3 (Testing and Test Control Notation) and compiled with the Eclipse TITAN compiler.

The SM-DP+ (smdpp) test suite specifically validates RSP (Remote SIM Provisioning) protocol implementations according to GSMA SGP.22 specifications.

## Key Commands

### Building the Test Suite
- YOU MUST NOT run make in /app/tt-smdpp !
- YOU MUST run make in subdirs. never at the top!

```bash
# For a specific test suite (e.g., smdpp)
cd smdpp/

# Initial setup (only needed once or when dependencies change)
cd smdpp; ./gen_links.sh      # Create symbolic links to dependencies
cd smdpp; ./regen_makefile.sh # Generate Makefile with proper dependencies

# Full clean build (recommended)
cd smdpp; make clean; make compile; make -j

# Generate compile_commands.json for language servers and IDEs
cd smdpp; make bear              # Full clean build with bear
cd smdpp; make bear-incremental  # Update compile_commands.json without clean

```

**⚠️ Important Build Notes**:
- **Always use `make clean; make compile; make -j` for clean builds**
- **Never use manual `rm` commands - always use `make clean`**
- **Don't use incremental `make -j` for minor changes, always clean, incremental only causes weird compiler issues and wastes time**
- Build process can take a few seconds for full clean builds

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

## Critical Implementation Notes

### Certificate Loading (CRITICAL)
- **RSPClientRegistry MUST load both certificates AND key pairs**:
  - `client->loadEUICCCertificate()` and `client->loadEUICCKeyPair()`
  - `client->loadEUMCertificate()` and `client->loadEUMKeyPair()`
- Missing certificate loading causes "TLV is not complete" BER decoding errors
- Certificates are loaded from `./sgp26/` subdirectories

### Port Handling
- Use `CURLOPT_PORT` to set port separately, don't embed in URL
- BRP tests run on port 8001, NIST tests on port 8000
- Session databases use suffix: `sm-dp-sessions-NIST` and `sm-dp-sessions-BRP`

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
- For "TLV is not complete" errors, check certificate loading in RSPClientRegistry

## Known Limitations

### BRP (Brainpool) Tests
**BRP tests are incompatible with TLS 1.3** - This is a fundamental protocol limitation:
- TLS 1.3 (RFC 8446) only supports specific curves: secp256r1, secp384r1, secp521r1, x25519, x448
- Brainpool curves (brainpoolP256r1, etc.) are NOT supported in TLS 1.3
- Since TLS >= 1.3 is required, BRP tests cannot function
- This is not fixable through patches or configuration changes

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
- **Test Implementation Status**: `/app/tt-smdpp/smdpp/TEST_IMPLEMENTATION_STATUS.md` - Current test implementation status and progress tracking
- **Test Case Analysis**: `/app/tt-smdpp/smdpp/TEST_CASE_ANALYSIS.md` - Detailed analysis of test cases with line references to testspec.md
- **JSON Error Handling**: `/app/tt-smdpp/smdpp/JSON_ERROR_HANDLING.md` - How JSON/ASN.1 dual-layer error handling works
- **ispresent() Guidelines**: `/app/tt-smdpp/smdpp/ISPRESENT_GUIDELINES.md` - When to use or omit ispresent() checks

## Memory Notes

### Testing and Execution Strategies
- Use tasks and workers to ensure your context does not get polluted by the expansive code and the massive log output these tests generate.
- Always use task workers to run the full test suite so you don't pollute your context
- Always make a plan, and use tasks and worker thingies to preserve your context, because this codebase is large.

### Testspec File Locations
- Full path of testspec: `/app/testspec.md`

### Test Case Analysis Reference
- Take note of previous test case analysis at `/app/tt-smdpp/smdpp/TEST_CASE_ANALYSIS.md` - reference this file for specific test case numbers and details to help locate and understand specific test scenarios

### Test Running Strategies
- You can and should run single tests most of the time like this: ./uns.sh smdpp_Tests.TC_SM_DP_ES9_CancelSession_After_AuthenticateClientNIST

## TTCN-3 Language Patterns and Best Practices

### Error Injection Framework Pattern
When implementing error test cases, use a generic function with error injection:

```ttcn
/* Define union type for different error scenarios */
type union AuthClientErrorInjection {
    CertificateError cert_error,
    SignatureError sig_error,
    TransactionIdError trans_error
}

/* Generic test function accepting error injection */
private function f_TC_AuthenticateClient_Error_Generic(
    charstring test_name,
    charstring expected_subject_code,
    charstring expected_reason_code,
    AuthClientErrorInjection error_injection
) runs on smdpp_ConnHdlr
```

This pattern enables:
- Reusable test logic for multiple error scenarios
- Type-safe error injection
- Clear test intent through descriptive injection types
- Consolidation of 20+ individual tests into 1 generic function

### Union Type Usage
TTCN-3 unions work like C unions or Rust enums:
- Use `ischosen()` to check which variant is active
- Only one variant can be active at a time
- Example: `if (ischosen(error_injection.sig_error)) { ... }`

### External Function Integration
TTCN-3 to C++ function mapping uses double underscores:
- TTCN-3: `external function ext_someFunction()`
- C++: `void ext__someFunction()` (note the double underscore)
- Parameters and return types must match exactly
- Use `enc_*/dec_*` naming convention for encode/decode functions

### Test Consolidation Strategy
Instead of implementing individual test cases for each error scenario:
- Create consolidated test cases that run multiple scenarios
- Use descriptive logging to identify which scenario is running
- Example: `TC_SM_DP_ES9_AuthenticateClientNIST_ErrorCases` covers 4 error scenarios

## HTTP Response Handling

### HTTP 204 No Content
HandleNotification endpoint returns HTTP 204 with no body on success:
```ttcn
} else if (resp.response.statuscode == 204) {
    /* HTTP 204 No Content - success with no body */
    es9p_resp.emptyResponse := {
        header := { functionExecutionStatus := { status := "Executed-Success" } }
    };
    es9p_resp.err := omit;
    return es9p_resp;
}
```

## Certificate and Cryptographic Variants

### Certificate Type Clarifications
- **NIST**: Uses NIST P-256 elliptic curves (default)
- **BRP**: Brainpool curves - same tests with different certificates
- **FRP**: Marked as FFS (For Further Study) - not applicable for current version

### ASN.1 Encoding Important Notes
- **Object Identifiers**: Must use numeric form: `objid { 2 999 10 }`
- **Signature Concatenation**: Raw concatenation without TLV wrappers
- **Order Matters**: eUICC data first, then SM-DP+ signature

## Error Validation Pattern
Centralize error validation for consistency:
```ttcn
private function f_validate_error_response(
    JSON_ESx_FunctionExecutionStatusCodeData errorData,
    charstring expected_subject_code,
    charstring expected_reason_code,
    charstring test_description
)
```

## Python Server State Management
For retry scenarios, the server maintains global state:
```python
# Global mapping for retry scenarios
self.otpk_mapping = {}  # Maps euicc_otpk -> (smdp_ot, smdp_otpk, timestamp)
self.otpk_timeout = 86400  # 24 hours

# Cleanup task using Twisted
self.cleanup_task = task.LoopingCall(self.cleanup_expired_otpk_mappings)
self.cleanup_task.start(3600)  # Hourly cleanup
```


## Memory Notes

### File and Code Handling
- NEVER read the *_part*.cc or h files, those are generated by ttcn3, are very large, and never contain any useful information
- NEVER manually rm -rf files because you don't know what you are doing

### Build Practices
- Always run make clean first, there is no point running into the usual arcane debugger error all the time.
### Specification File Location
- the full specification is available at `/app/sgp22_2.5.md` WARNING it is a very large file you must be selective when reading it
### MCP Restart Guidelines
- **you can't actually restart the mcp, you MUST tell the user to please restart it.**

### dir change
- cd to dirs might not work, try to use pushd and popd

### MCP and LSP Global Effects
- keep in mind that the mcp is the same for all languages, only the lsp behind it is different, so if you change it and then try to investigate a different language and the mcp behaves weird or unexpected it might be caused by your changes that had a global effect

### Workers and Task Handling
- Prefer parallel worker thingies if the problem is appropriate,  that means if the task is not too complicated.
- You MUST use worker thingies to look at compiler output, log output, or test output, because it is a lot of data.
- The only exception is that there must be a very good reason why you need to look at it yourself.

### MCP Usage Guidelines
- **make sure to prefer using the existing mcps that are available**

### Environment Sourcing
- **you must source /app/env.sh to be able to use the ttcn3 compiler**

# Markdown Grep Notes
- you **MUST** use proper escaping when reading markdown files.
- this fails: grep "INVALID\_MATCHING\_ID" testspec.md
- this fails, too: grep "INVALID\\_MATCHING\\_ID" testspec.md
- but this works: grep "INVALID\\\_MATCHING\\\_ID" testspec.md
