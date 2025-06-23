#!/usr/bin/env python3
"""
Test the improved VerifyError exception handling
"""

import sys
sys.path.insert(0, '/app/pysim')

from pySim.esim.x509_cert import VerifyError

# Test the new VerifyError class
print("Testing improved VerifyError class:")
print("-" * 50)

# Test 1: Basic error with all fields
e1 = VerifyError(
    "Missing intermediate certificate", 
    error_type=VerifyError.MISSING_INTERMEDIATE,
    cert_level=VerifyError.CERT_EUM,
    details={'auth_key_id': 'ABCD1234'}
)
print(f"Test 1 - Full error:")
print(f"  Message: {str(e1)}")
print(f"  Error Type: {e1.error_type}")
print(f"  Cert Level: {e1.cert_level}")
print(f"  Details: {e1.details}")
print()

# Test 2: Error without optional fields
e2 = VerifyError("Generic error")
print(f"Test 2 - Minimal error:")
print(f"  Message: {str(e2)}")
print(f"  Error Type: {e2.error_type}")
print(f"  Cert Level: {e2.cert_level}")
print(f"  Details: {e2.details}")
print()

# Test 3: Simulate the certificate chain verification scenario
print("Test 3 - Certificate chain verification scenario:")
try:
    # Simulate missing intermediate cert error
    raise VerifyError(
        'Could not find intermediate certificate for AuthKeyId 1234567890',
        error_type=VerifyError.MISSING_INTERMEDIATE,
        cert_level=VerifyError.CERT_UNKNOWN,
        details={'auth_key_id': '1234567890'}
    )
except VerifyError as e:
    print(f"  Caught VerifyError: {e}")
    if e.error_type == VerifyError.MISSING_INTERMEDIATE:
        print(f"  ✓ Correctly identified as missing intermediate certificate")
        print(f"  Missing certificate AKI: {e.details.get('auth_key_id')}")
    else:
        print(f"  ✗ Error type mismatch: {e.error_type}")
print()

# Test 4: Check all error type constants
print("Test 4 - Error type constants:")
print(f"  MISSING_INTERMEDIATE: {VerifyError.MISSING_INTERMEDIATE}")
print(f"  MAX_DEPTH_EXCEEDED: {VerifyError.MAX_DEPTH_EXCEEDED}")
print(f"  CRL_REVOKED: {VerifyError.CRL_REVOKED}")
print(f"  SIGNATURE_INVALID: {VerifyError.SIGNATURE_INVALID}")
print()

print("Test 5 - Certificate level constants:")
print(f"  CERT_CI: {VerifyError.CERT_CI}")
print(f"  CERT_EUM: {VerifyError.CERT_EUM}")
print(f"  CERT_EUICC: {VerifyError.CERT_EUICC}")
print(f"  CERT_UNKNOWN: {VerifyError.CERT_UNKNOWN}")

print("\n✅ All tests completed successfully!")