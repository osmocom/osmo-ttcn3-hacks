# SM-DP+ Test Case Analysis

## Overview
This document provides a comprehensive analysis of the SM-DP+ test cases defined in SGP.23 v1.15 test specification. The tests validate the ES9+ interface between LPA and SM-DP+ for Remote SIM Provisioning (RSP) protocol.

**Test Specification Location**: `/app/testspec.md`
**Note**: All line numbers referenced in this document refer to lines in the testspec.md file

## Test Case Inventory

### 1. InitiateAuthentication Tests (4.3.12 - Line 3 in testspec)
- **TC_SM-DP+_ES9+.InitiateAuthenticationNIST** (4.3.12.2.1 - Line 22 in testspec) - NIST P-256 curve tests
  - Test Sequence #01: Nominal case (Line 24 in testspec)
  - Test Sequence #02: Uniqueness of Transaction ID and Server Challenge (Line 31 in testspec)
  - Test Sequence #03: Error - Invalid Server Address (8.8.1/3.8) (Line 40 in testspec)
  - Test Sequence #04: Error - Unsupported Public Key Identifiers (8.8.2/3.1) (Line 47 in testspec)
  - Test Sequence #05: Error - Unsupported Specification Version (8.8.3/3.1) (Line 54 in testspec)
  - Test Sequence #06: Error - Unavailable Server Auth Certificate (8.8.4/3.7) (Line 63 in testspec)
  - Test Sequence #07: Nominal - eUICC v2.2.1 (Line 70 in testspec)
  - Test Sequence #08: Nominal - eUICC v2.2.2 (Line 77 in testspec)
  - Test Sequence #09: Nominal - eUICC v2.3 (Line 84 in testspec)
- **TC_SM-DP+_ES9+.InitiateAuthenticationFRP** (4.3.12.2.2 - Line 91 in testspec) - Fast RSA Provisioning (FFS)
- **TC_SM-DP+_ES9+.InitiateAuthenticationBRP** (4.3.12.2.3 - Line 95 in testspec) - BrainpoolP256r1 curve tests

### 2. GetBoundProfilePackage Tests (4.3.13 - Line 106 in testspec)
- **TC_SM-DP+_ES9+.GetBoundProfilePackageNIST** (4.3.13.2.1 - Line 118 in testspec)
  - Test Sequence #01: Using S-ENC/S-MAC without Confirmation Code (Line 125 in testspec)
  - Test Sequence #02: Using S-ENC/S-MAC with Confirmation Code (Line 137 in testspec)
  - Test Sequence #03: Using PPK-ENC/PPK-MAC without Confirmation Code (Line 149 in testspec)
  - Test Sequence #04: Using PPK-ENC/PPK-MAC with Confirmation Code (Line 161 in testspec)
  - Test Sequence #05: S-ENC/S-MAC with Metadata split over 2 segments (no CC) (Line 173 in testspec)
  - Test Sequence #06: PPK-ENC/PPK-MAC with Metadata split over 2 segments (no CC) (Line 186 in testspec)
- **TC_SM-DP+_ES9+.GetBoundProfilePackageFRP** (4.3.13.2.2 - Line 199 in testspec) - Fast RSA Provisioning
- **TC_SM-DP+_ES9+.GetBoundProfilePackageBRP** (4.3.13.2.3 - Line 203 in testspec) - BrainpoolP256r1 tests
- **TC_SM-DP+_ES9+.GetBoundProfilePackage_RetryCases_ReuseOTPK_NIST** (4.3.13.2.4 - Line 218 in testspec)
  - Test Sequence #01: Retry with same otPK using S-ENC/S-MAC without CC (Line 225 in testspec)
  - Test Sequence #02: Retry with same otPK using S-ENC/S-MAC with CC (Line 240 in testspec)
  - Test Sequence #03: Retry with same otPK using PPK-ENC/PPK-MAC without CC (Line 255 in testspec)
  - Test Sequence #04: Retry with same otPK using PPK-ENC/PPK-MAC with CC (Line 270 in testspec)
  - Test Sequence #05: Retry with same otPK rejected by eUICC using S-ENC/S-MAC without CC (Line 285 in testspec)
  - Test Sequence #06: Retry with same otPK rejected by eUICC using S-ENC/S-MAC with CC (Line 300 in testspec)
  - Test Sequence #07: Retry with same otPK rejected by eUICC using PPK-ENC/PPK-MAC without CC (Line 315 in testspec)
  - Test Sequence #08: Retry with same otPK rejected by eUICC using PPK-ENC/PPK-MAC with CC (Line 330 in testspec)
  - Test Sequence #09: Confirmation Code retry (Line 345 in testspec)
- **TC_SM-DP+_ES9+.GetBoundProfilePackage_RetryCases_DifferentOTPK_NIST** (4.3.13.2.7 - Line 364 in testspec)
  - Test Sequence #01: Retry without otPK using S-ENC/S-MAC without CC (Line 371 in testspec)
  - Test Sequence #02: Retry without otPK using S-ENC/S-MAC with CC (Line 386 in testspec)
  - Test Sequence #03: Retry without otPK using PPK-ENC/PPK-MAC without CC (Line 401 in testspec)
  - Test Sequence #04: Retry without otPK using PPK-ENC/PPK-MAC with CC (Line 416 in testspec)
- **TC_SM-DP+_ES9+.GetBoundProfilePackage_ErrorCasesNIST** (4.3.13.2.10 - Line 435 in testspec)
  - Test Sequence #01: Error - Invalid eUICC Signature (8.1/6.1) (Line 442 in testspec)
  - Test Sequence #02: Error - Unknown TransactionID in JSON (8.10.1/3.9) (Line 456 in testspec)
  - Test Sequence #03: Error - Unknown TransactionID in ASN.1 (8.10.1/3.9) (Line 470 in testspec)
  - Test Sequence #04: Error - Missing Confirmation Code (8.2.7/2.2) (Line 484 in testspec)
  - Test Sequence #05: Error - Refused Confirmation Code (8.2.7/3.8) (Line 498 in testspec)

### 3. AuthenticateClient Tests (4.3.14 - Line 519 in testspec)
- **TC_SM-DP+_ES9+.AuthenticateClientNIST** (4.3.14.2.1 - Line 531 in testspec)
  - Test Sequence #01: Profile Download Default DP+ Address (no CC) (Line 538 in testspec)
  - Test Sequence #02: Profile Download Default DP+ Address (with CC) (Line 551 in testspec)
  - Test Sequence #03: Profile Download Default DP+ Use Case Second Attempt (no CC) (Line 564 in testspec)
  - Test Sequence #05: SM-DS Use Case (no CC) (Line 580 in testspec)
  - Test Sequence #06: SM-DS Use Case (with CC) (Line 593 in testspec)
  - Test Sequence #08: Activation Code Use Case with Matching ID (no CC) (Line 608 in testspec)
  - Test Sequence #09: Activation Code Use Case with Matching ID (with CC) (Line 621 in testspec)
  - Test Sequence #11: Activation Code Use Case with Matching ID (no CC) not associated to EID (Line 636 in testspec)
  - Test Sequence #12: Activation Code Use Case with Matching ID (with CC) not associated to EID (Line 645 in testspec)
  - Test Sequence #14-21: Various nominal cases with MatchingId variations (Lines 656-729 in testspec)
- **TC_SM-DP+_ES9+.AuthenticateClientNIST_ErrorCases** (4.3.14.2.2 - Line 742 in testspec)
  - Test Sequence #1-8: Various certificate and signature errors (Lines 749-872 in testspec)
  - Test Sequence #9-21: Various business logic errors (Lines 885-1031 in testspec)
- **TC_SM-DP+_ES9+.AuthenticateClientFRP** (4.3.14.2.3 - Line 1044 in testspec) - Fast RSA Provisioning
- **TC_SM-DP+_ES9+.AuthenticateClientBRP** (4.3.14.2.5 - Line 1050 in testspec) - BrainpoolP256r1 tests
- **TC_SM-DP+_ES9+.AuthenticateClient_RetryCases_Reuse_OTPK** (4.3.14.2.6 - Line 1061 in testspec)

### 4. HandleNotification Tests (4.3.15 - Line 1114 in testspec)
- **TC_SM-DP+_ES9+_HandleNotificationNIST** (4.3.15.2.1 - Line 1126 in testspec)
  - Test Sequence #01: All Notifications (Line 1133 in testspec)
  - Test Sequence #02: Successful PIR with specific notifications (Line 1156 in testspec)
  - Test Sequence #03: Error - Invalid Transaction ID (Line 1173 in testspec)
  - Test Sequence #04-18: Various PIR error reasons (Lines 1186-1368 in testspec)
- **TC_SM-DP+_ES9+_HandleNotificationFRP** (4.3.15.2.2 - Line 1381 in testspec)
- **TC_SM-DP+_ES9+_HandleNotificationBRP** (4.3.15.2.3 - Line 1385 in testspec)

### 5. CancelSession Tests (4.3.16 - Line 1396 in testspec)
- **TC_SM-DP+_ES9+.CancelSession_After_AuthenticateClientNIST** (4.3.16.2.1 - Line 1408 in testspec)
  - Test Sequence #01: End User Rejection after Authenticate Client (Line 1415 in testspec)
  - Test Sequence #02: End User Postponed after Authenticate Client (Line 1430 in testspec)
  - Test Sequence #03: Timeout after Authenticate Client (Line 1445 in testspec)
  - Test Sequence #04: PPR Not Allowed after Authenticate Client (Line 1460 in testspec)
  - Test Sequence #05: Undefined Reason after Authenticate Client (Line 1475 in testspec)
  - Test Sequence #06-09: Various error cases (Lines 1490-1537 in testspec)
- **TC_SM-DP+_ES9+.CancelSession_After_GetBoundProfilePackageNIST** (4.3.16.2.2 - Line 1552 in testspec)
  - Test Sequence #01: End User Rejection after GetBoundProfilePackage (Line 1559 in testspec)
  - Test Sequence #02: End User Postponed after GetBoundProfilePackage (Line 1574 in testspec)
  - Test Sequence #03: Timeout after GetBoundProfilePackage (Line 1589 in testspec)
  - Test Sequence #04: PPR Not Allowed after GetBoundProfilePackage (Line 1604 in testspec)
  - Test Sequence #05: Metadata Mismatch after GetBoundProfilePackage (Line 1619 in testspec)
  - Test Sequence #06: Load BPP Execution Error after GetBoundProfilePackage (Line 1634 in testspec)
  - Test Sequence #07: Undefined Reason after GetBoundProfilePackage (Line 1649 in testspec)
  - Test Sequence #08-11: Various error cases (Lines 1664-1711 in testspec)
- **TC_SM-DP+_ES9+.CancelSession_After_AuthenticateClientFRP** (4.3.16.2.3 - Line 1726 in testspec)
- **TC_SM-DP+_ES9+.CancelSession_After_GetBoundProfilePackageFRP** (4.3.16.2.4 - Line 1730 in testspec)
- **TC_SM-DP+_ES9+.CancelSession_After_AuthenticateClientBRP** (4.3.16.2.5 - Line 1734 in testspec)
- **TC_SM-DP+_ES9+.CancelSession_After_GetBoundProfilePackageBRP** (4.3.16.2.6 - Line 1749 in testspec)

### 6. TLS Server Authentication Tests (4.3.17 - Line 1764 in testspec)
- **TC_SM-DP+_ES9+_Server_Authentication_for_HTTPS_EstablishmentNIST** (4.3.17.1 - Line 1766 in testspec)
- **TC_SM-DP+_ES9+_Server_Authentication_for_HTTPS_EstablishmentBRP** (4.3.17.2 - Line 1774 in testspec)

## Common Patterns

### 1. Cryptographic Algorithm Variants
Each major operation has three variants:
- **NIST**: Using NIST P-256 curve (most common)
- **FRP**: Fast RSA Provisioning (often marked as FFS - For Future Study)
- **BRP**: Using BrainpoolP256r1 curve

### 2. Confirmation Code Scenarios
Many tests have variants for:
- Without Confirmation Code
- With Confirmation Code (#CONFIRMATION_CODE1)

### 3. Protection Key Types
Profile protection variants:
- **S-ENC/S-MAC**: Session keys derived from ECDH
- **PPK-ENC/PPK-MAC**: Profile Protection Keys (pre-shared)

### 4. Error Code Format
Errors follow pattern: "Subject Code X.X.X Reason Code Y.Y"
- Subject Code identifies the component/area
- Reason Code identifies specific error

### 5. Common Error Categories
- **Certificate Errors**: Invalid/expired certificates (6.1, 6.3)
- **Signature Errors**: Invalid signatures (6.1)
- **Transaction Errors**: Unknown transaction IDs (3.9)
- **Business Logic Errors**: Profile states, matching IDs, etc.

## Test Data Requirements

### 1. Certificates
- CI (Certificate Issuer) certificates
- EUM (eUICC Manufacturer) certificates
- eUICC certificates
- SM-DP+ authentication certificates (DPauth)
- SM-DP+ profile binding certificates (DPpb)

### 2. Test Constants
- #SERVER_ADDRESS, #IUT_SM_DP_ADDRESS
- #S_EUICC_CHALLENGE, #SERVER_CHALLENGE
- #S_EUICC_INFO1 (and variants for different versions)
- #EID1, #MATCHING_ID1
- #CONFIRMATION_CODE1
- #SMDP_METADATA_OP_PROF1 (profile metadata)

### 3. Keys
- ECDH key pairs for session establishment
- Session keys (S-ENC, S-MAC)
- Profile Protection Keys (PPK-ENC, PPK-MAC)

## Test Dependencies

### 1. Sequential Dependencies
- AuthenticateClient must precede GetBoundProfilePackage
- InitiateAuthentication starts the session
- CancelSession can occur after AuthenticateClient or GetBoundProfilePackage

### 2. State Dependencies
- Profile states: Released, Downloaded, Error
- Session states: Active, Cancelled
- Transaction ID continuity throughout session

### 3. Retry Scenarios
- Same otPK (one-time public key) reuse
- Different otPK for new attempt
- Maximum retry limits

## Complex Scenarios

### 1. Multi-Segment Metadata
Tests handling of profile metadata split across multiple ASN.1 segments (sequenceOf88)

### 2. Retry Handling
- Reusing same cryptographic material
- Generating new cryptographic material
- Error recovery scenarios

### 3. Event Management (SM-DS)
- Event registration
- Event retrieval
- Event deletion
- Combined operations

### 4. Session Management
- Normal completion
- Cancellation at various stages
- Error-induced termination

## Implementation Considerations

### 1. HTTP Layer
- All operations use HTTP POST
- JSON transport for requests/responses
- MTD_HTTP_REQ/MTD_HTTP_RESP patterns

### 2. Cryptographic Operations
- ECDH for key agreement
- AES for encryption (S-ENC, PPK-ENC)
- MAC computation and verification
- Certificate chain validation

### 3. State Machine
- Session lifecycle management
- Profile download state tracking
- Error recovery procedures

### 4. Data Encoding
- ASN.1 for protocol structures
- JSON for transport layer
- Base64 encoding for binary data

## Test Execution Order

Recommended execution order:
1. TLS/Server Authentication tests
2. InitiateAuthentication (nominal cases)
3. AuthenticateClient (nominal cases)
4. GetBoundProfilePackage (nominal cases)
5. HandleNotification tests
6. Error test cases for each operation
7. Retry scenarios
8. CancelSession tests

This order ensures basic functionality works before testing error cases and complex scenarios.