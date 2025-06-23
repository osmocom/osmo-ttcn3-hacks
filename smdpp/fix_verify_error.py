#!/usr/bin/env python3
"""
Fix the VerifyError class to include structured error information
instead of relying on string parsing.
"""

import re

def fix_verify_error():
    # Read the x509_cert.py file
    with open('/app/pysim/pySim/esim/x509_cert.py', 'r') as f:
        content = f.read()
    
    # Replace the VerifyError class definition
    old_class = '''class VerifyError(Exception):
    """An error during certificate verification,"""'''
    
    new_class = '''class VerifyError(Exception):
    """An error during certificate verification with structured error information."""
    
    # Error types
    MISSING_INTERMEDIATE = 'missing_intermediate'
    MAX_DEPTH_EXCEEDED = 'max_depth_exceeded'
    CRL_REVOKED = 'crl_revoked'
    SIGNATURE_INVALID = 'signature_invalid'
    
    # Certificate levels
    CERT_CI = 'ci'
    CERT_EUM = 'eum'
    CERT_EUICC = 'euicc'
    CERT_UNKNOWN = 'unknown'
    
    def __init__(self, message, error_type=None, cert_level=None, details=None):
        super().__init__(message)
        self.error_type = error_type
        self.cert_level = cert_level
        self.details = details or {}'''
    
    content = content.replace(old_class, new_class)
    
    # Fix the raise statements in verify_cert_chain
    # 1. Fix CRL revocation error
    content = re.sub(
        r"raise VerifyError\('Certificate is present in CRL, verification failed'\)",
        "raise VerifyError('Certificate is present in CRL, verification failed', "
        "error_type=VerifyError.CRL_REVOKED, cert_level=VerifyError.CERT_UNKNOWN)",
        content
    )
    
    # 2. Fix missing intermediate certificate error
    content = re.sub(
        r"raise VerifyError\('Could not find intermediate certificate for AuthKeyId %s' % b2h\(aki\)\)",
        "raise VerifyError('Could not find intermediate certificate for AuthKeyId %s' % b2h(aki), "
        "error_type=VerifyError.MISSING_INTERMEDIATE, cert_level=VerifyError.CERT_UNKNOWN, "
        "details={'auth_key_id': b2h(aki)})",
        content
    )
    
    # 3. Fix max depth exceeded error
    content = re.sub(
        r"raise VerifyError\('Maximum depth %u exceeded while verifying certificate chain' % max_depth\)",
        "raise VerifyError('Maximum depth %u exceeded while verifying certificate chain' % max_depth, "
        "error_type=VerifyError.MAX_DEPTH_EXCEEDED, cert_level=VerifyError.CERT_UNKNOWN, "
        "details={'max_depth': max_depth, 'actual_depth': depth})",
        content
    )
    
    # Write back the file
    with open('/app/pysim/pySim/esim/x509_cert.py', 'w') as f:
        f.write(content)
    
    print("Fixed VerifyError in x509_cert.py")
    
    # Now fix the osmo-smdpp.py file to use the structured error information
    with open('/app/pysim/osmo-smdpp.py', 'r') as f:
        content = f.read()
    
    # Find and replace the certificate verification error handling
    old_pattern = r'''        # Verify the validity of the eUICC certificate chain
        try:
            cs = CertificateSet\(self\.ci_get_cert_for_pkid\(ss\.ci_cert_id\)\)
            cs\.add_intermediate_cert\(eum_cert\)
            # TODO v3: otherCertsInChain
            cs\.verify_cert_chain\(euicc_cert\)
        except VerifyError as e:
            # Determine if it's an EUM or eUICC certificate issue
            error_str = str\(e\)\.lower\(\)
            if 'eum' in error_str or 'intermediate' in error_str:
                raise ApiError\('8\.1\.2', '6\.1', 'Invalid EUM Certificate in chain'\)
            else:
                raise ApiError\('8\.1\.3', '6\.1', 'Invalid eUICC Certificate in chain'\)
        except Exception as e:
            logger\.error\(f"Certificate chain verification error: \{e\}"\)
            # Check if it's a missing CI certificate issue
            if 'ci_cert_id' in str\(e\) or 'Unknown' in str\(e\):
                raise ApiError\('8\.11\.1', '3\.9', 'Unknown CI Public Key'\)
            raise ApiError\('8\.1\.3', '6\.1', 'Certificate chain verification failed'\)'''
    
    new_pattern = '''        # Verify the validity of the eUICC certificate chain
        try:
            cs = CertificateSet(self.ci_get_cert_for_pkid(ss.ci_cert_id))
            cs.add_intermediate_cert(eum_cert)
            # TODO v3: otherCertsInChain
            cs.verify_cert_chain(euicc_cert)
        except VerifyError as e:
            # Use structured error information
            if e.error_type == VerifyError.MISSING_INTERMEDIATE:
                # Check if the missing certificate is the EUM cert
                if e.details.get('auth_key_id') == b2h(cert_get_auth_key_id(eum_cert)):
                    raise ApiError('8.1.2', '6.1', 'Invalid EUM Certificate in chain')
                else:
                    raise ApiError('8.1.3', '6.1', 'Invalid eUICC Certificate in chain')
            elif e.error_type == VerifyError.CRL_REVOKED:
                raise ApiError('8.1.3', '6.3', 'Certificate revoked')
            elif e.error_type == VerifyError.MAX_DEPTH_EXCEEDED:
                raise ApiError('8.1.3', '6.1', f'Certificate chain too deep: {e}')
            else:
                # Generic certificate chain error
                raise ApiError('8.1.3', '6.1', f'Certificate chain verification failed: {e}')
        except ValueError as e:
            # This could be raised by add_intermediate_cert for invalid EUM cert
            if 'intermediate certificate' in str(e):
                raise ApiError('8.1.2', '6.1', f'Invalid EUM Certificate: {e}')
            raise ApiError('8.1.3', '6.1', f'Certificate validation error: {e}')
        except Exception as e:
            logger.error(f"Certificate chain verification error: {e}")
            # Check if it's a missing CI certificate issue
            if 'ci_cert_id' in str(e) or 'Unknown' in str(e):
                raise ApiError('8.11.1', '3.9', 'Unknown CI Public Key')
            raise ApiError('8.1.3', '6.1', 'Certificate chain verification failed')'''
    
    # Apply the fix
    content = re.sub(old_pattern, new_pattern, content, flags=re.DOTALL)
    
    # Also need to add the import for b2h
    if 'from pySim.utils import b2h' not in content:
        # Find where to add the import (after other pySim imports)
        import_pattern = r'(from pySim\.esim\.x509_cert import[^\n]+\n)'
        replacement = r'\1from pySim.utils import b2h\n'
        content = re.sub(import_pattern, replacement, content)
    
    # Write back the file
    with open('/app/pysim/osmo-smdpp.py', 'w') as f:
        f.write(content)
    
    print("Fixed error handling in osmo-smdpp.py")

if __name__ == "__main__":
    fix_verify_error()