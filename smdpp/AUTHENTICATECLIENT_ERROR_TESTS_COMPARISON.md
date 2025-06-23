# AuthenticateClient Error Test Cases - Specification vs Implementation

## SGP.23 Section 4.3.14.2.2 Specified Error Tests

Total specified: 21 test sequences (with #18 VOID), so 20 actual tests

### Currently Implemented Tests

Based on the execute statements and test case definitions found:

1. **TC_SM_DP_ES9_AuthenticateClientNIST_Error_InvalidSignature** 
   - Maps to: Test Sequence #5 (Invalid eUICC Signature - 8.1/6.1)
   
2. **TC_SM_DP_ES9_AuthenticateClientNIST_Error_InvalidServerChallenge**
   - Maps to: Test Sequence #6 (Invalid Server Challenge - 8.1/6.1)
   
3. **TC_SM_DP_ES9_AuthenticateClientNIST_Error_ProfileNotReleased**
   - Maps to: Test Sequence #8 (Profile not released - 8.2/1.2)
   
4. **TC_SM_DP_ES9_AuthenticateClientNIST_Error_UnknownTransactionId**
   - Maps to: Test Sequence #9 (Unknown Transaction ID in JSON - 8.10.1/3.9)
   
5. **TC_SM_DP_ES9_AuthenticateClientNIST_Error_TransactionIdMismatchASN1**
   - Maps to: Test Sequence #10 (Unknown Transaction ID in ASN.1 - 8.10.1/3.9)
   
6. **TC_SM_DP_ES9_AuthenticateClientNIST_Error_UnmatchedMatchingID**
   - Maps to: Test Sequence #11 (Invalid Matching ID for Default DP+ - 8.2.6/3.8)
   
7. **TC_SM_DP_ES9_AuthenticateClientNIST_Error_Mismatched_Transaction_ID**
   - Appears to be redundant with #10 above
   
8. **TC_SM_DP_ES9_AuthenticateClientNIST_Error_eUICC_Challenge_Reuse**
   - Not specified in SGP.23 - appears to be an additional test

### Missing Test Cases

The following test sequences from SGP.23 are NOT implemented:

1. **Test Sequence #1**: Invalid EUM Certificate (8.1.2/6.1)
2. **Test Sequence #2**: Expired EUM Certificate (8.1.2/6.3)
3. **Test Sequence #3**: Invalid eUICC Certificate (8.1.3/6.1)
4. **Test Sequence #4**: Expired eUICC Certificate (8.1.3/6.3)
5. **Test Sequence #7**: Unknown CI Public Key (8.11.1/3.9)
6. **Test Sequence #12**: Invalid Matching ID for Activation Code (8.2.6/3.8)
7. **Test Sequence #13**: Invalid Matching ID for SM-DS (8.2.6/3.8)
8. **Test Sequence #14**: Un-matched EID for Default DP+ (8.1.1/3.8)
9. **Test Sequence #15**: No Eligible Profile (8.2.5/4.3)
10. **Test Sequence #16**: Download Order Expired (8.8.5/4.10)
11. **Test Sequence #17**: Maximum retries exceeded (8.8.5/6.4)
12. **Test Sequence #19**: Un-matched EID for SM-DS (8.1.1/3.8)
13. **Test Sequence #20**: Un-matched EID for Activation Code (8.1.1/3.8)
14. **Test Sequence #21**: Invalid MatchingId for Activation Code not associated to EID (8.2.6/3.8)

## Summary

- **Specified tests**: 20 (21 with one VOID)
- **Implemented tests**: 6 that match specification + 2 additional
- **Missing tests**: 14

## Implementation Priority

The missing tests can be categorized by complexity:

### High Priority (Certificate-related):
- Test #1-4: Certificate validation errors (EUM and eUICC)
- Test #7: Unknown CI Public Key

### Medium Priority (Business logic):
- Test #12, #13, #21: Additional Matching ID scenarios
- Test #14, #19, #20: EID mismatch scenarios
- Test #15: No Eligible Profile
- Test #16: Download Order Expired
- Test #17: Maximum retries exceeded

These missing tests are critical for comprehensive validation of the AuthenticateClient error handling as specified in SGP.23.