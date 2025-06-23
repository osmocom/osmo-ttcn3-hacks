#!/usr/bin/env python3
"""
Test the improved VerifyError exception hierarchy
"""

import sys
sys.path.insert(0, '/app/pysim')

from pySim.esim.x509_cert import (VerifyError, MissingIntermediateCertError, 
                                  CertificateRevokedError, MaxDepthExceededError,
                                  SignatureVerificationError)

print("Testing improved VerifyError exception hierarchy:")
print("-" * 50)

# Test 1: MissingIntermediateCertError
print("Test 1 - MissingIntermediateCertError:")
try:
    raise MissingIntermediateCertError('ABCD1234')
except MissingIntermediateCertError as e:
    print(f"  ✓ Caught MissingIntermediateCertError: {e}")
    print(f"  Auth Key ID: {e.auth_key_id}")
    assert e.auth_key_id == 'ABCD1234'
except Exception as e:
    print(f"  ✗ Wrong exception type: {type(e)}")
print()

# Test 2: CertificateRevokedError
print("Test 2 - CertificateRevokedError:")
try:
    raise CertificateRevokedError('12345')
except CertificateRevokedError as e:
    print(f"  ✓ Caught CertificateRevokedError: {e}")
    print(f"  Serial: {e.cert_serial}")
except Exception as e:
    print(f"  ✗ Wrong exception type: {type(e)}")
print()

# Test 3: MaxDepthExceededError
print("Test 3 - MaxDepthExceededError:")
try:
    raise MaxDepthExceededError(10, 15)
except MaxDepthExceededError as e:
    print(f"  ✓ Caught MaxDepthExceededError: {e}")
    print(f"  Max depth: {e.max_depth}, Actual depth: {e.actual_depth}")
except Exception as e:
    print(f"  ✗ Wrong exception type: {type(e)}")
print()

# Test 4: SignatureVerificationError
print("Test 4 - SignatureVerificationError:")
try:
    raise SignatureVerificationError("Test eUICC", "Test EUM")
except SignatureVerificationError as e:
    print(f"  ✓ Caught SignatureVerificationError: {e}")
    print(f"  Cert subject: {e.cert_subject}, Signer: {e.signer_subject}")
except Exception as e:
    print(f"  ✗ Wrong exception type: {type(e)}")
print()

# Test 5: Inheritance - all should be caught by VerifyError
print("Test 5 - Exception inheritance:")
errors = [
    MissingIntermediateCertError('123'),
    CertificateRevokedError(),
    MaxDepthExceededError(5, 10),
    SignatureVerificationError()
]

for error in errors:
    try:
        raise error
    except VerifyError as e:
        print(f"  ✓ {type(e).__name__} is correctly caught as VerifyError")
    except Exception as e:
        print(f"  ✗ {type(e).__name__} NOT caught as VerifyError!")
print()

# Test 6: Specific exception handling pattern (like in osmo-smdpp.py)
print("Test 6 - Specific exception handling pattern:")
test_cases = [
    ('missing_intermediate', MissingIntermediateCertError('XYZ789')),
    ('revoked', CertificateRevokedError()),
    ('max_depth', MaxDepthExceededError(100, 150)),
    ('signature', SignatureVerificationError())
]

for test_name, exception in test_cases:
    try:
        raise exception
    except MissingIntermediateCertError as e:
        print(f"  ✓ {test_name}: Caught as MissingIntermediateCertError")
    except CertificateRevokedError as e:
        print(f"  ✓ {test_name}: Caught as CertificateRevokedError")
    except MaxDepthExceededError as e:
        print(f"  ✓ {test_name}: Caught as MaxDepthExceededError")
    except SignatureVerificationError as e:
        print(f"  ✓ {test_name}: Caught as SignatureVerificationError")
    except VerifyError as e:
        print(f"  ! {test_name}: Caught as generic VerifyError")
    except Exception as e:
        print(f"  ✗ {test_name}: Unexpected exception type: {type(e)}")

print("\n✅ All tests completed successfully!")