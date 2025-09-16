#!/usr/bin/env python3

"""
Generate all test certificates for SM-DP+ AuthenticateClient error test cases.

This script generates 8 test certificates for various error scenarios:
1. Invalid EUM certificate (self-signed instead of CI-signed) - Error Test #1
2. Expired EUM certificate - Error Test #2
3. Invalid eUICC certificate (self-signed instead of EUM-signed) - Error Test #3
4. Expired eUICC certificate - Error Test #4
5. EUM certificate signed by unknown CI - Error Test #7
6. Unknown CI certificate (used by #5)
7. eUICC with invalid EID (outside EUM's permitted range) - Error Test #6
8. eUICC with unmatched EID (not in default profiles) - Error Test #14

All certificates are saved to ./InvalidTestCases/ directory.
"""

import os
from datetime import datetime
from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.backends import default_backend


def load_private_key_from_file(key_path):
    """Load EC private key from PEM file."""
    with open(key_path, "rb") as f:
        return serialization.load_pem_private_key(f.read(), password=None, backend=default_backend())


def load_certificate_from_file(cert_path):
    """Load certificate from DER file."""
    with open(cert_path, "rb") as f:
        return x509.load_der_x509_certificate(f.read(), default_backend())


def generate_invalid_eum_cert():
    """Generate EUM certificate that is self-signed (invalid) for Error Test #1."""
    print("Generating invalid EUM certificate (self-signed)...")

    eum_key = load_private_key_from_file("./sgp26/EUM/SK_EUM_ECDSA_NIST.pem")

    subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test Invalid EUM"),
            x509.NameAttribute(NameOID.COMMON_NAME, "Invalid EUM Certificate"),
        ]
    )

    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(subject)  # Self-signed (invalid)
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x1000000000000001)
    builder = builder.public_key(eum_key.public_key())

    builder = builder.add_extension(x509.SubjectKeyIdentifier.from_public_key(eum_key.public_key()), critical=False)

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
            decipher_only=False,
        ),
        critical=True,
    )

    builder = builder.add_extension(
        x509.CertificatePolicies(
            [
                x509.PolicyInformation(
                    x509.ObjectIdentifier("2.23.146.1.2.1.4"),  # EUM policy
                    policy_qualifiers=None,
                )
            ]
        ),
        critical=True,
    )

    certificate = builder.sign(eum_key, hashes.SHA256(), default_backend())

    output_path = "./InvalidTestCases/CERT_EUM_ECDSA_NIST_INVALID.der"
    os.makedirs("./InvalidTestCases", exist_ok=True)
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))

    print(f"  Saved to: {output_path}")
    return certificate


def generate_expired_eum_cert():
    """Generate EUM certificate that is expired for Error Test #2."""
    print("Generating expired EUM certificate...")

    ci_cert = load_certificate_from_file("./sgp26/CertificateIssuer/CERT_CI_ECDSA_NIST.der")
    ci_key = load_private_key_from_file("./sgp26/CertificateIssuer/SK_CI_ECDSA_NIST.pem")

    eum_key = load_private_key_from_file("./sgp26/EUM/SK_EUM_ECDSA_NIST.pem")

    subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
            x509.NameAttribute(NameOID.COMMON_NAME, "Expired EUM Certificate"),
        ]
    )

    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(ci_cert.subject)  # Signed by CI
    builder = builder.not_valid_before(datetime(2010, 1, 1, 0, 0, 0))  # Old
    builder = builder.not_valid_after(datetime(2015, 1, 1, 0, 0, 0))  # Expired
    builder = builder.serial_number(0x1000000000000002)
    builder = builder.public_key(eum_key.public_key())

    # Add extensions
    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(ci_key.public_key()), critical=False
    )

    builder = builder.add_extension(x509.SubjectKeyIdentifier.from_public_key(eum_key.public_key()), critical=False)

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
            decipher_only=False,
        ),
        critical=True,
    )

    builder = builder.add_extension(
        x509.CertificatePolicies(
            [
                x509.PolicyInformation(
                    x509.ObjectIdentifier("2.23.146.1.2.1.4"),  # EUM policy
                    policy_qualifiers=None,
                )
            ]
        ),
        critical=True,
    )

    certificate = builder.sign(ci_key, hashes.SHA256(), default_backend())

    output_path = "./InvalidTestCases/CERT_EUM_ECDSA_NIST_EXPIRED.der"
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))

    print(f"  Saved to: {output_path}")
    return certificate


def generate_invalid_euicc_cert():
    """Generate eUICC certificate that is self-signed (invalid) for Error Test #3."""
    print("Generating invalid eUICC certificate (self-signed)...")

    euicc_key = load_private_key_from_file("./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem")

    subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test eUICC"),
            x509.NameAttribute(NameOID.SERIAL_NUMBER, "89049032123451234512345678901235"),
            x509.NameAttribute(NameOID.COMMON_NAME, "Invalid eUICC Certificate"),
        ]
    )

    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(subject)  # Self-signed (invalid)
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x2000000000000001)
    builder = builder.public_key(euicc_key.public_key())

    builder = builder.add_extension(x509.SubjectKeyIdentifier.from_public_key(euicc_key.public_key()), critical=False)

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
            decipher_only=False,
        ),
        critical=True,
    )

    builder = builder.add_extension(
        x509.CertificatePolicies(
            [
                x509.PolicyInformation(
                    x509.ObjectIdentifier("2.23.146.1.2.1.1"),  # eUICC policy
                    policy_qualifiers=None,
                )
            ]
        ),
        critical=True,
    )

    certificate = builder.sign(euicc_key, hashes.SHA256(), default_backend())

    output_path = "./InvalidTestCases/CERT_EUICC_ECDSA_NIST_INVALID.der"
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))

    print(f"  Saved to: {output_path}")
    return certificate


def generate_expired_euicc_cert():
    """Generate eUICC certificate that is expired for Error Test #4."""
    print("Generating expired eUICC certificate...")

    eum_cert = load_certificate_from_file("./sgp26/EUM/CERT_EUM_ECDSA_NIST.der")
    eum_key = load_private_key_from_file("./sgp26/EUM/SK_EUM_ECDSA_NIST.pem")

    euicc_key = load_private_key_from_file("./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem")

    subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
            x509.NameAttribute(NameOID.SERIAL_NUMBER, "89049032123451234512345678901235"),
            x509.NameAttribute(NameOID.COMMON_NAME, "Expired eUICC Certificate"),
        ]
    )

    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(eum_cert.subject)  # Signed by EUM
    builder = builder.not_valid_before(datetime(2010, 1, 1, 0, 0, 0))  # Old
    builder = builder.not_valid_after(datetime(2015, 1, 1, 0, 0, 0))  # Expired
    builder = builder.serial_number(0x2000000000000002)
    builder = builder.public_key(euicc_key.public_key())

    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(eum_key.public_key()), critical=False
    )

    builder = builder.add_extension(x509.SubjectKeyIdentifier.from_public_key(euicc_key.public_key()), critical=False)

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
            decipher_only=False,
        ),
        critical=True,
    )

    builder = builder.add_extension(
        x509.CertificatePolicies(
            [
                x509.PolicyInformation(
                    x509.ObjectIdentifier("2.23.146.1.2.1.1"),  # eUICC policy
                    policy_qualifiers=None,
                )
            ]
        ),
        critical=True,
    )

    certificate = builder.sign(eum_key, hashes.SHA256(), default_backend())

    output_path = "./InvalidTestCases/CERT_EUICC_ECDSA_NIST_EXPIRED.der"
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))

    print(f"  Saved to: {output_path}")
    return certificate


def generate_unknown_ci_eum_cert():
    """Generate EUM certificate signed by unknown CI for Error Test #7."""
    print("Generating EUM certificate signed by unknown CI...")

    unknown_ci_key = ec.generate_private_key(ec.SECP256R1(), default_backend())

    ci_subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "XX"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Unknown CI"),
            x509.NameAttribute(NameOID.COMMON_NAME, "Unknown Certificate Issuer"),
        ]
    )

    ci_builder = x509.CertificateBuilder()
    ci_builder = ci_builder.subject_name(ci_subject)
    ci_builder = ci_builder.issuer_name(ci_subject)
    ci_builder = ci_builder.not_valid_before(datetime(2020, 1, 1, 0, 0, 0))
    ci_builder = ci_builder.not_valid_after(datetime(2050, 1, 1, 0, 0, 0))
    ci_builder = ci_builder.serial_number(0x9000000000000001)
    ci_builder = ci_builder.public_key(unknown_ci_key.public_key())

    unknown_ci_cert = ci_builder.sign(unknown_ci_key, hashes.SHA256(), default_backend())

    eum_key = load_private_key_from_file("./sgp26/EUM/SK_EUM_ECDSA_NIST.pem")

    subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
            x509.NameAttribute(NameOID.COMMON_NAME, "EUM with Unknown CI"),
        ]
    )

    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(unknown_ci_cert.subject)  # Signed by unknown CI
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x1000000000000007)
    builder = builder.public_key(eum_key.public_key())

    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(unknown_ci_key.public_key()), critical=False
    )

    builder = builder.add_extension(x509.SubjectKeyIdentifier.from_public_key(eum_key.public_key()), critical=False)

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
            decipher_only=False,
        ),
        critical=True,
    )

    builder = builder.add_extension(
        x509.CertificatePolicies(
            [
                x509.PolicyInformation(
                    x509.ObjectIdentifier("2.23.146.1.2.1.4"),  # EUM policy
                    policy_qualifiers=None,
                )
            ]
        ),
        critical=True,
    )

    certificate = builder.sign(unknown_ci_key, hashes.SHA256(), default_backend())

    output_path = "./InvalidTestCases/CERT_EUM_ECDSA_NIST_UNKNOWN_CI.der"
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))

    ci_output_path = "./InvalidTestCases/CERT_UNKNOWN_CI_ECDSA_NIST.der"
    with open(ci_output_path, "wb") as f:
        f.write(unknown_ci_cert.public_bytes(serialization.Encoding.DER))

    print(f"  Saved EUM cert to: {output_path}")
    print(f"  Saved unknown CI cert to: {ci_output_path}")
    return certificate


def generate_euicc_cert_with_invalid_eid():
    """Generate eUICC certificate with EID outside EUM's permitted range for Error Test #6."""
    print("Generating eUICC certificate with invalid EID (outside EUM's range)...")

    eum_cert = load_certificate_from_file("./sgp26/EUM/CERT_EUM_ECDSA_NIST.der")
    eum_key = load_private_key_from_file("./sgp26/EUM/SK_EUM_ECDSA_NIST.pem")

    euicc_key = load_private_key_from_file("./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem")

    # Create subject with DIFFERENT EID PREFIX - outside EUM's permitted range
    # EUM permits only EIDs starting with 89049032
    # We'll use a different prefix: 89049033 (just increment the last digit)
    subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
            x509.NameAttribute(NameOID.SERIAL_NUMBER, "89049033123451234512345678901234"),  # Different prefix!
            x509.NameAttribute(NameOID.COMMON_NAME, "Test eUICC Invalid EID"),
        ]
    )

    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(eum_cert.subject)
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x0200000000000002)
    builder = builder.public_key(euicc_key.public_key())

    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(eum_key.public_key()), critical=False
    )

    builder = builder.add_extension(x509.SubjectKeyIdentifier.from_public_key(euicc_key.public_key()), critical=False)

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
            decipher_only=False,
        ),
        critical=True,
    )

    builder = builder.add_extension(
        x509.CertificatePolicies(
            [
                x509.PolicyInformation(
                    x509.ObjectIdentifier("2.23.146.1.2.1.1"),  # eUICC policy
                    policy_qualifiers=None,
                )
            ]
        ),
        critical=True,
    )

    certificate = builder.sign(eum_key, hashes.SHA256(), default_backend())

    output_path = "./InvalidTestCases/CERT_EUICC_ECDSA_NIST_INVALID_EID.der"
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))

    print("  EUM permits prefix: 89049032")
    print("  Test EID uses:      89049033 (outside permitted range)")
    print("  Full test EID:      89049033123451234512345678901234")
    print(f"  Saved to: {output_path}")
    print("  Note: Uses same private key as SK_EUICC_ECDSA_NIST.pem")

    # Also save as PEM for inspection
    pem_path = output_path.replace(".der", ".pem")
    with open(pem_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.PEM))

    return certificate


def generate_euicc_cert_unmatched_eid():
    """Generate eUICC certificate with EID not in default profiles for Error Test #14."""
    print("Generating eUICC certificate with unmatched EID (not in default profiles)...")

    eum_cert = load_certificate_from_file("./sgp26/EUM/CERT_EUM_ECDSA_NIST.der")
    eum_key = load_private_key_from_file("./sgp26/EUM/SK_EUM_ECDSA_NIST.pem")

    euicc_key = load_private_key_from_file("./sgp26/eUICC/SK_EUICC_ECDSA_NIST.pem")

    # Create subject with EID that:
    # - Starts with 89049032 (within EUM's permitted range)
    # - But is NOT in the server's default_profiles:
    #   - 89049032123451234512345678901235 is EID1 (has default profile)
    #   - We'll use 89049032123451234512345678901299 (different ending)
    subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "ES"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "RSP Test EUM"),
            x509.NameAttribute(
                NameOID.SERIAL_NUMBER, "89049032123451234512345678901299"
            ),  # Valid prefix, not in defaults
            x509.NameAttribute(NameOID.COMMON_NAME, "Test eUICC Unmatched EID"),
        ]
    )

    builder = x509.CertificateBuilder()
    builder = builder.subject_name(subject)
    builder = builder.issuer_name(eum_cert.subject)
    builder = builder.not_valid_before(datetime(2020, 4, 1, 9, 48, 58))
    builder = builder.not_valid_after(datetime(2055, 1, 24, 9, 48, 58))
    builder = builder.serial_number(0x0200000000000003)
    builder = builder.public_key(euicc_key.public_key())

    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_public_key(eum_key.public_key()), critical=False
    )

    builder = builder.add_extension(x509.SubjectKeyIdentifier.from_public_key(euicc_key.public_key()), critical=False)

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
            decipher_only=False,
        ),
        critical=True,
    )

    builder = builder.add_extension(
        x509.CertificatePolicies(
            [
                x509.PolicyInformation(
                    x509.ObjectIdentifier("2.23.146.1.2.1.1"),  # eUICC policy
                    policy_qualifiers=None,
                )
            ]
        ),
        critical=True,
    )

    certificate = builder.sign(eum_key, hashes.SHA256(), default_backend())

    output_path = "./InvalidTestCases/CERT_EUICC_ECDSA_NIST_UNMATCHED_EID.der"
    with open(output_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.DER))

    print("  EUM permits prefix: 89049032")
    print("  Test EID:           89049032123451234512345678901299")
    print("  This EID is within permitted range but NOT in default_profiles")
    print(f"  Saved to: {output_path}")
    print("  Note: Uses same private key as SK_EUICC_ECDSA_NIST.pem")

    # Also save as PEM for inspection
    pem_path = output_path.replace(".der", ".pem")
    with open(pem_path, "wb") as f:
        f.write(certificate.public_bytes(serialization.Encoding.PEM))

    return certificate


if __name__ == "__main__":
    print("Generating all test certificates for SM-DP+ error test cases...")
    print("=" * 60)

    certificates_generated = []

    # Error Tests #1-4: Invalid/Expired certificates
    generate_invalid_eum_cert()
    certificates_generated.append("CERT_EUM_ECDSA_NIST_INVALID.der - Self-signed EUM (Error Test #1)")

    generate_expired_eum_cert()
    certificates_generated.append("CERT_EUM_ECDSA_NIST_EXPIRED.der - Expired EUM (Error Test #2)")

    generate_invalid_euicc_cert()
    certificates_generated.append("CERT_EUICC_ECDSA_NIST_INVALID.der - Self-signed eUICC (Error Test #3)")

    generate_expired_euicc_cert()
    certificates_generated.append("CERT_EUICC_ECDSA_NIST_EXPIRED.der - Expired eUICC (Error Test #4)")

    # Error Test #7: Unknown CI
    generate_unknown_ci_eum_cert()
    certificates_generated.append("CERT_EUM_ECDSA_NIST_UNKNOWN_CI.der - EUM signed by unknown CI (Error Test #7)")
    certificates_generated.append("CERT_UNKNOWN_CI_ECDSA_NIST.der - Unknown CI certificate")

    # Error Test #6: Invalid EID
    generate_euicc_cert_with_invalid_eid()
    certificates_generated.append(
        "CERT_EUICC_ECDSA_NIST_INVALID_EID.der - eUICC with EID outside EUM range (Error Test #6)"
    )

    # Error Test #14: Unmatched EID
    generate_euicc_cert_unmatched_eid()
    certificates_generated.append("CERT_EUICC_ECDSA_NIST_UNMATCHED_EID.der - eUICC with unmatched EID (Error Test #14)")

    print("\n" + "=" * 60)
    print("Summary: All test certificates generated successfully!")
    print("=" * 60)
    print("\nGenerated certificates in ./InvalidTestCases/:")
    for cert in certificates_generated:
        print(f"  • {cert}")
    print("\nTotal: {} certificates generated".format(len(certificates_generated)))
