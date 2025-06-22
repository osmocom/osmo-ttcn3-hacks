# Missing Test Sequences in SM-DP+ Test Suite

This document tracks test sequences from the SGP.23 testspec that have not yet been implemented in the TTCN-3 test suite.

## Overall Implementation Coverage
- **Total sequences in spec**: ~90 test sequences
- **Implemented**: ~20-23 sequences
- **Coverage**: ~20-25%

## 1. InitiateAuthentication (Section 4.3.12)

**Implemented**: 3 of 9 sequences (33%)

### Missing Sequences:
- **Test Sequence #04** - Error: Unsupported Public Key Identifiers (Subject Code 8.8.2 Reason Code 3.1)
- **Test Sequence #05** - Error: Unsupported Specification Version Number (Subject Code 8.8.3 Reason Code 3.1)
- **Test Sequence #06** - Error: Unavailable Server Auth Certificate (Subject Code 8.8.4 Reason Code 3.7)
- **Test Sequence #07** - Nominal: eUICC v2.2.1
- **Test Sequence #08** - Nominal: eUICC v2.2.2
- **Test Sequence #09** - Nominal: eUICC v2.3

## 2. AuthenticateClient (Section 4.3.14)

**Implemented**: ~7 of 21+ sequences (~33%)

### Missing Nominal Sequences:
- **Test Sequences #03-21** - Various SM-DS use cases, Activation Code use cases, and MatchingId variations

### Missing Error Sequences (Section 4.3.14.2.2):
Most of the 21 error test sequences are not implemented, including:
- Certificate validation errors
- Matching ID errors
- Various use case specific errors

## 3. GetBoundProfilePackage (Section 4.3.13)

**Implemented**: ~6 of 20+ sequences (~30%)

### Missing Nominal Sequences:
- **Test Sequence #02** - Nominal: Using S-ENC and S-MAC with Confirmation Code
- **Test Sequence #03** - Nominal: Using PPK-ENC and PPK-MAC without Confirmation Code
- **Test Sequence #04** - Nominal: Using PPK-ENC and PPK-MAC with Confirmation Code
- **Test Sequence #05** - Nominal: Using S-ENC and S-MAC with Metadata split over 2 segments without CC
- **Test Sequence #06** - Nominal: Using PPK-ENC and PPK-MAC with Metadata split over 2 segments without CC

### Missing Retry Sequences (Section 4.3.13.2.4):
Most retry scenarios with PPK encryption and various CC combinations

### Missing Error Sequences:
- **Test Sequence #04** - Error: Missing Confirmation Code (Subject Code 8.2.7 Reason Code 2.2)
- **Test Sequence #05** - Error: Refused Confirmation Code (Subject Code 8.2.7 Reason Code 3.8)

## 4. HandleNotification (Section 4.3.15.2.1)

**Implemented**: 1 of 18 sequences (6%)

### Missing Nominal Cases:
- **Test Sequence #02** - Nominal: Successful PIR, no install OtherSignedNotification and then Enable OtherSignedNotification Notifications

### Missing Error Cases:
- **Test Sequence #03** - Error: Invalid Transaction ID
- **Test Sequence #04** - Error: PIR Error Reason - incorrect Input Values
- **Test Sequence #05** - Error: PIR Error Reason - invalid signature
- **Test Sequence #06** - Error: PIR Error Reason - unsupported Crt Values
- **Test Sequence #07** - Error: PIR Error Reason - unsupported Remote Operation Type
- **Test Sequence #08** - Error: PIR Error Reason - unsupported Profile Class
- **Test Sequence #09** - Error: PIR Error Reason - SCP03t Structure Error
- **Test Sequence #10** - Error: PIR Error Reason - SCP03t Security Error
- **Test Sequence #11** - Error: PIR Error Reason - install Failed Due To Iccid Already Exists On eUICC
- **Test Sequence #12** - Error: PIR Error Reason - install Failed Due To Insufficient Memory For Profile
- **Test Sequence #13** - Error: PIR Error Reason - install Failed Due To Interruption
- **Test Sequence #14** - Error: PIR Error Reason - install Failed Due To PE Processing Error
- **Test Sequence #15** - Error: PIR Error Reason - install Failed Due To Data Mismatch
- **Test Sequence #16** - Error: PIR Error Reason - test Profile Install Failed Due To Invalid Naa Key
- **Test Sequence #17** - Error: PIR Error Reason - PPR Not Allowed
- **Test Sequence #18** - Error: PIR Error Reason - install Failed Due To Unknown Error

### Missing BRP Variant:
- `TC_SM_DP_ES9_HandleNotificationBRP`

## 5. CancelSession (Section 4.3.11)

**Implemented**: 2 of 20 sequences (10%)

### Missing Sequences:
- **Test Sequences #01-09** for CancelSession_After_AuthenticateClient (various cancellation reasons)
- **Test Sequences #01-11** for CancelSession_After_GetBoundProfilePackage (various cancellation reasons)
- BRP variants for all CancelSession tests

## 6. TLS Server Authentication

**Implemented**: 0 of 2 tests (0%)

### Missing Tests:
- `TC_SM-DP+_ES9+_Server_Authentication_for_HTTPS_EstablishmentNIST`
- `TC_SM-DP+_ES9+_Server_Authentication_for_HTTPS_EstablishmentBRP`

## Key Implementation Gaps

1. **PPK (Profile Protection Key) scenarios** - No tests with PPK-ENC/PPK-MAC implemented
2. **Metadata segmentation** - No tests for profile metadata split across segments
3. **Confirmation Code variations** - Limited CC test coverage
4. **Error scenarios** - Most error test sequences not implemented
5. **Retry scenarios** - Limited retry test coverage
6. **SM-DS integration** - No SM-DS use case tests
7. **Activation Code flows** - No activation code tests
8. **eUICC version tests** - No tests for different eUICC versions (2.2.1, 2.2.2, 2.3)
9. **TLS-specific tests** - No TLS authentication tests