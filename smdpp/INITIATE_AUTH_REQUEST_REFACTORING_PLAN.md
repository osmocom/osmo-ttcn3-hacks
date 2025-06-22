# InitiateAuthentication Request Creation - Refactoring Plan

## Current State Analysis

### Function Overview
`f_create_initiate_authentication_request()` is called 10 times in the test suite:
- 9 times used as-is with default server address
- 1 time modified for invalid server address test

### Key Observations
1. The function is well-designed and encapsulates:
   - eUICC challenge generation
   - EUICCInfo1 structure creation  
   - Request assembly

2. Helper functions work correctly:
   - `f_get_ci_pkids_for_verification()` and `f_get_ci_pkids_for_signing()` both call `f_get_ci_pkids_list()`
   - Returns different PKIDs based on certificate type (NIST vs BRP)

3. Only variation: Invalid server address test modifies the request after creation

## Refactoring Decision: Minimal Change

### Option 1: Add Optional Parameter (RECOMMENDED)
Add an optional server address parameter with default value:

```ttcn
function f_create_initiate_authentication_request(
    template (omit) charstring smdp_address_override := omit
) runs on smdpp_ConnHdlr return RemoteProfileProvisioningRequest {
    
    /* Generate eUICC challenge using C++ crypto */
    g_euiccChallenge := ext_RSPClient_generateChallenge(g_rsp_client_handle);
    ext_logInfo("Generated eUICC challenge: " & ext_bytesToHex(g_euiccChallenge));
    
    /* Create EUICC_INFO1 using TTCN-3 native types */
    var EUICCInfo1 euiccInfo1 := {
        svn := '020200'O, /* SGP.22 v2.2.0 */
        euiccCiPKIdListForVerification := f_get_ci_pkids_for_verification(),
        euiccCiPKIdListForSigning := f_get_ci_pkids_for_signing()
    };
    
    /* Determine which server address to use */
    var charstring smdp_address;
    if (ispresent(smdp_address_override)) {
        smdp_address := valueof(smdp_address_override);
    } else {
        smdp_address := g_pars_smdpp.smdp_server_url;
    }
    
    /* Create the request structure */
    var RemoteProfileProvisioningRequest request := {
        initiateAuthenticationRequest := {
            euiccChallenge := g_euiccChallenge,
            smdpAddress := smdp_address,
            euiccInfo1 := euiccInfo1
        }
    };
    
    return request;
}
```

### Option 2: Keep Current Design (ALSO VALID)
The current design is clean and the one modification case is handled elegantly by modifying the returned structure. This is a valid TTCN-3 pattern.

## Recommendation

**Keep the current design**. Here's why:

1. **Simplicity**: The function has a single responsibility - create a standard request
2. **Clarity**: The invalid server test explicitly shows it's modifying the standard request
3. **YAGNI**: We don't need the flexibility for just one edge case
4. **Testability**: The modification pattern makes the test intent clearer

## Alternative Improvements (if desired)

1. **Document the pattern**: Add a comment explaining that the returned request can be modified
2. **Consider a constant**: Define the SVN version as a module constant rather than hardcoding '020200'O

## Conclusion

The current implementation is already well-designed. Adding parameters for a single edge case would add complexity without significant benefit. The explicit modification in the test makes the test's intent clearer than hiding it behind a parameter.