# OpenSSL 3.0+ API Migration Guide

## Summary of Deprecated Functions and Their Replacements

### 1. EC_KEY Functions → EVP_PKEY API
- `EC_KEY_new()` → Use `EVP_EC_gen()` or `EVP_PKEY_new()`
- `EC_KEY_free()` → Use `EVP_PKEY_free()`
- `EC_KEY_set_group()` → Use `EVP_PKEY_fromdata()` with OSSL_PARAM
- `EC_KEY_generate_key()` → Use `EVP_PKEY_generate()`
- `EC_KEY_get0_public_key()` → Use `EVP_PKEY_get_bn_param()` or `EVP_PKEY_get_octet_string_param()`
- `EC_KEY_get0_group()` → Use `EVP_PKEY_get_group_name()`
- `EC_KEY_get1_EC_KEY()` → Use EVP_PKEY directly
- `EC_KEY_dup()` → Use `EVP_PKEY_dup()`

### 2. ECDH Functions → EVP_PKEY_derive API
- `ECDH_compute_key()` → Use `EVP_PKEY_derive()`

### 3. Initialization/Cleanup Functions
- `SSL_library_init()` → No longer needed in OpenSSL 3.0+
- `SSL_load_error_strings()` → No longer needed
- `ERR_load_crypto_strings()` → No longer needed
- `OpenSSL_add_all_algorithms()` → No longer needed
- `EVP_cleanup()` → No longer needed
- `CRYPTO_cleanup_all_ex_data()` → No longer needed
- `ERR_free_strings()` → No longer needed

## Detailed Code Changes

### 1. Fix EC_KEY Usage in generateEUICCOtpk()

**OLD CODE:**
```cpp
void generateEUICCOtpk() {
    std::unique_ptr<EC_KEY, EC_KEY_Deleter> ecKey(EC_KEY_new());
    if (!ecKey) {
        throw OpenSSLError("Failed to create EC_KEY");
    }
    
    std::unique_ptr<EC_GROUP, decltype(&EC_GROUP_free)> group(
        EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1), EC_GROUP_free);
    
    if (EC_KEY_set_group(ecKey.get(), group.get()) != 1) {
        throw OpenSSLError("Failed to set EC group");
    }
    
    if (EC_KEY_generate_key(ecKey.get()) != 1) {
        throw OpenSSLError("Failed to generate EC key pair");
    }
    // ... rest of the code
}
```

**NEW CODE:**
```cpp
void generateEUICCOtpk() {
    // Generate P-256 key pair directly
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> pctx(
        EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr), EVP_PKEY_CTX_free);
    if (!pctx) {
        throw OpenSSLError("Failed to create EVP_PKEY_CTX");
    }
    
    if (EVP_PKEY_keygen_init(pctx.get()) <= 0) {
        throw OpenSSLError("Failed to initialize key generation");
    }
    
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx.get(), NID_X9_62_prime256v1) <= 0) {
        throw OpenSSLError("Failed to set P-256 curve");
    }
    
    EVP_PKEY* pkey_raw = nullptr;
    if (EVP_PKEY_keygen(pctx.get(), &pkey_raw) <= 0) {
        throw OpenSSLError("Failed to generate EC key pair");
    }
    
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> pkey(pkey_raw);
    
    // Extract public key in uncompressed format
    size_t pub_len = 0;
    if (EVP_PKEY_get_octet_string_param(pkey.get(), "encoded-pub-key", 
                                        nullptr, 0, &pub_len) != 1) {
        throw OpenSSLError("Failed to get public key length");
    }
    
    m_euiccOtpk.resize(pub_len);
    if (EVP_PKEY_get_octet_string_param(pkey.get(), "encoded-pub-key",
                                        m_euiccOtpk.data(), pub_len, &pub_len) != 1) {
        throw OpenSSLError("Failed to get public key");
    }
    
    // Store the private key for later use
    m_euicc_ot_PrivateKey = std::move(pkey);
    
    Logger::info("Generated eUICC OtPK (P-256): " + HexUtil::bytesToHex(m_euiccOtpk));
}
```

### 2. Fix ECDH_compute_key Usage

**OLD CODE:**
```cpp
std::vector<uint8_t> computeECDHSharedSecret(const std::vector<uint8_t> &otherPublicKey) {
    // ... setup code ...
    
    if (ECDH_compute_key(shared_secret.data(), secret_len, other_point.get(), 
                         m_euicc_ot_PrivateKey.get(), nullptr) <= 0) {
        throw OpenSSLError("ECDH computation failed");
    }
    
    return shared_secret;
}
```

**NEW CODE:**
```cpp
std::vector<uint8_t> computeECDHSharedSecret(const std::vector<uint8_t> &otherPublicKey) {
    if (!m_euicc_ot_PrivateKey) {
        throw std::runtime_error("eUICC ephemeral private key not available");
    }
    
    // Create EVP_PKEY for the other party's public key
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> pctx(
        EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr), EVP_PKEY_CTX_free);
    
    if (!pctx || EVP_PKEY_fromdata_init(pctx.get()) <= 0) {
        throw OpenSSLError("Failed to initialize public key import");
    }
    
    // Build OSSL_PARAM array for public key
    OSSL_PARAM_BLD* param_bld = OSSL_PARAM_BLD_new();
    if (!param_bld) {
        throw OpenSSLError("Failed to create OSSL_PARAM_BLD");
    }
    
    if (!OSSL_PARAM_BLD_push_utf8_string(param_bld, "group", "prime256v1", 0) ||
        !OSSL_PARAM_BLD_push_octet_string(param_bld, "pub", 
                                          otherPublicKey.data(), otherPublicKey.size())) {
        OSSL_PARAM_BLD_free(param_bld);
        throw OpenSSLError("Failed to build parameters");
    }
    
    OSSL_PARAM* params = OSSL_PARAM_BLD_to_param(param_bld);
    OSSL_PARAM_BLD_free(param_bld);
    
    EVP_PKEY* other_pkey_raw = nullptr;
    if (EVP_PKEY_fromdata(pctx.get(), &other_pkey_raw, EVP_PKEY_PUBLIC_KEY, params) <= 0) {
        OSSL_PARAM_free(params);
        throw OpenSSLError("Failed to create public key from data");
    }
    OSSL_PARAM_free(params);
    
    std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> other_pkey(other_pkey_raw);
    
    // Perform ECDH key derivation
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> derive_ctx(
        EVP_PKEY_CTX_new(m_euicc_ot_PrivateKey.get(), nullptr), EVP_PKEY_CTX_free);
    
    if (!derive_ctx || EVP_PKEY_derive_init(derive_ctx.get()) <= 0) {
        throw OpenSSLError("Failed to initialize key derivation");
    }
    
    if (EVP_PKEY_derive_set_peer(derive_ctx.get(), other_pkey.get()) <= 0) {
        throw OpenSSLError("Failed to set peer key");
    }
    
    // Get shared secret length
    size_t secret_len = 0;
    if (EVP_PKEY_derive(derive_ctx.get(), nullptr, &secret_len) <= 0) {
        throw OpenSSLError("Failed to get shared secret length");
    }
    
    // Derive shared secret
    std::vector<uint8_t> shared_secret(secret_len);
    if (EVP_PKEY_derive(derive_ctx.get(), shared_secret.data(), &secret_len) <= 0) {
        throw OpenSSLError("Failed to derive shared secret");
    }
    
    shared_secret.resize(secret_len);
    Logger::info("Computed ECDH shared secret: " + HexUtil::bytesToHex(shared_secret));
    return shared_secret;
}
```

### 3. Fix EC_KEY Usage in Public Key Operations

**OLD CODE:**
```cpp
EC_KEY *ec_key = EVP_PKEY_get1_EC_KEY(pubKey.get());
if (!ec_key) {
    Logger::warning("Failed to extract EC key");
    return false;
}

std::unique_ptr<EC_KEY, void (*)(EC_KEY *)> ec_key_guard(ec_key, EC_KEY_free);

const EC_POINT *pub_point = EC_KEY_get0_public_key(ec_key);
const EC_GROUP *group = EC_KEY_get0_group(ec_key);
```

**NEW CODE:**
```cpp
// Get public key directly from EVP_PKEY
size_t pub_len = 0;
if (EVP_PKEY_get_octet_string_param(pubKey.get(), "encoded-pub-key", 
                                    nullptr, 0, &pub_len) != 1) {
    Logger::warning("Failed to get public key length");
    return false;
}

publicKeyStorage.resize(pub_len);
if (EVP_PKEY_get_octet_string_param(pubKey.get(), "encoded-pub-key",
                                    publicKeyStorage.data(), pub_len, &pub_len) != 1) {
    Logger::warning("Failed to extract public key data");
    return false;
}
```

### 4. Fix Initialization and Cleanup

**OLD CODE:**
```cpp
// In constructor
OpenSSL_add_all_algorithms();
ERR_load_crypto_strings();

// In destructor
EVP_cleanup();
CRYPTO_cleanup_all_ex_data();
ERR_free_strings();

// In main()
SSL_library_init();
SSL_load_error_strings();
ERR_load_crypto_strings();
OpenSSL_add_all_algorithms();
```

**NEW CODE:**
```cpp
// Remove all initialization calls - OpenSSL 3.0+ handles this automatically
// In constructor - remove OpenSSL_add_all_algorithms() and ERR_load_crypto_strings()

// In destructor - remove all cleanup calls

// In main() - remove all initialization calls
```

### 5. Update Member Variables

**OLD CODE:**
```cpp
std::unique_ptr<EC_KEY, EC_KEY_Deleter> m_euicc_ot_PrivateKey;
```

**NEW CODE:**
```cpp
std::unique_ptr<EVP_PKEY, EVP_PKEY_Deleter> m_euicc_ot_PrivateKey;
```

### 6. Remove EC_KEY_Deleter

Since we're no longer using EC_KEY, remove the EC_KEY_Deleter from helpers.h:

```cpp
// Remove this struct:
struct EC_KEY_Deleter {
    void operator()(EC_KEY *key) const {
        EC_KEY_free(key);
    }
};
```

## Additional Notes

1. **OSSL_PARAM**: OpenSSL 3.0+ uses OSSL_PARAM for passing parameters to many functions. This is a more flexible and type-safe approach than the old APIs.

2. **EVP_PKEY**: The EVP_PKEY type is now the primary abstraction for all types of keys (RSA, EC, etc.). Direct manipulation of EC_KEY is discouraged.

3. **Automatic Initialization**: OpenSSL 3.0+ automatically initializes itself when needed, so explicit initialization calls are no longer required.

4. **Error Handling**: The new APIs generally return <= 0 on error (not just 0), so error checks should use `<= 0` rather than `== 0`.

5. **Memory Management**: The new APIs often allocate memory that must be freed with specific functions (e.g., OSSL_PARAM_free, OSSL_PARAM_BLD_free).