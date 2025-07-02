# Refactoring Examples - Before and After

## Example 1: Simple Setter Function

### Before (17 lines):
```cpp
INTEGER ext__RSPClient__setConfirmationCode(const INTEGER& clientHandle, const OCTETSTRING& code) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);
        
        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }
        
        std::vector<uint8_t> codeBytes = octetstring_to_bytes(code);
        client->setConfirmationCode(codeBytes);
        
        return INTEGER(0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__setConfirmationCode failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}
```

### After (3 lines):
```cpp
INTEGER ext__RSPClient__setConfirmationCode(const INTEGER& clientHandle, const OCTETSTRING& code) {
    return client_setter<OCTETSTRING, std::vector<uint8_t>, &RSPClient::setConfirmationCode>(
        clientHandle, "ext__RSPClient__setConfirmationCode", code, octetstring_to_bytes);
}
```

**Reduction: 82%**

## Example 2: Certificate Utility Function

### Before (11 lines):
```cpp
BOOLEAN ext__CertificateUtil__hasRSPRole(const OCTETSTRING& cert) {
    try {
        std::vector<uint8_t> certData = octetstring_to_bytes(cert);
        bool hasRole = CertificateUtil::hasRSPRole(certData);
        return BOOLEAN(hasRole);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__CertificateUtil__hasRSPRole failed: " + std::string(e.what()));
        return BOOLEAN(false);
    }
}
```

### After (1 line):
```cpp
BOOLEAN ext__CertificateUtil__hasRSPRole(const OCTETSTRING& cert) {
    return cert_bool_wrapper<CertificateUtil::hasRSPRole>("ext__CertificateUtil__hasRSPRole", cert);
}
```

**Reduction: 91%**

## Example 3: Client Operation

### Before (16 lines):
```cpp
OCTETSTRING ext__RSPClient__generateChallenge(const INTEGER& clientHandle) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);
        
        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return OCTETSTRING();
        }
        
        std::vector<uint8_t> challenge = client->generateChallenge();
        return bytes_to_octetstring(challenge);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__generateChallenge failed: " + std::string(e.what()));
        return OCTETSTRING();
    }
}
```

### After (3 lines):
```cpp
OCTETSTRING ext__RSPClient__generateChallenge(const INTEGER& clientHandle) {
    return client_getter<std::vector<uint8_t>, OCTETSTRING, &RSPClient::generateChallenge>(
        clientHandle, "ext__RSPClient__generateChallenge", OCTETSTRING(), bytes_to_octetstring);
}
```

**Reduction: 81%**

## Example 4: Complex Operation with Lambda

### Before (22 lines):
```cpp
INTEGER ext__RSPClient__verifyServerSignature(const INTEGER& clientHandle,
                                            const OCTETSTRING& data,
                                            const OCTETSTRING& signature,
                                            const OCTETSTRING& cert) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);
        
        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            return INTEGER(-1);
        }
        
        std::vector<uint8_t> dataBytes = octetstring_to_bytes(data);
        std::vector<uint8_t> sigBytes = octetstring_to_bytes(signature);
        std::vector<uint8_t> certBytes = octetstring_to_bytes(cert);
        
        bool verified = client->verifyServerSignature(dataBytes, sigBytes, certBytes);
        return INTEGER(verified ? 1 : 0);
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__verifyServerSignature failed: " + std::string(e.what()));
        return INTEGER(-1);
    }
}
```

### After (10 lines):
```cpp
INTEGER ext__RSPClient__verifyServerSignature(const INTEGER& clientHandle,
                                            const OCTETSTRING& data,
                                            const OCTETSTRING& signature,
                                            const OCTETSTRING& cert) {
    return with_client(clientHandle, "ext__RSPClient__verifyServerSignature", INTEGER(-1),
        [&](RSPClient* client) {
            bool verified = client->verifyServerSignature(
                octetstring_to_bytes(data),
                octetstring_to_bytes(signature),
                octetstring_to_bytes(cert));
            return INTEGER(verified ? 1 : 0);
        });
}
```

**Reduction: 55%**

## Example 5: HTTP Operation

### Before (35 lines):
```cpp
CHARSTRING ext__RSPClient__sendHttpsPost(const INTEGER& clientHandle,
                                        const CHARSTRING& url,
                                        const CHARSTRING& contentType,
                                        const OCTETSTRING& body,
                                        INTEGER& statusCode) {
    try {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);
        
        if (!client) {
            LOG_ERROR("Invalid RSP client handle: " + std::to_string(handle));
            statusCode = INTEGER(0);
            return CHARSTRING("");
        }
        
        std::string urlStr = charstring_to_string(url);
        std::string contentTypeStr = charstring_to_string(contentType);
        std::vector<uint8_t> bodyData = octetstring_to_bytes(body);
        
        HttpResponse response = client->sendHttpsPost(urlStr, contentTypeStr, bodyData);
        
        statusCode = INTEGER(response.statusCode);
        
        if (response.statusCode == 200) {
            return string_to_charstring(response.body);
        } else {
            LOG_ERROR("HTTP request failed with status: " + std::to_string(response.statusCode));
            return CHARSTRING("");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("ext__RSPClient__sendHttpsPost failed: " + std::string(e.what()));
        statusCode = INTEGER(0);
        return CHARSTRING("");
    }
}
```

### After (15 lines):
```cpp
CHARSTRING ext__RSPClient__sendHttpsPost(const INTEGER& clientHandle,
                                        const CHARSTRING& url,
                                        const CHARSTRING& contentType,
                                        const OCTETSTRING& body,
                                        INTEGER& statusCode) {
    return safe_execute("ext__RSPClient__sendHttpsPost", CHARSTRING(""),
        [&]() {
            statusCode = INTEGER(0);
            
            auto response = with_client(clientHandle, "", HttpResponse{0, ""},
                [&](RSPClient* client) {
                    return client->sendHttpsPost(
                        charstring_to_string(url),
                        charstring_to_string(contentType),
                        octetstring_to_bytes(body));
                });
            
            statusCode = INTEGER(response.statusCode);
            if (response.statusCode != 200) {
                throw std::runtime_error("HTTP request failed with status: " + 
                                       std::to_string(response.statusCode));
            }
            return string_to_charstring(response.body);
        });
}
```

**Reduction: 57%**

## Summary of Benefits

1. **Dramatic Code Reduction**: 55-91% reduction in lines of code
2. **Consistent Error Handling**: All functions use the same pattern
3. **Type Safety**: Templates ensure correct type conversions
4. **Maintainability**: Changes to error handling only need template updates
5. **Readability**: Intent is clearer with descriptive template names
6. **Zero Runtime Overhead**: Templates are compile-time constructs