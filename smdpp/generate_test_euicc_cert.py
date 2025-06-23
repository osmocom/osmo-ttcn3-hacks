#!/usr/bin/env python3

"""
Generate a test eUICC certificate with a different EID for the Invalid_euiccInfo1 test case.
This certificate will have EID 89049032123451234512345678901299 instead of ...1235
"""

import os
import binascii
from datetime import datetime
from cryptography import x509
from cryptography.x509.oid import NameOID, ExtensionOID
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.backends import default_backend

def load_private_key_from_file(key_path):
    """Load EC private key from PEM file."""
    with open(key_path, 'rb') as f:
        return serialization.load_pem_private_key(f.read(), password=None, backend=default_backend())

def load_certificate_from_file(cert_path):
    """Load certificate from DER file."""
    with open(cert_path, 'rb') as f:
        return x509.load_der_x509_certificate(f.read(), default_backend())

def generate_euicc_cert_with_different_eid():
    """Generate eUICC certificate with a different EID for testing."""
    
    # Load EUM certificate and key (NIST version)
    eum_cert = load_certificate_from_file('./sgp26/EUM/CERT_EUM_ECDSA_NIST.der')
    eum_key = load_private_key_from_file('./sgp26/EUM/SK_EUM_ECDSA_NIST.pem')
    
    # Load eUICC private key
    euicc_key = load_private_key_from_file('./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem')
    
    # Create subject with DIFFERENT EID PREFIX - outside EUM's permitted range
    # EUM permits only EIDs starting with 89049032
    # We'll use a different prefix: 89049033 (just increment the last digit)
    subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
        x509.NameAttribute(NameOID.SERIAL_NUMBER, "89049033123451234512345678901234"),  # Different prefix!
        x509.NameAttribute(NameOID.COMMON_NAME, "Test eUICC Invalid EID"),
    ])
    
    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(eum_cert.subject)
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))  # Shorter validity
    builder = builder.serial_number(0x0200000000000002)  # Different serial
    builder = builder.public_key(euicc_key.public_key())
    
    # Add extensions
    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(eum_key.public_key()),
        critical=False
    )
    
    builder = builder.add_extension(
        x509.SubjectKeyIdentifier.from_public_key(euicc_key.public_key()),
        critical=False
    )
    
    builder = builder.add_extension(
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            key_cert_sign=False,
            crl_sign=False,
            encipher_only=False,
            decipher_only=False
        ),
        critical=True
    )
    
    builder = builder.add_extension(
        x509.CertificatePolicies([
            x509.PolicyInformation(
                x509.ObjectIdentifier("2.23.146.1.2.1.1"),  # eUICC policy
                policy_qualifiers=None
            )
        ]),
        critical=True
    )
    
    certificate = builder.sign(eum_key, hashes.SHA256(), default_backend())
    
    # Save the certificate
    output_path = './sgp26/eUICC/CERT_EUICC_ECDSA_NIST_INVALID_EID.der'
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))
    
    print(f"Generated test eUICC certificate with EID outside EUM's permitted range")
    print(f"EUM permits prefix: 89049032")
    print(f"Test EID uses:      89049033 (outside permitted range)")
    print(f"Full test EID:      89049033123451234512345678901234")
    print(f"Saved to: {output_path}")
    
    # Also save as PEM for inspection
    pem_path = output_path.replace('.der', '.pem')
    with open(pem_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.PEM))
    
    return certificate

if __name__ == "__main__":
    generate_euicc_cert_with_different_eid()