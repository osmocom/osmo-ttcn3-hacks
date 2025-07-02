# smdvalcpp2.cpp Function Usage Analysis

## Functions/Methods USED in smdpp_Tests_Functions.cc

### CertificateUtil Static Methods (USED)
1. `getEID()` - Used via ext__CertificateUtil__getEID
2. `loadCertFromDER()` - Used multiple times for certificate loading
3. `validateEIDRange()` - Used via ext__CertificateUtil__validateEIDRange
4. `hasRSPRole()` - Used via ext__CertificateUtil__hasRSPRole
5. `getPermittedEINs()` - Used via ext__CertificateUtil__getPermittedEINs
6. `verifyECDHCompatible()` - Used via ext__CertificateUtil__verifyECDHCompatible
7. `getCurveOID()` - Used via ext__CertificateUtil__getCurveOID
8. `verifyCertificateChainDynamic()` - Used via ext__CertificateUtil__verifyCertificateChainDynamic
9. `loadCertificatesFromDirectory()` - Used in RSPClientRegistry::createClient
10. `getSubjectKeyIdentifier()` - Used internally by RSPClient
11. `certToDER()` - Used internally by RSPClient getters
12. `xx_loadCertificate()` - Used internally by RSPClient
13. `xx_loadPrivateKey()` - Used internally by RSPClient
14. `xx_loadPublicKey()` - Used internally by RSPClient
15. `xx_generatePublicKeyFromPrivate()` - Used internally by RSPClient
16. `convertDER_to_BSI_TR03111()` - Used internally by RSPClient
17. `verify_TR031111()` - Used internally by RSPClient

### RSPClient Public Methods (USED)
1. `RSPClient()` constructor - Used in RSPClientRegistry::createClient
2. `setCACertPath()` - Used in RSPClientRegistry::createClient
3. `loadEUICCCertificate()` - Used in RSPClientRegistry::createClient and ext function
4. `loadEUICCKeyPair()` - Used in RSPClientRegistry::createClient and ext function
5. `loadEUMCertificate()` - Used in RSPClientRegistry::createClient and ext function
6. `loadEUMKeyPair()` - Used in RSPClientRegistry::createClient
7. `generateChallenge()` - Used via ext__RSPClient__generateChallenge
8. `generateEUICCOtpk()` - Used via ext__RSPClient__generateEUICCOtpk
9. `getEUICCOtpk()` - Used via ext__RSPClient__getEUICCOtpk
10. `computeECDHSharedSecret()` - Used via ext__RSPClient__computeSharedSecret
11. `signDataWithEUICC()` - Used via ext__RSPClient__signDataWithEUICC
12. `verifyServerSignature()` - Used via ext__RSPClient__verifyServerSignature
13. `setConfirmationCode()` - Used via ext__RSPClient__setConfirmationCode
14. `setTransactionId()` - Used via ext__RSPClient__setTransactionId
15. `getConfirmationCodeHash()` - Used via ext__RSPClient__getConfirmationCodeHash
16. `setConfirmationCodeHash()` - Used via ext__RSPClient__setConfirmationCodeHash
17. `getEUMCertificate()` - Used via ext__RSPClient__getEUMCertificate
18. `getEUICCCertificate()` - Used via ext__RSPClient__getEUICCCertificate
19. `getCICertificate()` - Used via ext__RSPClient__getCICertificate
20. `configureHttpClient()` - Used via ext__RSPClient__configureHttpClient
21. `sendHttpsPost()` - Used via ext__RSPClient__sendHttpsPost

### RSPClient Private/Internal Methods (USED internally)
1. `getCertificateCurveNID()` - Used internally
2. `getEUICCCurveNID()` - Used internally
3. `signDataWithKey()` - Used internally by signDataWithEUICC

### Helper Functions (USED)
1. `computeConfirmationCodeHash()` - Used internally by RSPClient

## Functions/Methods NOT USED in smdpp_Tests_Functions.cc

### CertificateUtil Static Methods (NOT USED from TTCN-3, but USED INTERNALLY)
1. `getSubjectName()` - Used internally for logging throughout the code
2. `getIssuerName()` - Not used at all (can be removed)
3. `getAuthorityKeyIdentifier()` - Used internally by findIssuerCertificate and buildCertificateChain
4. `loadCertFromPEM()` - Not used at all (can be removed)
5. `loadCertificateChain()` - Used internally by RSPClient constructor and loadCertificatesFromDirectory
6. `certToPEM()` - Not used at all (can be removed)
7. `isExpired()` - Not used at all (can be removed)
8. `isNotYetValid()` - Not used at all (can be removed)
9. `printCertificateDetails()` - Used internally by buildCertificateChain for debugging
10. `findIssuerCertificate()` - Used internally by buildCertificateChain
11. `buildCertificateChain()` - Not used at all (can be removed)
12. `find_cert_files()` - Used internally by loadCertificatesFromDirectory
13. `xx_loadCompleteKeySet()` (both overloads) - Used internally by loadAnyCompleteKeySet
14. `parse_permitted_eins_from_cert()` - Used internally by getPermittedEINs and validateEIDRange
15. `convertBSI_TR03111_to_DER()` - Used internally by verify_TR031111

### RSPClient Public Methods (NOT USED)
1. `loadAnyCompleteKeySet()` - Not called from TTCN-3 functions

### Other Classes/Structures (NOT USED)
1. `X509Deleter` - Used internally only
2. `EVP_PKEY_Deleter` - Used internally only
3. `EVP_MD_CTX_Deleter` - Used internally only
4. `EVP_PKEY_CTX_Deleter` - Used internally only
5. `OpenSSLError` - Used internally only

## Summary

**Used Functions**: The majority of the RSPClient public interface is used, along with key CertificateUtil static methods that are wrapped by external functions. Most internal/private methods are used indirectly.

**Unused Functions**: Many CertificateUtil utility methods are not directly used but may be useful for debugging or future features. Some are only used internally by other CertificateUtil methods.

## Recommendations for Code Cleanup

### Functions that CAN BE SAFELY REMOVED:
1. **`getIssuerName()`** - Completely unused
2. **`loadCertFromPEM()`** - Completely unused (only DER loading is used)
3. **`certToPEM()`** - Completely unused
4. **`isExpired()`** - Completely unused
5. **`isNotYetValid()`** - Completely unused
6. **`buildCertificateChain()`** - Completely unused (and with it, `findIssuerCertificate()` and `printCertificateDetails()` if not needed for debugging)
7. **`loadAnyCompleteKeySet()`** - Public method never called from TTCN-3

### Functions that MUST BE KEPT (used internally):
1. **`getSubjectName()`** - Used extensively for logging
2. **`getAuthorityKeyIdentifier()`** - Used by certificate chain building (if kept)
3. **`loadCertificateChain()`** - Used by RSPClient constructor
4. **`find_cert_files()`** - Used by loadCertificatesFromDirectory
5. **`xx_loadCompleteKeySet()`** - Used by loadAnyCompleteKeySet (if kept)
6. **`parse_permitted_eins_from_cert()`** - Used by getPermittedEINs and validateEIDRange
7. **`convertBSI_TR03111_to_DER()`** - Used by verify_TR031111

### Dependencies to Consider:
- If removing `buildCertificateChain()`, also remove:
  - `findIssuerCertificate()`
  - `printCertificateDetails()`
  - Parts of `getAuthorityKeyIdentifier()` usage
- If removing `loadAnyCompleteKeySet()`, also remove:
  - Both overloads of `xx_loadCompleteKeySet()`

### Code Organization Suggestion:
Consider marking internal-only functions more clearly or moving them to a private section/namespace to distinguish between the public API and internal utilities.