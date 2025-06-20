# SM-DP+ Test Case Analysis

## Overview
This document provides a comprehensive analysis of the SM-DP+ test cases defined in SGP.23 v1.15 test specification. The tests validate the ES9+ interface between LPA and SM-DP+ for Remote SIM Provisioning (RSP) protocol.

**Test Specification Location**: `/app/testspec.md`

## Test Case Inventory

### 1. InitiateAuthentication Tests (4.3.12 - Line 3)
- **TC_SM-DP+_ES9+.InitiateAuthenticationNIST** (4.3.12.2.1 - Line 22) - NIST P-256 curve tests
  - Test Sequence #01: Nominal case (Line 24)
  - Test Sequence #02: Uniqueness of Transaction ID and Server Challenge (Line 31)
  - Test Sequence #03: Error - Invalid Server Address (8.8.1/3.8) (Line 40)
  - Test Sequence #04: Error - Unsupported Public Key Identifiers (8.8.2/3.1) (Line 47)
  - Test Sequence #05: Error - Unsupported Specification Version (8.8.3/3.1) (Line 54)
  - Test Sequence #06: Error - Unavailable Server Auth Certificate (8.8.4/3.7) (Line 63)
  - Test Sequence #07: Nominal - eUICC v2.2.1 (Line 70)
  - Test Sequence #08: Nominal - eUICC v2.2.2 (Line 77)
  - Test Sequence #09: Nominal - eUICC v2.3 (Line 84)
- **TC_SM-DP+_ES9+.InitiateAuthenticationFRP** (4.3.12.2.2 - Line 91) - Fast RSA Provisioning (FFS)
- **TC_SM-DP+_ES9+.InitiateAuthenticationBRP** (4.3.12.2.3 - Line 95) - BrainpoolP256r1 curve tests

### 2. GetBoundProfilePackage Tests (4.3.13 - Line 106)
- **TC_SM-DP+_ES9+.GetBoundProfilePackageNIST** (4.3.13.2.1 - Line 118)
  - Test Sequence #01: Using S-ENC/S-MAC without Confirmation Code (Line 125)
  - Test Sequence #02: Using S-ENC/S-MAC with Confirmation Code (Line 137)
  - Test Sequence #03: Using PPK-ENC/PPK-MAC without Confirmation Code (Line 149)
  - Test Sequence #04: Using PPK-ENC/PPK-MAC with Confirmation Code (Line 161)
  - Test Sequence #05: S-ENC/S-MAC with Metadata split over 2 segments (no CC) (Line 173)
  - Test Sequence #06: PPK-ENC/PPK-MAC with Metadata split over 2 segments (no CC) (Line 186)
- **TC_SM-DP+_ES9+.GetBoundProfilePackageFRP** (4.3.13.2.2 - Line 199) - Fast RSA Provisioning
- **TC_SM-DP+_ES9+.GetBoundProfilePackageBRP** (4.3.13.2.3 - Line 203) - BrainpoolP256r1 tests
- **TC_SM-DP+_ES9+.GetBoundProfilePackage_RetryCases_ReuseOTPK_NIST** (4.3.13.2.4 - Line 218)
  - Test Sequence #01: Retry with same otPK using S-ENC/S-MAC without CC (Line 225)
  - Test Sequence #02: Retry with same otPK using S-ENC/S-MAC with CC (Line 240)
  - Test Sequence #03: Retry with same otPK using PPK-ENC/PPK-MAC without CC (Line 255)
  - Test Sequence #04: Retry with same otPK using PPK-ENC/PPK-MAC with CC (Line 270)
  - Test Sequence #05: Retry with same otPK rejected by eUICC using S-ENC/S-MAC without CC (Line 285)
  - Test Sequence #06: Retry with same otPK rejected by eUICC using S-ENC/S-MAC with CC (Line 300)
  - Test Sequence #07: Retry with same otPK rejected by eUICC using PPK-ENC/PPK-MAC without CC (Line 315)
  - Test Sequence #08: Retry with same otPK rejected by eUICC using PPK-ENC/PPK-MAC with CC (Line 330)
  - Test Sequence #09: Confirmation Code retry (Line 345)
- **TC_SM-DP+_ES9+.GetBoundProfilePackage_RetryCases_DifferentOTPK_NIST** (4.3.13.2.7 - Line 364)
  - Test Sequence #01: Retry without otPK using S-ENC/S-MAC without CC (Line 371)
  - Test Sequence #02: Retry without otPK using S-ENC/S-MAC with CC (Line 386)
  - Test Sequence #03: Retry without otPK using PPK-ENC/PPK-MAC without CC (Line 401)
  - Test Sequence #04: Retry without otPK using PPK-ENC/PPK-MAC with CC (Line 416)
- **TC_SM-DP+_ES9+.GetBoundProfilePackage_ErrorCasesNIST** (Not found in current excerpt)
  - Test Sequence #01: Error - Invalid eUICC Signature (8.1/6.1)
  - Test Sequence #02: Error - Unknown TransactionID in JSON (8.10.1/3.9)
  - Test Sequence #03: Error - Unknown TransactionID in ASN.1 (8.10.1/3.9)
  - Test Sequence #04: Error - Missing Confirmation Code (8.2.7/2.2)
  - Test Sequence #05: Error - Refused Confirmation Code (8.2.7/3.8)

### 3. AuthenticateClient Tests (4.3.14)
- **TC_SM-DP+_ES9+.AuthenticateClientNIST**
  - Test Sequence #01: Profile Download Default DP+ Address (no CC)
  - Test Sequence #02: Profile Download Default DP+ Address (with CC)
  - Test Sequence #03: Profile Download Activation Code
  - Test Sequence #04: Profile Download SM-DS Use Case
  - Test Sequence #05: SM-DS Event Retrieval
  - Test Sequence #06: SM-DS Event Registration
  - Test Sequence #07: SM-DS Event Deletion
  - Test Sequence #08: Profile Download Activation Code + Event Registration
- **TC_SM-DP+_ES9+.AuthenticateClientNIST_ErrorCases**
  - Test Sequence #1-8: Various certificate and signature errors
  - Test Sequence #9-17: Various business logic errors
- **TC_SM-DP+_ES9+.AuthenticateClientFRP** - Fast RSA Provisioning
- **TC_SM-DP+_ES9+.AuthenticateClientBRP** - BrainpoolP256r1 tests
- **TC_SM-DP+_ES9+.AuthenticateClient_RetryCases_Reuse_OTPK**

### 4. HandleNotification Tests (4.3.15)
- **TC_SM-DP+_ES9+_HandleNotificationNIST**
  - Multiple test sequences for various notification reasons
- **TC_SM-DP+_ES9+_HandleNotificationFRP**
- **TC_SM-DP+_ES9+_HandleNotificationBRP**

### 5. CancelSession Tests (4.3.16)
- **TC_SM-DP+_ES9+.CancelSession_After_AuthenticateClientNIST**
- **TC_SM-DP+_ES9+.CancelSession_After_GetBoundProfilePackageNIST**
- **TC_SM-DP+_ES9+.CancelSession_After_AuthenticateClientFRP**
- **TC_SM-DP+_ES9+.CancelSession_After_GetBoundProfilePackageFRP**
- **TC_SM-DP+_ES9+.CancelSession_After_AuthenticateClientBRP**
- **TC_SM-DP+_ES9+.CancelSession_After_GetBoundProfilePackageBRP**

### 6. TLS Server Authentication Tests (4.3.17)
- **TC_SM-DP+_ES9+_Server_Authentication_for_HTTPS_EstablishmentNIST**
- **TC_SM-DP+_ES9+_Server_Authentication_for_HTTPS_EstablishmentBRP**

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