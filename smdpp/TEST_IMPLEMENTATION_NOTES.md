# Test Implementation Notes

## Implemented Test Cases

### 1. TC_SM_DP_ES9_InitiateAuthenticationNIST_01_Nominal
- **Purpose**: Tests nominal successful InitiateAuthentication flow
- **Location**: smdpp_Tests.ttcn:1504-1512
- **Handler**: f_TC_InitiateAuth_01_Nominal (lines 958-991)
- **Key Points**: 
  - Reuses existing helper functions from complete flow test
  - Validates server response structure and certificate chain

### 2. TC_SM_DP_ES9_InitiateAuthenticationNIST_02_Uniqueness  
- **Purpose**: Validates transaction IDs and server challenges are unique across calls
- **Location**: smdpp_Tests.ttcn:1516-1524
- **Handler**: f_TC_InitiateAuth_02_Uniqueness (lines 994-1069)
- **Key Points**:
  - Makes 3 calls and stores transaction IDs/challenges
  - Compares all pairs to ensure uniqueness
  - Shows pattern for multi-call test scenarios

### 3. TC_SM_DP_ES9_InitiateAuthenticationNIST_03_InvalidServerAddress
- **Purpose**: Tests JSON error handling for invalid server address
- **Location**: smdpp_Tests.ttcn:1528-1536  
- **Handler**: f_TC_InitiateAuth_03_InvalidServerAddress (lines 1072-1119)
- **Key Points**:
  - Uses new f_perform_es9p_request_expecting_error helper
  - Validates JSON error codes (8.8.1/3.8)
  - Shows pattern for error test cases

## Key Patterns and Reusable Components

### 1. Test Case Structure
```ttcn
testcase TC_TestName() runs on MTC_CT {
    var smdpp_ConnHdlrPars pars := valueof(t_Pars());
    var smdpp_ConnHdlr vc_conn;
    f_init(pars);
    vc_conn := f_start_handler(refers(f_TC_HandlerFunction), pars);
    vc_conn.done;
    setverdict(pass);
}
```

### 2. Handler Function Pattern
```ttcn
private function f_TC_HandlerFunction(smdpp_ConnHdlrPars pars) runs on smdpp_ConnHdlr {
    // Test implementation
    setverdict(pass);
}
```

### 3. Helper Functions Created

#### f_es9p_transceive_wrap (NEW)
- **Purpose**: Unified function that uses f_es9p_send_new for automatic URL routing
- **Returns**: DecodedRPPReponse_Wrap containing either success or error
- **Usage**: Base function for all ES9+ request/response handling

#### f_es9p_transceive_success (NEW)
- **Purpose**: Helper for tests expecting successful responses
- **Returns**: RemoteProfileProvisioningResponse or fails the test
- **Usage**: Replace f_perform_es9p_request_response_new calls

#### f_es9p_transceive_error (NEW)
- **Purpose**: Helper for tests expecting error responses
- **Returns**: JSON_ESx_FunctionExecutionStatusCodeData or fails the test
- **Usage**: Replace f_perform_es9p_request_expecting_error calls

#### f_validate_error_response (NEW)
- **Purpose**: Reusable error code validation
- **Parameters**: actual error, expected subject/reason codes, test description
- **Usage**: One-line error validation in tests

#### f_perform_es9p_request_expecting_error (DEPRECATED)
- **Status**: Deprecated - use f_es9p_transceive_error instead
- **Issue**: Requires manual endpoint specification

#### f_perform_es9p_request_response_new (DEPRECATED)
- **Status**: Deprecated - use f_es9p_transceive_success instead
- **Issue**: Requires manual endpoint specification

### 4. JSON/ASN.1 Dual Layer Handling
- **Success Path**: JSON wrapper → ASN.1 pdu in asn1_pdu field
- **Error Path**: JSON wrapper → error data in err field
- **Never both**: Either asn1_pdu OR err is present, not both

## Important Gotchas and Learnings

### 1. Reserved Words
- "message" is a TTCN-3 reserved word
- Use "message_" with JSON name mapping: `charstring message_ ("message")`

### 2. Error Code Format
- Error codes are strings, not integers (e.g., "8.8.1", not 8.8.1)
- Format: "subjectCode/reasonCode" (e.g., "8.8.1/3.8")
- Must match exactly as specified in testspec.md

### 3. Import Requirements
- Must import esx_header_Types_JSON for error structures
- Must import all _JSON modules for JSON handling
- Check existing imports before adding new ones

### 4. Certificate Handling
- Server certificates come as array in InitiateAuthenticationResponse
- Index [0] is the server certificate
- Certificate validation is already implemented in helper functions

### 5. Endpoint Patterns
- All ES9+ endpoints follow: "/gsma/rsp2/es9plus/{operation}"
- ES2+ endpoints follow: "/gsma/rsp2/es2plus/{operation}"
- ES12 endpoints follow: "/gsma/rsp/es12/{operation}"

## Recommendations for Future Test Implementation

### 1. Before Starting
- Read the complete test flow (f_TC_rsp_complete_flow) to understand the full protocol
- Check if helper functions already exist for the operation
- Review testspec.md for exact error codes and test sequences

### 2. For Success Tests
- Reuse existing helper functions where possible
- Follow the nominal test pattern from TC_01
- Validate all response fields according to spec

### 3. For Error Tests
- Use f_perform_es9p_request_expecting_error
- Match error codes exactly as strings
- Consider both subject and reason codes

### 4. For Complex Flows
- Break down into smaller helper functions
- Use the todo list to track multi-step implementations
- Test incrementally - don't try to implement everything at once

### 5. Common Mistakes to Avoid
- Don't use reserved words as field names
- Don't assume error codes are integers
- Don't forget to import necessary JSON modules
- Don't access optional fields without ispresent() check
- Don't create new certificate loading functions - use existing ones

### 6. Important Note on Response Structure Checking
When using `f_es9p_transceive_success`, you don't need to check `ispresent()` on the response structure fields because:
- If `wrap.asn1_pdu` exists (checked internally), the response structure is guaranteed to be valid
- The function fails the test if the response isn't successful
- Direct field access like `authResponse.initiateAuthenticationResponse.initiateAuthenticationOk` is safe

This is different from the old pattern where manual validation was needed.

### 7. Note on Optional Field Access
For optional fields in TTCN-3:
- **General rule**: Use `ispresent()` before accessing optional fields
- **Exception**: If the field behavior is well-understood and the access is non-critical (e.g., logging after test has passed)
- **Example**: `errorData.message_` in error responses - if it's always present (even as empty string) or if absence doesn't cause issues, the check can be omitted
- **Be consistent**: Once you determine a field's behavior, apply the same pattern throughout

## Test Execution Notes
- Always use ./uns.sh for running tests (provides Python server)
- Check pyserver.log for server-side debugging
- Check merged.log for detailed TTCN-3 execution traces
- Build with: cd smdpp; make clean; make compile; make -j