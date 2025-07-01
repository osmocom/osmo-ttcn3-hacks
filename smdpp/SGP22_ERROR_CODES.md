# SGP.22 Error Code Meanings

Based on SGP.22 v2.5 specification, here are the meanings of the requested error codes:

## Subject Codes

### 8.1 - eUICC
- **8.1.1** - EID (eUICC Identifier)
- **8.1.2** - EUM Certificate (eUICC Manufacturer Certificate)
- **8.1.3** - eUICC Certificate

### 8.2 - Profile
- **8.2** - Profile (general)
- **8.2.1** - Profile ICCID
- **8.2.5** - Profile Type
- **8.2.6** - Matching ID

### 8.8 - SM-DP+
- **8.8.5** - Download order

### 8.10 - RSP Operation
- **8.10.1** - TransactionId

### 8.11 - GSMA CI
- **8.11.1** - Public Key (PK)

## Reason Codes

### 1. Access Error
- **1.2** - Not Allowed (Authorisation)

### 3. Conditions of Use Not Satisfied
- **3.7** - Unavailable
- **3.8** - Refused
- **3.9** - Unknown

### 4. Processing Error
- **4.3** - Stopped on Warning
- **4.10** - Time to Live Expired

### 6. Security Error
- **6.1** - Verification Failed
- **6.3** - Expired
- **6.4** - Maximum number of retries exceeded

## Complete Error Code Meanings

- **Subject Code 8.1.2 Reason Code 6.1**: EUM Certificate - Verification Failed
- **Subject Code 8.1.2 Reason Code 6.3**: EUM Certificate - Expired
- **Subject Code 8.1.3 Reason Code 6.1**: eUICC Certificate - Verification Failed
- **Subject Code 8.1.3 Reason Code 6.3**: eUICC Certificate - Expired
- **Subject Code 8.1 Reason Code 6.1**: eUICC - Verification Failed
- **Subject Code 8.11.1 Reason Code 3.9**: GSMA CI Public Key - Unknown
- **Subject Code 8.2 Reason Code 1.2**: Profile - Not Allowed (Authorisation)
- **Subject Code 8.10.1 Reason Code 3.9**: TransactionId - Unknown
- **Subject Code 8.2.6 Reason Code 3.8**: Matching ID - Refused
- **Subject Code 8.1.1 Reason Code 3.8**: EID - Refused
- **Subject Code 8.2.5 Reason Code 4.3**: Profile Type - Stopped on Warning
- **Subject Code 8.8.5 Reason Code 4.10**: Download order - Time to Live Expired
- **Subject Code 8.8.5 Reason Code 6.4**: Download order - Maximum number of retries exceeded