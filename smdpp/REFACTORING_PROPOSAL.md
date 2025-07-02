# smdpp_Tests_Functions.cc Refactoring Proposal

## Overview
The current implementation has ~40-50% code duplication across 50+ external functions. This proposal reduces duplication using C++ templates and helper classes.

## Template-Based Solution

### 1. Exception Handler Template
```cpp
template<typename ReturnType, typename Func>
ReturnType safe_execute(const char* func_name, ReturnType error_value, Func&& f) {
    try {
        return f();
    } catch (const std::exception& e) {
        LOG_ERROR(std::string(func_name) + " failed: " + e.what());
        return error_value;
    }
}

// Usage example:
INTEGER ext__RSPClient__createClient(...) {
    return safe_execute("ext__RSPClient__createClient", INTEGER(-1), [&]() {
        // Original function body
        return INTEGER(handle);
    });
}
```

### 2. Client Handle Validator Template
```cpp
template<typename ReturnType, typename Func>
ReturnType with_client(const INTEGER& clientHandle, const char* func_name, 
                      ReturnType error_value, Func&& f) {
    return safe_execute(func_name, error_value, [&]() {
        int handle = static_cast<int>(clientHandle);
        RSPClient* client = RSPClientRegistry::getInstance().getClient(handle);
        
        if (!client) {
            throw std::runtime_error("Invalid RSP client handle: " + std::to_string(handle));
        }
        
        return f(client);
    });
}

// Usage example:
INTEGER ext__RSPClient__loadEUICCCertificate(const INTEGER& clientHandle, const CHARSTRING& certPath) {
    return with_client(clientHandle, "ext__RSPClient__loadEUICCCertificate", INTEGER(-1), 
        [&](RSPClient* client) {
            client->loadEUICCCertificate(charstring_to_string(certPath));
            return INTEGER(0);
        });
}
```

### 3. Type Conversion Wrapper Templates
```cpp
// For functions that convert input and output
template<typename TtcnInput, typename CppInput, typename CppOutput, typename TtcnOutput>
TtcnOutput convert_wrapper(const char* func_name, TtcnOutput error_value,
                          const TtcnInput& input,
                          std::function<CppInput(const TtcnInput&)> input_converter,
                          std::function<CppOutput(const CppInput&)> operation,
                          std::function<TtcnOutput(const CppOutput&)> output_converter) {
    return safe_execute(func_name, error_value, [&]() {
        CppInput cpp_input = input_converter(input);
        CppOutput cpp_output = operation(cpp_input);
        return output_converter(cpp_output);
    });
}

// Usage for CertificateUtil functions:
BOOLEAN ext__CertificateUtil__hasRSPRole(const OCTETSTRING& cert) {
    return convert_wrapper<OCTETSTRING, std::vector<uint8_t>, bool, BOOLEAN>(
        "ext__CertificateUtil__hasRSPRole", BOOLEAN(false),
        cert,
        octetstring_to_bytes,
        [](const auto& data) { return CertificateUtil::hasRSPRole(data); },
        [](bool result) { return BOOLEAN(result); }
    );
}
```

### 4. Unified Logging Template
```cpp
template<void (*LogFunc)(const std::string&)>
void log_wrapper(const char* func_name, const CHARSTRING& message) {
    try {
        LogFunc(charstring_to_string(message));
    } catch (const std::exception& e) {
        std::cerr << func_name << " failed: " << e.what() << std::endl;
    }
}

// Replace all three logging functions:
void ext__logInfo(const CHARSTRING& message) {
    log_wrapper<Logger::info>("ext__logInfo", message);
}

void ext__logError(const CHARSTRING& message) {
    log_wrapper<Logger::error>("ext__logError", message);
}

void ext__logDebug(const CHARSTRING& message) {
    log_wrapper<Logger::debug>("ext__logDebug", message);
}
```

### 5. Generic Setter Template
```cpp
template<typename T, void (RSPClient::*SetterFunc)(const T&)>
INTEGER generic_setter(const INTEGER& clientHandle, const char* func_name,
                      const OCTETSTRING& value) {
    return with_client(clientHandle, func_name, INTEGER(-1),
        [&](RSPClient* client) {
            (client->*SetterFunc)(octetstring_to_bytes(value));
            return INTEGER(0);
        });
}

// Usage:
INTEGER ext__RSPClient__setConfirmationCode(const INTEGER& clientHandle, const OCTETSTRING& code) {
    return generic_setter<std::vector<uint8_t>, &RSPClient::setConfirmationCode>(
        clientHandle, "ext__RSPClient__setConfirmationCode", code);
}
```

### 6. Certificate Chain Verification Template
```cpp
template<typename VerifyFunc>
BOOLEAN verify_chain_template(const char* func_name,
                             const std::vector<OCTETSTRING>& cert_data,
                             VerifyFunc&& verify_func) {
    return safe_execute(func_name, BOOLEAN(false), [&]() {
        std::vector<std::unique_ptr<X509, X509Deleter>> certs;
        
        for (const auto& data : cert_data) {
            auto bytes = octetstring_to_bytes(data);
            certs.push_back(CertificateUtil::loadCertFromDER(bytes));
        }
        
        return BOOLEAN(verify_func(certs));
    });
}
```

## Benefits

1. **Code Reduction**: ~40-50% reduction in lines of code
2. **Consistency**: All error handling follows the same pattern
3. **Maintainability**: Changes to patterns only need to be made once
4. **Type Safety**: Templates provide compile-time type checking
5. **Readability**: Intent is clearer with descriptive template names

## Implementation Strategy

1. **Phase 1**: Implement core templates (safe_execute, with_client)
2. **Phase 2**: Convert simple functions (logging, getters/setters)
3. **Phase 3**: Convert complex functions (certificate operations)
4. **Phase 4**: Test thoroughly with existing test suite

## Estimated Impact

- Current: ~830 lines
- After refactoring: ~400-500 lines
- Reduction: ~40-50%

## Additional Improvements

1. **Constexpr for error messages**: Define common error messages as constants
2. **Type traits**: Use type traits for automatic type conversion selection
3. **Concepts (C++20)**: Add constraints to templates for better error messages