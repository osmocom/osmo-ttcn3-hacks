#!/usr/bin/env python3

"""
Generate a test eUICC certificate with an EID that:
1. Is within the EUM's permitted range (starts with 89049032)
2. Is NOT in the server's default_profiles dictionary
This is for Test Sequence #14: Un-matched EID for Default SM-DP+ Address Use Case
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

def generate_euicc_cert_unmatched_eid():
    """Generate eUICC certificate with an EID not in default profiles."""
    
    # Load EUM certificate and key (NIST version)
    eum_cert = load_certificate_from_file('./sgp26/EUM/CERT_EUM_ECDSA_NIST.der')
    eum_key = load_private_key_from_file('./sgp26/EUM/SK_EUM_ECDSA_NIST.pem')
    
    # Load eUICC private key (use the same key as regular NIST cert)
    euicc_key = load_private_key_from_file('./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem')
    
    # Create subject with EID that:
    # - Starts with 89049032 (within EUM's permitted range)
    # - But is NOT in the server's default_profiles:
    #   - 89049032123451234512345678901235 is EID1 (has default profile)
    #   - We'll use 89049032123451234512345678901299 (different ending)
    subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
        x509.NameAttribute(NameOID.SERIAL_NUMBER, "89049032123451234512345678901299"),  # Valid prefix, not in defaults
        x509.NameAttribute(NameOID.COMMON_NAME, "Test eUICC Unmatched EID"),
    ])
    
    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(eum_cert.subject)
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x0200000000000003)  # Different serial
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
    output_path = './CERT_EUICC_ECDSA_NIST_UNMATCHED_EID.der'
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))
    
    print(f"Generated test eUICC certificate with unmatched EID")
    print(f"EUM permits prefix: 89049032")
    print(f"Test EID:           89049032123451234512345678901299")
    print(f"This EID is within permitted range but NOT in default_profiles")
    print(f"Saved to: {output_path}")
    print(f"Uses same private key as: ./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem")
    
    # Also save as PEM for inspection
    pem_path = output_path.replace('.der', '.pem')
    with open(pem_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.PEM))
    
    return certificate

if __name__ == "__main__":
    generate_euicc_cert_unmatched_eid()