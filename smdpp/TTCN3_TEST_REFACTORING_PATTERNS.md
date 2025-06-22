# TTCN-3 Test Refactoring Patterns

## Overview
This document describes the refactoring patterns applied to TTCN-3 test functions to reduce code duplication and improve maintainability.

## Pattern: Extract Common Test Steps into Helper Functions

### Problem
Multiple test functions for the same operation (e.g., AuthenticateClient) contain identical code blocks for:
- Initial setup and authentication steps
- Request building with minor variations
- Response validation

This leads to:
- 60-80% code duplication between related tests
- Maintenance burden when protocol changes
- Obscured test intent due to boilerplate code

### Solution: Helper Function Extraction

#### 1. **Extract Common Setup Steps**
```ttcn
// Helper function to perform InitiateAuthentication and return server challenge
function f_performInitiateAuthentication() runs on smdpp_ConnHdlr return octetstring {
    ext_logInfo("Step 1: InitiateAuthentication");
    var RemoteProfileProvisioningRequest initReq := f_create_initiate_authentication_request();
    var RemoteProfileProvisioningResponse initResp := f_es9p_transceive_success(initReq);
    
    if (not f_validate_initiate_authentication_response(initResp)) {
        setverdict(fail, "InitiateAuthentication validation failed");
        f_rsp_client_cleanup();
        f_shutdown(__FILE__, __LINE__);
    }
    
    var InitiateAuthenticationOkEs9 authOk := initResp.initiateAuthenticationResponse.initiateAuthenticationOk;
    g_transactionId := authOk.transactionId;
    return authOk.serverSigned1.serverChallenge;
}
```

**Benefits:**
- Single point of maintenance for authentication flow
- Automatic cleanup on failure
- Returns only what's needed by caller

#### 2. **Extract Request Building with Parameters**
```ttcn
// Helper function to build AuthenticateClient request with optional matching ID
function f_buildAuthenticateClientRequest(
    octetstring serverChallenge, 
    charstring matchingId := "TS48v4_SAIP2.3_BERTLV"
) runs on smdpp_ConnHdlr return RemoteProfileProvisioningRequest
```

**Key Design Decisions:**
- Use default parameters for nominal cases
- Make variations explicit through parameters
- Keep the full request building logic together

#### 3. **Extract Common Validation**
```ttcn
function f_validateAuthenticateClientResponse(
    AuthenticateClientOk authClientOk
) runs on smdpp_ConnHdlr
```

### Applying the Pattern

#### Before (80+ lines per test):
```ttcn
function f_TC_AuthenticateClient_01_Nominal(charstring id) runs on smdpp_ConnHdlr {
    // 15 lines of InitiateAuthentication
    // 35 lines of request building
    // 10 lines of response handling
    // 20 lines of test-specific validation
}
```

#### After (30 lines per test):
```ttcn
function f_TC_AuthenticateClient_01_Nominal(charstring id) runs on smdpp_ConnHdlr {
    if (not f_rsp_client_init()) {
        setverdict(fail, "RSP client initialization failed");
        return;
    }

    var octetstring serverChallenge := f_performInitiateAuthentication();
    
    var RemoteProfileProvisioningRequest authReq := f_buildAuthenticateClientRequest(serverChallenge);
    var RemoteProfileProvisioningResponse authResp := f_es9p_transceive_success(authReq);
    
    f_validateAuthenticateClientResponse(authResp.authenticateClientResponseEs9.authenticateClientOk);
    
    // Test-specific validation only
    if (authClientOk.smdpSigned2.ccRequiredFlag) {
        setverdict(fail, "Unexpected ccRequiredFlag set");
    }
    
    setverdict(pass);
}
```

## Pattern: Error Injection Through Helper Parameters

### Problem
Error test cases need to modify specific parts of the request/flow while keeping the rest identical.

### Solution
Add optional parameters to helper functions for error injection:

```ttcn
function f_buildAuthenticateClientRequest(
    octetstring serverChallenge,
    charstring matchingId := "TS48v4_SAIP2.3_BERTLV",
    template (omit) octetstring transactionIdOverride := omit,  // For error injection
    template (omit) boolean invalidateSignature := omit        // For signature errors
) runs on smdpp_ConnHdlr return RemoteProfileProvisioningRequest
```

This allows error tests to remain concise while clearly showing what's being modified.

**Why use `template (omit)` instead of empty values?**
- More idiomatic TTCN-3 - `omit` clearly indicates "not provided"
- Type-safe - can't accidentally pass an empty value when you mean "use default"
- Self-documenting - the parameter type shows it's optional
- Cleaner calls - no need to pass `''O` or `false` for defaults

## Guidelines for Applying These Patterns

1. **When to Extract Helper Functions:**
   - Code block appears in 3+ related tests
   - Code block is >10 lines of boilerplate
   - Only minor variations exist between instances

2. **Helper Function Design:**
   - Use `runs on smdpp_ConnHdlr` to maintain component context
   - Return only what subsequent steps need
   - Use default parameters for nominal behavior
   - Make error injection explicit through parameters

3. **Naming Conventions:**
   - Prefix with `f_` for functions
   - Use descriptive names: `f_performInitiateAuthentication`
   - Group related helpers together in the file

4. **Error Handling:**
   - Helper functions should handle their own errors
   - Use `f_shutdown(__FILE__, __LINE__)` for fatal errors
   - Clean up resources before failing

## Benefits Achieved

- **60% code reduction** in individual test functions
- **Single maintenance point** for protocol flows
- **Clearer test intent** - differences between tests are explicit
- **Easier to add new tests** - just call helpers with different parameters
- **Consistent error handling** across all tests

## Example: Refactoring AuthenticateClient Tests

### Original Test (70+ lines):
```ttcn
function f_TC_AuthenticateClient_03_Mismatched_Transaction_ID(charstring id) runs on smdpp_ConnHdlr {
    // 15 lines of initialization and InitiateAuthentication
    // 35 lines of manual AuthenticateClient request building
    // 5 lines of error validation
}
```

### Refactored Test (27 lines):
```ttcn
function f_TC_AuthenticateClient_03_Mismatched_Transaction_ID(charstring id) runs on smdpp_ConnHdlr {
    ext_logInfo("=== Test Case: AuthenticateClient - Mismatched Transaction ID ===");
    
    if (not f_rsp_client_init()) {
        setverdict(fail, "RSP client initialization failed");
        return;
    }
    
    /* Step 1: InitiateAuthentication */
    var octetstring serverChallenge := f_performInitiateAuthentication();
    
    /* Step 2: AuthenticateClient with mismatched transaction ID */
    ext_logInfo("Step 2: AuthenticateClient with wrong transaction ID in signed data");
    
    var octetstring wrongTransactionId := f_rnd_octstring(16);
    var RemoteProfileProvisioningRequest authReq := f_buildAuthenticateClientRequest(
        serverChallenge, 
        "TS48v4_SAIP2.3_BERTLV", 
        wrongTransactionId  /* Inject wrong transaction ID */
    );
    
    /* Expect error: Subject Code 8.1.1, Reason Code 3.9 */
    var JSON_ESx_FunctionExecutionStatusCodeData errorData := f_es9p_transceive_error(authReq);
    f_validate_error_response(errorData, "8.1.1", "3.9", "Unknown/Invalid Transaction ID");
    
    ext_logInfo("Test passed - Correct error for mismatched transaction ID");
    f_rsp_client_cleanup();
    setverdict(pass);
}
```

### Results Achieved:
- **62% code reduction** (from 70 to 27 lines)
- **Clear error injection point** via the transactionIdOverride parameter
- **Reuses common flow** while maintaining test-specific behavior
- **Improved readability** - the test intent is immediately clear

## Future Considerations

- Consider creating a test base class pattern if TTCN-3 supports it
- Look for opportunities to create data-driven tests
- Extract more complex validation logic into reusable functions
- Apply similar patterns to other protocol operations (GetBoundProfilePackage, etc.)