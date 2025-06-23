# Missing AuthenticateClient Error Tests Implementation Plan

## Overview
Based on SGP.23 section 4.3.14.2.2, we need to implement 14 missing error test cases for AuthenticateClient.

## Missing Error Tests

### Certificate Validation Errors

#### 1. Test #1 - Invalid EUM Certificate (8.1.2/6.1)
- Multiple sub-tests for different validation failures:
  - Invalid signature
  - Invalid key usage extension
  - Invalid certificate policies extension
  - Invalid basic constraints
  - Invalid path length constraint

#### 2. Test #2 - Expired EUM Certificate (8.1.2/6.3)
- Test with EUM certificate that has expired validity period

#### 3. Test #3 - Invalid eUICC Certificate (8.1.3/6.1)
- Multiple sub-tests similar to Test #1 but for eUICC cert

#### 4. Test #4 - Expired eUICC Certificate (8.1.3/6.3)
- Test with eUICC certificate that has expired validity period

#### 5. Test #7 - Unknown CI Public Key (8.11.1/3.9)
- Test with certificates signed by unknown CI

### Matching ID Errors

#### 6. Test #12 - Invalid Matching ID for Activation Code (8.2.6/3.8)
- Wrong matching ID for activation code use case

#### 7. Test #13 - Invalid Matching ID for SM-DS (8.2.6/3.8)
- Wrong matching ID for SM-DS event use case

### EID Mismatch Errors

#### 8. Test #14 - Un-matched EID for Default DP+ (8.1.1/3.8)
- EID not associated with profile for default SM-DP+ use case

#### 9. Test #19 - Un-matched EID for SM-DS (8.1.1/3.8)
- EID not allowed for SM-DS event profile

#### 10. Test #20 - Un-matched EID for Activation Code (8.1.1/3.8)
- EID not allowed for activation code profile

### Profile/Order Errors

#### 11. Test #15 - No Eligible Profile (8.2.5/4.3)
- No profile matches device capabilities

#### 12. Test #16 - Download Order Expired (8.8.5/4.10)
- Profile download order has expired

#### 13. Test #17 - Maximum retries exceeded (8.8.5/6.4)
- Too many download attempts

### Special Cases

#### 14. Test #21 - Invalid MatchingId for Activation Code not associated to EID (8.2.6/3.8)
- Activation code exists but not for this specific EID

## Implementation Strategy

### 1. Certificate Error Tests (#1-4, #7)
These require special handling as we can't use actual invalid certificates:
- Create a certificate injection mechanism in test functions
- Use valid certificates but inject errors at validation time
- Alternatively, create test certificates with specific defects

### 2. Business Logic Error Tests (#12-21)
These can be implemented using the existing error test framework:
- Use specific matchingId values that trigger errors
- Configure Python server with test profiles that enforce these conditions
- Leverage existing `f_test_authenticateClient_error` function

### 3. Python Server Updates Required
- Add test profiles with specific error conditions
- Implement download attempt tracking
- Add profile eligibility checking
- Support order expiration simulation

## Priority Order
1. Business logic errors (easier to implement)
2. Certificate validation errors (require more infrastructure)