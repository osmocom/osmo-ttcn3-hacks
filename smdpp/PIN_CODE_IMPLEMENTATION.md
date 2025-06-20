# PIN Code (Confirmation Code) Implementation Plan

## Overview
The RSP specification (SGP.22) defines a Confirmation Code mechanism to ensure user consent for profile downloads. This document outlines the implementation plan for integrating this feature into the SM-DP+ test suite.

## Specification Details (SGP.22)

### Hash Calculation
According to SGP.22, if required, the LPAd SHALL calculate the hash of the Confirmation Code as follows:
```
Hashed Confirmation Code = SHA256(SHA256(Confirmation Code) | TransactionID)
```
Where:
- `|` means concatenation of data
- Confirmation Code is the user-provided PIN (typically 8 digits)
- TransactionID is the binary transaction ID (16 bytes)

### Protocol Flow
1. SM-DP+ sets `ccRequiredFlag = true` in `SmdpSigned2` during `AuthenticateClient`
2. eUICC/LPA prompts user for Confirmation Code
3. eUICC/LPA calculates the hash as specified above
4. eUICC includes `hashCc` in `PrepareDownloadRequest`
5. SM-DP+ verifies the hash before proceeding with profile download

## Current State Analysis

### Python Server (osmo-smdpp.py)
- Line 578: `ccRequiredFlag` is hardcoded to `False`
- Line 648: TODO comment about checking if confirmation code verification is required
- No actual implementation of confirmation code verification

### C++ Implementation (smdvalcpp2.cpp)
- Has `setConfirmationCodeHash()` method to store the hash
- Stores it in `m_confirmationCodeHash` member variable
- No actual verification logic implemented

### TTCN-3 Test
- No confirmation code handling in the current test flow

## Implementation Plan

### Phase 1: Hash Calculation Implementation

#### C++ Side (smdvalcpp2.cpp or new file)
```cpp
// Compute confirmation code hash according to SGP.22
std::vector<uint8_t> computeConfirmationCodeHash(
    const std::string& confirmationCode,
    const std::vector<uint8_t>& transactionId) {
    
    // Step 1: SHA256(Confirmation Code)
    std::vector<uint8_t> firstHash = computeSHA256(confirmationCode);
    
    // Step 2: Concatenate with TransactionID
    std::vector<uint8_t> concatenated = firstHash;
    concatenated.insert(concatenated.end(), transactionId.begin(), transactionId.end());
    
    // Step 3: SHA256(SHA256(CC) | TransactionID)
    return computeSHA256(concatenated);
}
```

#### Python Side (for server verification)
```python
def compute_confirmation_code_hash(confirmation_code: str, transaction_id: bytes) -> bytes:
    """Compute hash according to SGP.22 specification."""
    import hashlib
    
    # Step 1: SHA256(Confirmation Code)
    first_hash = hashlib.sha256(confirmation_code.encode('utf-8')).digest()
    
    # Step 2: SHA256(SHA256(CC) | TransactionID)
    return hashlib.sha256(first_hash + transaction_id).digest()
```

### Phase 2: Server-Side Support (Python)

1. **Add configuration for confirmation code requirement:**
```python
# In SmDppHttpServer.__init__
self.confirmation_codes = {}  # Mapping of matchingId to expected CC
self.require_confirmation_code = False  # Global flag, can be overridden per profile
```

2. **Store expected confirmation code during profile metadata:**
```python
# In authenticateClient, after determining matchingId
if matchingId in self.confirmation_codes:
    ss.expected_confirmation_code = self.confirmation_codes[matchingId]
    ss.ccRequiredFlag = True
else:
    ss.ccRequiredFlag = self.require_confirmation_code
    ss.expected_confirmation_code = "12345678"  # Default for testing
```

3. **Modify authenticateClient to set ccRequiredFlag:**
```python
# Line 578 in osmo-smdpp.py
'ccRequiredFlag': ss.ccRequiredFlag
```

4. **Implement verification in getBoundProfilePackage:**
```python
# After parsing PrepareDownloadRequest
if ss.ccRequiredFlag:
    if 'hashCc' not in r_ok:
        raise ApiError('8.2.1', '3.7', 'Confirmation code required but not provided')
    
    received_hash = r_ok['hashCc']
    expected_hash = compute_confirmation_code_hash(
        ss.expected_confirmation_code,
        h2b(transactionId)
    )
    
    if received_hash != expected_hash:
        raise ApiError('8.2.2', '3.7', 'Invalid confirmation code')
```

### Phase 3: Client-Side Support (C++)

1. **Extend RSPClient class:**
```cpp
class RSPClient {
private:
    bool m_ccRequired = false;
    std::string m_confirmationCode;
    std::vector<uint8_t> m_transactionId;
    
public:
    void setConfirmationCode(const std::string& code) {
        m_confirmationCode = code;
    }
    
    void setCcRequired(bool required) {
        m_ccRequired = required;
    }
    
    void setTransactionId(const std::vector<uint8_t>& tid) {
        m_transactionId = tid;
    }
    
    std::vector<uint8_t> getConfirmationCodeHash() {
        if (m_confirmationCode.empty() || m_transactionId.empty()) {
            return {};
        }
        return computeConfirmationCodeHash(m_confirmationCode, m_transactionId);
    }
};
```

### Phase 4: TTCN-3 Test Integration

1. **Add external functions:**
```ttcn
external function ext_RSPClient_setConfirmationCode(
    integer clientHandle,
    charstring confirmationCode
) return integer;

external function ext_RSPClient_setTransactionId(
    integer clientHandle,
    octetstring transactionId
) return integer;
```

2. **Modify test flow:**
```ttcn
// After receiving AuthenticateClientOk
if (auok.smdpSigned2.ccRequiredFlag == true) {
    ext_logInfo("Confirmation code is required for this profile");
    
    // Store transaction ID in client
    var octetstring tidBytes := hex2oct(auok.transactionId);
    ext_RSPClient_setTransactionId(g_rsp_client_handle, tidBytes);
    
    // Set confirmation code (from test parameters or default)
    var charstring confirmationCode := "12345678";  // Default test code
    if (ispresent(g_pars_smdpp.confirmation_code)) {
        confirmationCode := g_pars_smdpp.confirmation_code;
    }
    ext_RSPClient_setConfirmationCode(g_rsp_client_handle, confirmationCode);
}

// When building PrepareDownloadRequest
if (auok.smdpSigned2.ccRequiredFlag == true) {
    var octetstring ccHash := ext_RSPClient_getConfirmationCodeHash(g_rsp_client_handle);
    if (lengthof(ccHash) == 32) {  // SHA256 is 32 bytes
        prepDownloadReq.hashCc := ccHash;
    }
}
```

### Phase 5: Testing Strategy

1. **Test Cases:**
   - CC not required (current behavior)
   - CC required with correct code
   - CC required with incorrect code
   - CC required but not provided
   - CC provided but not required (should be ignored)

2. **Test Confirmation Codes:**
   - Valid: "12345678"
   - Invalid: "87654321"
   - Edge cases: empty string, very long string

## Security Considerations

1. **No Storage of Plaintext:** Never store the confirmation code in plaintext
2. **Timing Attacks:** Use constant-time comparison for hash verification
3. **Transaction Binding:** The transaction ID in the hash prevents replay attacks
4. **User Interface:** In real implementation, ensure proper UI for code entry

## Implementation Order

1. Implement hash calculation functions in both C++ and Python
2. Add server-side verification in Python
3. Add client-side support in C++
4. Integrate into TTCN-3 tests
5. Add comprehensive test cases

## Notes

- Confirmation codes are typically 8-digit strings but the spec doesn't mandate this
- The double-hashing with transaction ID provides security against various attacks
- This implementation focuses on the cryptographic protocol, not the UI aspects