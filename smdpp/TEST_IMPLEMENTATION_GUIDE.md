# SM-DP+ Test Implementation Guide

## Overview
This guide provides patterns and examples for implementing SM-DP+ test cases based on the existing infrastructure.

## Test Implementation Patterns

### 1. Basic Test Structure
```ttcn
// Handler function
private function f_TC_<TestName>() runs on smdpp_ConnHdlr {
    // Initialize RSP client
    f_rsp_client_init();
    
    // Perform test steps
    // ...
    
    // Cleanup
    f_rsp_client_cleanup();
    setverdict(pass);
}

// Test case
testcase TC_<Full_Test_Name>() runs on MTC_CT {
    var charstring id := testcasename();
    var smdpp_ConnHdlrPars pars := f_init_pars();
    var smdpp_ConnHdlr vc_conn;
    
    f_init(id);
    vc_conn := f_start_handler(refers(f_TC_<TestName>), pars);
    vc_conn.done;
    setverdict(pass);
}
```

### 2. Error Test Pattern
```ttcn
private function f_TC_<TestName>_Error() runs on smdpp_ConnHdlr {
    f_rsp_client_init();
    
    // Create request with invalid data
    var RemoteProfileProvisioningRequest req := {
        // ... request with error condition
    };
    
    // Expect error response
    var JSON_ESx_FunctionExecutionStatusCodeData error;
    error := f_es9p_transceive_error(req);
    
    // Validate error codes
    f_validate_error_response(error, "8.1.1", "3.9", "Invalid Transaction ID");
    
    f_rsp_client_cleanup();
    setverdict(pass);
}
```

### 3. Confirmation Code Test Pattern
```ttcn
// In test handler after AuthenticateClient response:
if (authResponse.ccRequiredFlag) {
    ext_logInfo("Confirmation code required flag is set");
    
    // Set transaction ID and confirmation code
    ext_RSPClient_setTransactionId(g_rsp_client_handle, g_transaction_id);
    ext_RSPClient_setConfirmationCode(g_rsp_client_handle, c_test_confirmation_code);
    
    // Get computed hash
    var octetstring confirmationCodeHash := ext_RSPClient_getConfirmationCodeHash(g_rsp_client_handle);
    
    // Add to PrepareDownloadRequest
    var PrepareDownloadRequestEs9 prepareReq := {
        smdpSigned2 := {
            // ... other fields
            hashCc := confirmationCodeHash
        },
        // ... other fields
    };
}
```

### 4. Certificate Validation Pattern
```ttcn
// Validate certificate chain
f_validate_certificate_roles(serverCert1, serverCert2);
f_validate_ein_permissions(eumCerts, g_eid);
f_validate_pkid_match(eumCerts, g_euiccInfo1);
```

### 5. Common Request Builders

#### InitiateAuthentication
```ttcn
var RemoteProfileProvisioningRequest req := {
    initiateAuthenticationRequest := {
        euiccChallenge := f_rnd_octstring(16),
        euiccInfo1 := g_euiccInfo1,
        smdpAddress := c_test_server_address
    }
};
```

#### AuthenticateClient
```ttcn
var RemoteProfileProvisioningRequest req := {
    authenticateClientRequest := {
        transactionId := g_transaction_id,
        authenticateServerResponse := {
            euiccSigned1 := {
                transactionId := g_transaction_id,
                serverAddress := g_server_address,
                serverChallenge := g_server_challenge,
                euiccInfo2 := g_euiccInfo2,
                ctxParams1 := g_ctxParams1
            },
            euiccSignature1 := g_euicc_signature1,
            euiccCertificate := g_euicc_cert,
            eumCertificate := g_eum_cert
        }
    }
};
```

#### GetBoundProfilePackage
```ttcn
var RemoteProfileProvisioningRequest req := {
    getBoundProfilePackageRequest := {
        transactionId := g_transaction_id,
        prepareDownloadResponse := {
            euiccSigned2 := {
                transactionId := g_transaction_id,
                euiccInfo2 := g_euiccInfo2,
                hashCc := confirmationCodeHash  // if required
            },
            euiccSignature2 := euiccSignature2
        }
    }
};
```

### 6. Error Code Reference
Common error codes to test:
- **8.1.1/3.1**: Invalid EID
- **8.1.1/3.9**: Unknown/Invalid Transaction ID  
- **8.1.2/3.2**: Certificate verification failed
- **8.2.1/3.7**: Missing confirmation code
- **8.2.1/5.1**: Confirmation code mismatch
- **8.2.2/3.1**: Missing data (profile unavailable)
- **8.8.1/3.8**: Invalid SM-DP+ Address
- **8.10.1/4.2**: No active RSP session
- **8.11.1/6.1**: EIN mismatch

### 7. Test Data Management
```ttcn
// Save state between operations
g_transaction_id := authResponse.transactionId;
g_server_challenge := authResponse.serverChallenge;
g_euicc_challenge := req.initiateAuthenticationRequest.euiccChallenge;

// Generate random data
var octetstring randomChallenge := f_rnd_octstring(16);
var hexstring randomEid := f_rnd_hexstring(32);
```

### 8. Retry Test Pattern
```ttcn
// First attempt
var RemoteProfileProvisioningRequest req1 := { /* ... */ };
var RemoteProfileProvisioningResponse resp1 := f_es9p_transceive_success(req1);

// Save state
var octetstring firstChallenge := resp1.serverChallenge;

// Retry with same/different parameters
var RemoteProfileProvisioningRequest req2 := { /* ... */ };
var RemoteProfileProvisioningResponse resp2 := f_es9p_transceive_success(req2);

// Validate retry behavior
if (sameChallenge) {
    if (resp2.serverChallenge != firstChallenge) {
        setverdict(fail, "Expected same challenge on retry");
    }
} else {
    if (resp2.serverChallenge == firstChallenge) {
        setverdict(fail, "Expected different challenge on retry");
    }
}
```

### 9. Session Cancellation Pattern
```ttcn
// Start session
f_create_initiate_authentication_request();
// ... authenticate client if needed

// Cancel session
var RemoteProfileProvisioningRequest cancelReq := {
    cancelSessionRequest := {
        transactionId := g_transaction_id,
        cancelSessionResponse := {
            euiccCancellationResult := {
                transactionId := g_transaction_id,
                cancelSessionResponseOk := { }
            }
        }
    }
};

var RemoteProfileProvisioningResponse resp := f_es9p_transceive_success(cancelReq);
```

## Testing Best Practices

1. **Always initialize and cleanup**: Use `f_rsp_client_init()` and `f_rsp_client_cleanup()`
2. **Track state**: Save transaction IDs, challenges, and other session data
3. **Use existing validators**: Leverage existing validation functions
4. **Follow naming conventions**: `f_TC_<TestName>` for handlers, `TC_<Full_Name>` for test cases
5. **Document special cases**: Add comments for non-obvious test scenarios
6. **Test both success and failure paths**: Ensure comprehensive coverage

## Next Steps for Implementation

1. Start with AuthenticateClient tests (build on existing InitiateAuthentication)
2. Move to GetBoundProfilePackage tests (complete flow validation)
3. Implement CancelSession tests (state management)
4. Add HandleNotification tests (ES2+ interface)
5. Complete with TLS tests (transport layer)
6. Finally add cryptographic variants (FRP, BRP)