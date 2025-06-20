# JSON Error Handling Implementation

## Overview
The SM-DP+ protocol uses a dual-layer approach with JSON wrapping ASN.1 structures. Error responses are handled at the JSON layer using specific subject/reason code pairs.

## Key Components

### 1. DecodedRPPReponse_Wrap Structure
Located in `/app/tt-smdpp/library/euicc/es9p_Types_JSON.ttcn`:
```ttcn
type record DecodedRPPReponse_Wrap {
    RemoteProfileProvisioningResponse asn1_pdu optional,
    JSON_ESx_FunctionExecutionStatusCodeData err optional
}
```
This wrapper allows handling both successful ASN.1 responses and JSON error responses.

### 2. JSON Error Structure
Located in `/app/tt-smdpp/library/euicc/esx_header_Types_JSON.ttcn`:
```ttcn
type record JSON_ESx_FunctionExecutionStatusCodeData {
    charstring subjectCode,
    charstring reasonCode,
    charstring message_ ("message")
}
```
Note: `message_` is used because "message" is a TTCN-3 reserved word.

### 3. Error Handling Functions

#### f_perform_es9p_request_expecting_error
This helper function specifically handles cases where we expect an error response:
```ttcn
private function f_perform_es9p_request_expecting_error(
    RemoteProfileProvisioningRequest request,
    charstring endpoint
) runs on smdpp_ConnHdlr
return JSON_ESx_FunctionExecutionStatusCodeData
```

#### f_perform_es9p_request_response_new
Updated to handle the new wrapper structure:
- Checks if `asn1_pdu` is present for successful responses
- Returns omit if an error occurred (error handled by caller)

## Error Code Format
Error codes follow the SGP.23 specification format:
- Subject Code: X.X.X (e.g., "8.8.1")
- Reason Code: Y.Y (e.g., "3.8")
- Example: Invalid SM-DP+ Address returns subjectCode="8.8.1", reasonCode="3.8"

## Implementation Example
From test case TC_SM_DP_ES9_InitiateAuthenticationNIST_03_InvalidServerAddress:
```ttcn
// Send request with invalid server address
var JSON_ESx_FunctionExecutionStatusCodeData error_resp := 
    f_perform_es9p_request_expecting_error(req, "/gsma/rsp2/es9plus/initiateAuthentication");

// Verify error codes
if (error_resp.subjectCode != "8.8.1" or error_resp.reasonCode != "3.8") {
    setverdict(fail, "Expected error 8.8.1/3.8 but got ", 
               error_resp.subjectCode, "/", error_resp.reasonCode);
    return;
}
```

## Python Server Integration
The Python SM-DP+ server (`pyserver.log`) returns JSON error responses:
```
<twisted.python.failure.Failure __main__.ApiError: ('8.8.1', '3.8', 'Invalid SM-DP+ Address')>
```

## Key Considerations
1. Always check `ispresent()` before accessing optional fields
2. Use separate functions for success vs error response handling
3. Error codes are strings (not integers) at the JSON layer
4. The outer communication layer is JSON, inner payloads are ASN.1