# ES9+ Functions Specification Compliance Analysis

## Overview
This document analyzes the ES9+ functions in `/app/pysim/osmo-smdpp.py` for structural compliance with SGP.22 v2.5 specification. Focus is on request/response structure, parameter names, error codes, and HTTP status codes.

## 1. InitiateAuthentication (line 838-945)

### Specification Requirements (SGP.22 Section 5.6.1, lines 6129-6198)

**Request Parameters:**
- `euiccChallenge`: Binary[16] (base64 encoded)
- `euiccInfo1`: Binary(1) (base64 encoded)
- `smdpAddress`: FQDN string

**Response Parameters:**
- `transactionId`: Binary[1-16] (hex string pattern ^[0-9,A-F]{2,32}$)
- `serverSigned1`: Binary(1) (base64 encoded)
- `serverSignature1`: Binary(1) (base64 encoded)
- `euiccCiPKIdToBeUsed`: Binary(1) (base64 encoded)
- `serverCertificate`: Binary(1) (base64 encoded)

**Error Codes:**
- 8.8.1 - 3.8: Invalid SM-DP+ Address
- 8.8.2 - 3.1: None of the proposed Public Key Identifiers is supported
- 8.8.3 - 3.1: Specification Version Number not supported
- 8.8.4 - 3.7: SM-DP+ has no CERT.DPAuth.ECDSA

### Implementation Issues Found:

1. **Line 840: Function name typo** - `initiateAutentication` should be `initiateAuthentication`

2. **Line 845: Incorrect error code** - Using ApiError('8.8.1', '3.8', 'Invalid SM-DP+ Address') correctly

3. **Line 853: Non-standard error** - Using ApiError('8.1.3', '3.3', 'eUICC challenge reuse detected') - this error code is not defined in spec for InitiateAuthentication

4. **Line 902: Incorrect error code** - Using ApiError('8.8.2', '3.1', ...) correctly

5. **Line 870-872: Incorrect error codes** - Using ApiError('8.8.3', '3.1', ...) correctly

6. **Line 907: Incorrect error code** - Using ApiError('8.8.4', '3.7', ...) but with wrong description text

### Fixes Needed:
- Fix function name typo
- Remove non-standard error for challenge reuse or use a generic error
- Fix error description for 8.8.4

## 2. AuthenticateClient (line 947-1237)

### Specification Requirements (SGP.22 Section 5.6.3, lines 6279-6387)

**Request Parameters:**
- `transactionId`: Binary[1-16] (hex string)
- `authenticateServerResponse`: Binary(1) (base64 encoded)

**Response Parameters:**
- `transactionId`: Binary[1-16] (hex string)
- `profileMetadata`: Binary(1) (base64 encoded) - CONDITIONAL
- `smdpSigned2`: Binary(1) (base64 encoded) - CONDITIONAL
- `smdpSignature2`: Binary(1) (base64 encoded) - CONDITIONAL
- `smdpCertificate`: Binary(1) (base64 encoded) - CONDITIONAL

**Error Codes:**
- 8.1.2 - 6.1: EUM Certificate verification failed
- 8.1.2 - 6.3: EUM Certificate expired
- 8.1.3 - 6.1: eUICC Certificate verification failed
- 8.1.3 - 6.3: eUICC Certificate expired
- 8.1 - 6.1: eUICC signature invalid or serverChallenge invalid
- 8.1 - 4.8: Insufficient Memory
- 8.11.1 - 3.9: Unknown CI Public Key
- 8.2 - 1.2: Profile not released
- 8.10.1 - 3.9: Unknown TransactionID
- 8.2.6 - 3.8: MatchingID refused
- 8.1.1 - 3.8: EID refused
- 8.2.5 - 4.3: No eligible Profile
- 8.8.5 - 4.10: Download order expired
- 8.8.5 - 6.4: Maximum retries exceeded

### Implementation Issues Found:

1. **Line 981: Incorrect error code** - Using ApiError('8.10.1', '3.9', 'Unknown') - should be "The RSP session identified by the TransactionID is unknown"

2. **Line 994: Incorrect error code** - Using ApiError('8.11.1', '3.9', 'Unknown') - should be "Unknown CI Public Key"

3. **Lines 1004-1012: Certificate expiry checks** - Correctly implemented

4. **Line 1074: Incorrect error description** - Using "Verification failed (euiccSignature1 over euiccSigned1)" - should be "eUICC signature is invalid or serverChallenge is invalid"

5. **Line 1087: Incorrect error description** - Using "Verification failed (serverChallenge)" - should be combined with signature error

6. **Line 1091: Incorrect error description** - Using "The signed transactionId != outer transactionId" - this is correct but should use 8.10.1

7. **Line 1112: Incorrect error description** - Using "No Profile available for this EID" - should be "EID doesn't match the expected value" or "EID refused"

8. **Line 1125: Incorrect error code** - Using ApiError('8.2.6', '3.8', 'MatchingID could not be matched') - description should be "MatchingID (AC_Token or EventID) is refused"

9. **Line 1135: Correct error code** - Using ApiError('8.1.1', '3.8', ...) correctly

10. **Line 1156: Incorrect error description** - Using "Activation Code has not been successfully verified" - should be "MatchingID (AC_Token or EventID) is refused"

11. **Line 1162: Correct error code** - Using ApiError('8.2', '1.2', ...) correctly

12. **Line 1169: Correct error code** - Using ApiError('8.8.5', '4.10', ...) correctly

13. **Line 1173: Incorrect error code** - Using ApiError('8.8.5', '6.4', ...) correctly

14. **Line 1179: Correct error code** - Using ApiError('8.2.5', '4.3', ...) correctly

### Fixes Needed:
- Update error descriptions to match specification exactly
- Consolidate signature and serverChallenge verification errors under 8.1 - 6.1
- Fix MatchingID error descriptions

## 3. GetBoundProfilePackage (line 1239-1349)

### Specification Requirements (SGP.22 Section 5.6.2, lines 6200-6278)

**Request Parameters:**
- `transactionId`: Binary[1-16] (hex string)
- `prepareDownloadResponse`: Binary(1) (base64 encoded)

**Response Parameters:**
- `transactionId`: Binary[1-16] (hex string)
- `boundProfilePackage`: Binary(1) (base64 encoded)

**Error Codes:**
- 8.1 - 6.1: eUICC signature is invalid
- 8.2 - 3.7: BPP not available for new binding
- 8.10.1 - 3.9: The RSP session identified by the TransactionID is unknown
- 8.2.7 - 2.2: Confirmation Code is missing
- 8.2.7 - 3.8: Confirmation Code is refused
- 8.2.7 - 6.4: Maximum retries for Confirmation Code exceeded
- 8.8.5 - 4.10: Download order expired

### Implementation Issues Found:

1. **Line 1248: Correct error** - Using ApiError('8.10.1', '3.9', ...) correctly

2. **Line 1260: Non-standard error** - Using ApiError('8.2.10', '4.2', ...) - this error code doesn't exist in spec for GetBoundProfilePackage

3. **Line 1268: Correct error** - Using ApiError('8.1', '6.1', ...) correctly

4. **Line 1272: Duplicate TransactionID check** - Using ApiError('8.10.1', '3.9', ...) - this check is redundant

5. **Line 1316: Correct error** - Using ApiError('8.2.7', '2.2', ...) correctly

6. **Line 1328: Correct error** - Using ApiError('8.2.7', '3.8', ...) correctly

### Fixes Needed:
- Remove non-standard error code 8.2.10
- Remove redundant transaction ID check
- Handle download response errors according to spec

## 4. HandleNotification (line 1351-1402)

### Specification Requirements (SGP.22 Section 5.6.4, lines 6387-6415)

**Request Parameters:**
- `pendingNotification`: Binary(1) (base64 encoded)

**Response:**
- HTTP 204 No Content with empty body (per SGP.22 Section 6.3, line 8367)

**No specific error codes defined**

### Implementation Issues Found:

1. **Line 1357: Correct HTTP status** - Setting response code 204 correctly

2. **Line 1374, 1393: Exception handling** - Raising generic exceptions instead of returning proper error responses

### Fixes Needed:
- Should not raise exceptions for verification failures
- Should return empty response even on errors (per HTTP 204 semantics)
- Consider logging errors instead of throwing exceptions

## 5. CancelSession (line 1409-1429)

### Specification Requirements (SGP.22 Section 5.6.5, lines 6415-6478)

**Request Parameters:**
- `transactionId`: Binary[1-16] (hex string)
- `cancelSessionResponse`: Binary(1) (base64 encoded)

**Response:**
- No output data (empty response with functionExecutionStatus)

**Error Codes:**
- 8.10.1 - 3.9: The RSP session identified by the TransactionID is unknown
- 8.1 - 6.1: eUICC signature is invalid
- 8.8 - 3.10: The provided SM-DP+ OID is invalid

### Implementation Issues Found:

1. **Line 1419: Correct error** - Using ApiError('8.10.1', '3.9', ...) correctly

2. **Missing validation** - Not verifying eUICC signature (euiccCancelSessionSignature)

3. **Missing validation** - Not verifying smdpOid

4. **Line 1427: Incomplete implementation** - Just returns without proper response

### Fixes Needed:
- Implement signature verification
- Implement SM-DP+ OID verification
- Return proper response structure

## HTTP Response Structure Issues

Per SGP.22 Section 6.3 (line 8357-8369):
- Normal request-response: HTTP 200 with JSON response body
- Normal notification: HTTP 204 with empty body
- Errors should still return HTTP 200 with error in JSON response

### Current Implementation:
- Using `@rsp_api_wrapper` decorator that builds proper JSON response structure
- HandleNotification correctly sets HTTP 204
- Other functions return JSON with header containing functionExecutionStatus

## Summary of Critical Issues

1. **Function naming**: `initiateAutentication` typo
2. **Error descriptions**: Many don't match specification exactly
3. **Missing validations**: CancelSession missing signature and OID checks
4. **Non-standard errors**: Using undefined error codes
5. **Exception handling**: HandleNotification throws exceptions instead of returning proper response

## Recommended Priority Fixes

1. Fix function name typo (Critical)
2. Update all error descriptions to match spec exactly (High)
3. Implement missing validations in CancelSession (High)
4. Remove non-standard error codes (Medium)
5. Fix HandleNotification exception handling (Medium)