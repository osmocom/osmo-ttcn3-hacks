#!/usr/bin/env python3

"""
Generate invalid test certificates for AuthenticateClient error test cases.
This script generates:
1. Invalid EUM certificate (self-signed instead of CI-signed)
2. Expired EUM certificate
3. Invalid eUICC certificate (self-signed instead of EUM-signed)
4. Expired eUICC certificate
5. EUM certificate signed by unknown CI
"""

import os
from datetime import datetime, timedelta
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

def generate_invalid_eum_cert():
    """Generate EUM certificate that is self-signed (invalid) for Error Test #1."""
    print("Generating invalid EUM certificate (self-signed)...")
    
    # Load EUM private key
    eum_key = load_private_key_from_file('./sgp26/EUM/SK_EUM_ECDSA_NIST.pem')
    
    # Create subject
    subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test Invalid EUM"),
        x509.NameAttribute(NameOID.COMMON_NAME, "Invalid EUM Certificate"),
    ])
    
    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(subject)  # Self-signed (invalid)
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x1000000000000001)
    builder = builder.public_key(eum_key.public_key())
    
    # Add extensions
    builder = builder.add_extension(
        x509.SubjectKeyIdentifier.from_public_key(eum_key.public_key()),
        critical=False
    )
    
    builder = builder.add_extension(
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            key_cert_sign=True,  # CA
            crl_sign=False,
            encipher_only=False,
            decipher_only=False
        ),
        critical=True
    )
    
    builder = builder.add_extension(
        x509.CertificatePolicies([
            x509.PolicyInformation(
                x509.ObjectIdentifier("2.23.146.1.2.1.4"),  # EUM policy
                policy_qualifiers=None
            )
        ]),
        critical=True
    )
    
    certificate = builder.sign(eum_key, hashes.SHA256(), default_backend())
    
    output_path = './InvalidTestCases/CERT_EUM_ECDSA_NIST_INVALID.der'
    os.makedirs('./InvalidTestCases', exist_ok=True)
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))
    
    print(f"Saved to: {output_path}")
    return certificate

def generate_expired_eum_cert():
    """Generate EUM certificate that is expired for Error Test #2."""
    print("Generating expired EUM certificate...")
    
    # Load CI certificate and key to sign it properly
    ci_cert = load_certificate_from_file('./sgp26/CertificateIssuer/CERT_CI_ECDSA_NIST.der')
    ci_key = load_private_key_from_file('./sgp26/CertificateIssuer/SK_CI_ECDSA_NIST.pem')
    
    # Load EUM private key
    eum_key = load_private_key_from_file('./sgp26/EUM/SK_EUM_ECDSA_NIST.pem')
    
    # Create subject
    subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
        x509.NameAttribute(NameOID.COMMON_NAME, "Expired EUM Certificate"),
    ])
    
    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(ci_cert.subject)  # Signed by CI
    builder = builder.not_valid_before(datetime(2010, 1, 1, 0, 0, 0))  # Old
    builder = builder.not_valid_after(datetime(2015, 1, 1, 0, 0, 0))    # Expired
    builder = builder.serial_number(0x1000000000000002)
    builder = builder.public_key(eum_key.public_key())
    
    # Add extensions
    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(ci_key.public_key()),
        critical=False
    )
    
    builder = builder.add_extension(
        x509.SubjectKeyIdentifier.from_public_key(eum_key.public_key()),
        critical=False
    )
    
    builder = builder.add_extension(
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            key_cert_sign=True,
            crl_sign=False,
            encipher_only=False,
            decipher_only=False
        ),
        critical=True
    )
    
    builder = builder.add_extension(
        x509.CertificatePolicies([
            x509.PolicyInformation(
                x509.ObjectIdentifier("2.23.146.1.2.1.4"),  # EUM policy
                policy_qualifiers=None
            )
        ]),
        critical=True
    )
    
    certificate = builder.sign(ci_key, hashes.SHA256(), default_backend())
    
    output_path = './InvalidTestCases/CERT_EUM_ECDSA_NIST_EXPIRED.der'
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))
    
    print(f"Saved to: {output_path}")
    return certificate

def generate_invalid_euicc_cert():
    """Generate eUICC certificate that is self-signed (invalid) for Error Test #3."""
    print("Generating invalid eUICC certificate (self-signed)...")
    
    # Load eUICC private key
    euicc_key = load_private_key_from_file('./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem')
    
    # Create subject with valid EID
    subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test eUICC"),
        x509.NameAttribute(NameOID.SERIAL_NUMBER, "89049032123451234512345678901235"),
        x509.NameAttribute(NameOID.COMMON_NAME, "Invalid eUICC Certificate"),
    ])
    
    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(subject)  # Self-signed (invalid)
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x2000000000000001)
    builder = builder.public_key(euicc_key.public_key())
    
    # Add extensions
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
    
    certificate = builder.sign(euicc_key, hashes.SHA256(), default_backend())
    
    output_path = './InvalidTestCases/CERT_EUICC_ECDSA_NIST_INVALID.der'
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))
    
    print(f"Saved to: {output_path}")
    return certificate

def generate_expired_euicc_cert():
    """Generate eUICC certificate that is expired for Error Test #4."""
    print("Generating expired eUICC certificate...")
    
    # Load EUM certificate and key to sign it properly
    eum_cert = load_certificate_from_file('./sgp26/EUM/CERT_EUM_ECDSA_NIST.der')
    eum_key = load_private_key_from_file('./sgp26/EUM/SK_EUM_ECDSA_NIST.pem')
    
    # Load eUICC private key
    euicc_key = load_private_key_from_file('./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem')
    
    # Create subject with valid EID
    subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
        x509.NameAttribute(NameOID.SERIAL_NUMBER, "89049032123451234512345678901235"),
        x509.NameAttribute(NameOID.COMMON_NAME, "Expired eUICC Certificate"),
    ])
    
    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(eum_cert.subject)  # Signed by EUM
    builder = builder.not_valid_before(datetime(2010, 1, 1, 0, 0, 0))  # Old
    builder = builder.not_valid_after(datetime(2015, 1, 1, 0, 0, 0))    # Expired
    builder = builder.serial_number(0x2000000000000002)
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
    
    output_path = './InvalidTestCases/CERT_EUICC_ECDSA_NIST_EXPIRED.der'
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))
    
    print(f"Saved to: {output_path}")
    return certificate

def generate_unknown_ci_eum_cert():
    """Generate EUM certificate signed by unknown CI for Error Test #7."""
    print("Generating EUM certificate signed by unknown CI...")
    
    # Generate a new "unknown" CI key pair
    unknown_ci_key = ec.generate_private_key(ec.SECP256R1(), default_backend())
    
    # Create unknown CI certificate (self-signed)
    ci_subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "XX"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Unknown CI"),
        x509.NameAttribute(NameOID.COMMON_NAME, "Unknown Certificate Issuer"),
    ])
    
    ci_builder = x509.CertificateBuilder()
    ci_builder = ci_builder.subject_name(ci_subject)
    ci_builder = ci_builder.issuer_name(ci_subject)
    ci_builder = ci_builder.not_valid_before(datetime(2020, 1, 1, 0, 0, 0))
    ci_builder = ci_builder.not_valid_after(datetime(2050, 1, 1, 0, 0, 0))
    ci_builder = ci_builder.serial_number(0x9000000000000001)
    ci_builder = ci_builder.public_key(unknown_ci_key.public_key())
    
    unknown_ci_cert = ci_builder.sign(unknown_ci_key, hashes.SHA256(), default_backend())
    
    # Now create EUM certificate signed by this unknown CI
    eum_key = load_private_key_from_file('./sgp26/EUM/SK_EUM_ECDSA_NIST.pem')
    
    subject = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
        x509.NameAttribute(NameOID.COMMON_NAME, "EUM with Unknown CI"),
    ])
    
    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(unknown_ci_cert.subject)  # Signed by unknown CI
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x1000000000000007)
    builder = builder.public_key(eum_key.public_key())
    
    # Add extensions
    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(unknown_ci_key.public_key()),
        critical=False
    )
    
    builder = builder.add_extension(
        x509.SubjectKeyIdentifier.from_public_key(eum_key.public_key()),
        critical=False
    )
    
    builder = builder.add_extension(
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            key_cert_sign=True,
            crl_sign=False,
            encipher_only=False,
            decipher_only=False
        ),
        critical=True
    )
    
    builder = builder.add_extension(
        x509.CertificatePolicies([
            x509.PolicyInformation(
                x509.ObjectIdentifier("2.23.146.1.2.1.4"),  # EUM policy
                policy_qualifiers=None
            )
        ]),
        critical=True
    )
    
    certificate = builder.sign(unknown_ci_key, hashes.SHA256(), default_backend())
    
    # Save both certificates
    output_path = './InvalidTestCases/CERT_EUM_ECDSA_NIST_UNKNOWN_CI.der'
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))
    
    ci_output_path = './InvalidTestCases/CERT_UNKNOWN_CI_ECDSA_NIST.der'
    with open(ci_output_path, "wb") as f:
        f.write(unknown_ci_cert.public_bytes(serialization.Encoding.DER))
    
    print(f"Saved EUM cert to: {output_path}")
    print(f"Saved unknown CI cert to: {ci_output_path}")
    return certificate

if __name__ == "__main__":
    # Generate all invalid certificates
    generate_invalid_eum_cert()
    generate_expired_eum_cert()
    generate_invalid_euicc_cert()
    generate_expired_euicc_cert()
    generate_unknown_ci_eum_cert()
    
    print("\nAll invalid test certificates generated successfully!")