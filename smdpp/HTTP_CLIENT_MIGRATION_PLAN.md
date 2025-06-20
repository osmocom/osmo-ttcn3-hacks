# HTTP Client Migration Plan

## Overview
Replace the current TITAN HTTP test port with the HttpClient class from smdvalcpp2.cpp for more flexibility and better integration with the existing certificate infrastructure.

## Key Principles
1. **JSON stays in TTCN-3**: C++ code handles only raw strings (HTTP body), no JSON parsing
2. **Flexible TLS verification**: Support both custom certificates AND public CA bundle
3. **Minimal changes**: Keep existing JSON codec infrastructure

## Design

### RSPClient HTTP Methods
```cpp
class RSPClient {
    // Existing members...
    HttpClient* httpClient;
    bool useCustomTlsCert;  // Flag for custom vs public CA
    
    // New method - returns raw HTTP response body as string
    std::string sendHttpsPost(
        const std::string& endpoint,  // e.g., "/gsma/rsp2/es9plus/initiateAuthentication"
        const std::string& body,      // Raw JSON string from TTCN-3
        int& httpStatusCode           // Output: HTTP status code
    );
};
```

### TTCN-3 External Functions
```ttcn
// Simple wrapper - takes charstring, returns charstring
external function ext_RSPClient_sendHttpsPost(
    integer handle,
    charstring endpoint,
    charstring body,
    out integer statusCode
) return charstring;
```

## Certificate Handling Strategy

### Dual-mode TLS Verification
1. **Custom Certificate Mode** (for test SM-DP+ servers):
   - Use existing certificate store from RSPClient
   - Add server's TLS certificate to verification chain
   - Similar to current `sslCtxFunction` approach

2. **Public CA Mode** (for production-like servers):
   - Use system's default CA bundle
   - Standard curl certificate verification
   - No custom SSL context needed

Configuration in TTCN-3:
```ttcn
// Module parameter to control TLS mode
modulepar boolean mp_use_custom_tls_cert := true;
modulepar charstring mp_custom_tls_cert_path := "./sgp26/DPtls/CERT_DP_TLS.der";
```

## Implementation Flow

### TTCN-3 Side (minimal changes)
```ttcn
// Current flow (keep as-is):
var charstring json_enc := oct2char(enc_JSON_ES9p_RemoteProfileProvisioningRequest(json_req));

// Replace only the HTTP transport:
// OLD: response_http := f_http_tx_request(...) + f_http_rx_response(...)
// NEW:
var integer http_status;
var charstring json_resp := ext_RSPClient_sendHttpsPost(
    g_rsp_client_handle,
    "/gsma/rsp2/es9plus/initiateAuthentication",
    json_enc,
    http_status
);

// Continue with existing JSON decoding:
var JSON_ES9p_RemoteProfileProvisioningResponse resp := 
    dec_JSON_ES9p_RemoteProfileProvisioningResponse(char2oct(json_resp));
```

### C++ Side (smdpp_Tests_Functions.cc)
```cpp
CHARSTRING ext__RSPClient__sendHttpsPost(
    INTEGER handle,
    const CHARSTRING& endpoint,
    const CHARSTRING& body,
    INTEGER& statusCode
) {
    RSPClient* client = getRSPClient(handle);
    int status;
    std::string response = client->sendHttpsPost(
        std::string(endpoint),
        std::string(body),
        status
    );
    statusCode = status;
    return CHARSTRING(response.c_str());
}
```

## RSPClient::sendHttpsPost Implementation
```cpp
std::string RSPClient::sendHttpsPost(const std::string& endpoint, 
                                     const std::string& body, 
                                     int& httpStatusCode) {
    if (!httpClient) {
        httpClient = new HttpClient();
    }
    
    std::string url = "https://" + serverHost + ":" + 
                      std::to_string(serverPort) + endpoint;
    
    HttpClient::ResponseData response;
    
    if (useCustomTlsCert) {
        // Use custom certificate verification with our cert store
        response = httpClient->postJsonWithCustomVerification(
            url, body, serverPort, certStore, certPool
        );
    } else {
        // Use standard curl CA bundle verification
        response = httpClient->postJsonWithStandardVerification(
            url, body, serverPort
        );
    }
    
    httpStatusCode = response.statusCode;
    return response.body;
}
```

## Key Advantages
1. **Minimal TTCN-3 changes**: Only replace HTTP transport calls
2. **Preserves JSON handling**: All JSON encoding/decoding stays in TTCN-3
3. **Flexible TLS**: Supports both test and production scenarios
4. **Reuses infrastructure**: Leverages existing HttpClient and certificate management
5. **Clear separation**: C++ handles HTTP/TLS, TTCN-3 handles protocol/JSON

## Migration Steps
1. Add `useCustomTlsCert` flag and TLS cert path to RSPClient initialization
2. Implement `sendHttpsPost` method in RSPClient
3. Add external function wrapper in smdpp_Tests_Functions.cc
4. Create helper function in TTCN-3 to replace HTTP adapter calls
5. Test with both custom and public certificates
6. Gradually migrate each HTTP call
7. Remove HTTP adapter dependencies once verified