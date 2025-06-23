#!/usr/bin/env python3
"""
Test the x509_err module approach
"""

import sys
sys.path.insert(0, '/app/pysim')

from pySim.esim import x509_err

print("Testing x509_err module approach:")
print("-" * 50)

# Test 1: Import and use exceptions via module
print("Test 1 - Module-based exception usage:")
try:
    raise x509_err.MissingIntermediateCert('ABCD1234')
except x509_err.MissingIntermediateCert as e:
    print(f"  ✓ Caught x509_err.MissingIntermediateCert: {e}")
    print(f"  Auth Key ID: {e.auth_key_id}")
print()

# Test 2: Multiple exceptions with clean syntax
print("Test 2 - Clean exception handling pattern:")
test_exceptions = [
    x509_err.MissingIntermediateCert('XYZ789'),
    x509_err.CertificateRevoked('12345'),
    x509_err.MaxDepthExceeded(10, 15),
    x509_err.SignatureVerification("Test", "Signer"),
    x509_err.InvalidCertificate("missing required field"),
    x509_err.CertificateExpired("Test Cert")
]

for exc in test_exceptions:
    try:
        raise exc
    except x509_err.MissingIntermediateCert as e:
        print(f"  ✓ MissingIntermediateCert: AKI={e.auth_key_id}")
    except x509_err.CertificateRevoked as e:
        print(f"  ✓ CertificateRevoked: serial={e.cert_serial}")
    except x509_err.MaxDepthExceeded as e:
        print(f"  ✓ MaxDepthExceeded: max={e.max_depth}, actual={e.actual_depth}")
    except x509_err.SignatureVerification as e:
        print(f"  ✓ SignatureVerification: {e.cert_subject} by {e.signer_subject}")
    except x509_err.InvalidCertificate as e:
        print(f"  ✓ InvalidCertificate: {e.reason}")
    except x509_err.CertificateExpired as e:
        print(f"  ✓ CertificateExpired: {e.cert_subject}")
    except x509_err.VerifyError as e:
        print(f"  ! Generic VerifyError: {e}")
print()

# Test 3: Inheritance still works
print("Test 3 - Exception inheritance via module:")
try:
    raise x509_err.MaxDepthExceeded(5, 10)
except x509_err.VerifyError as e:
    print(f"  ✓ {type(e).__name__} correctly caught as x509_err.VerifyError")
print()

# Test 4: Show that we don't need individual imports
print("Test 4 - No individual imports needed:")
print(f"  All exceptions accessed via x509_err module:")
print(f"  - x509_err.VerifyError")
print(f"  - x509_err.MissingIntermediateCert")
print(f"  - x509_err.CertificateRevoked")
print(f"  - x509_err.MaxDepthExceeded")
print(f"  - x509_err.SignatureVerification")
print(f"  - x509_err.InvalidCertificate") 
print(f"  - x509_err.CertificateExpired")

print("\n✅ Module-based approach works perfectly!")