#!/usr/bin/env python3
"""
Refactor VerifyError to use proper exception hierarchy
"""

def refactor_verify_error():
    # Read the x509_cert.py file
    with open('/app/pysim/pySim/esim/x509_cert.py', 'r') as f:
        content = f.read()
    
    # Replace the VerifyError class definition with a proper hierarchy
    old_class = '''class VerifyError(Exception):
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
    
    new_class = '''class VerifyError(Exception):
    """Base class for certificate verification errors."""
    pass


class MissingIntermediateCertError(VerifyError):
    """Raised when an intermediate certificate in the chain cannot be found."""
    def __init__(self, auth_key_id: str):
        self.auth_key_id = auth_key_id
        super().__init__(f'Could not find intermediate certificate for AuthKeyId {auth_key_id}')


class CertificateRevokedError(VerifyError):
    """Raised when a certificate is found in the CRL."""
    def __init__(self, cert_serial: str = None):
        self.cert_serial = cert_serial
        msg = 'Certificate is present in CRL, verification failed'
        if cert_serial:
            msg += f' (serial: {cert_serial})'
        super().__init__(msg)


class MaxDepthExceededError(VerifyError):
    """Raised when certificate chain depth exceeds the maximum allowed."""
    def __init__(self, max_depth: int, actual_depth: int):
        self.max_depth = max_depth
        self.actual_depth = actual_depth
        super().__init__(f'Maximum depth {max_depth} exceeded while verifying certificate chain (actual: {actual_depth})')


class SignatureVerificationError(VerifyError):
    """Raised when certificate signature verification fails."""
    def __init__(self, cert_subject: str = None, signer_subject: str = None):
        self.cert_subject = cert_subject
        self.signer_subject = signer_subject
        msg = 'Certificate signature verification failed'
        if cert_subject and signer_subject:
            msg += f': {cert_subject} not signed by {signer_subject}'
        super().__init__(msg)'''
    
    content = content.replace(old_class, new_class)
    
    # Update the raise statements
    # 1. Fix CRL revocation error
    content = content.replace(
        "raise VerifyError('Certificate is present in CRL, verification failed', error_type=VerifyError.CRL_REVOKED, cert_level=VerifyError.CERT_UNKNOWN)",
        "raise CertificateRevokedError()"
    )
    
    # 2. Fix missing intermediate certificate error  
    content = content.replace(
        "raise VerifyError('Could not find intermediate certificate for AuthKeyId %s' % b2h(aki), error_type=VerifyError.MISSING_INTERMEDIATE, cert_level=VerifyError.CERT_UNKNOWN, details={'auth_key_id': b2h(aki)})",
        "raise MissingIntermediateCertError(b2h(aki))"
    )
    
    # 3. Fix max depth exceeded error
    content = content.replace(
        "raise VerifyError('Maximum depth %u exceeded while verifying certificate chain' % max_depth, error_type=VerifyError.MAX_DEPTH_EXCEEDED, cert_level=VerifyError.CERT_UNKNOWN, details={'max_depth': max_depth, 'actual_depth': depth})",
        "raise MaxDepthExceededError(max_depth, depth)"
    )
    
    # Write back the file
    with open('/app/pysim/pySim/esim/x509_cert.py', 'w') as f:
        f.write(content)
    
    print("Refactored VerifyError hierarchy in x509_cert.py")
    
    # Now update osmo-smdpp.py to use the new exception classes
    with open('/app/pysim/osmo-smdpp.py', 'r') as f:
        content = f.read()
    
    # Find and replace the certificate verification error handling
    old_handler = '''        except VerifyError as e:
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
                raise ApiError('8.1.3', '6.1', f'Certificate chain verification failed: {e}')'''
    
    new_handler = '''        except MissingIntermediateCertError as e:
            # Check if the missing certificate is the EUM cert
            if e.auth_key_id == b2h(cert_get_auth_key_id(eum_cert)):
                raise ApiError('8.1.2', '6.1', 'Invalid EUM Certificate in chain')
            else:
                raise ApiError('8.1.3', '6.1', 'Invalid eUICC Certificate in chain')
        except CertificateRevokedError as e:
            raise ApiError('8.1.3', '6.3', 'Certificate revoked')
        except MaxDepthExceededError as e:
            raise ApiError('8.1.3', '6.1', f'Certificate chain too deep: {e}')
        except SignatureVerificationError as e:
            raise ApiError('8.1.3', '6.1', f'Certificate signature verification failed: {e}')
        except VerifyError as e:
            # Generic certificate chain error for any other VerifyError subclasses
            raise ApiError('8.1.3', '6.1', f'Certificate chain verification failed: {e}')'''
    
    content = content.replace(old_handler, new_handler)
    
    # Add the imports for the new exception classes
    import_line = "from pySim.esim.x509_cert import CertAndPrivkey, CertificateSet, cert_get_subject_key_id, VerifyError"
    new_import_line = "from pySim.esim.x509_cert import (CertAndPrivkey, CertificateSet, cert_get_subject_key_id, \n                                      VerifyError, MissingIntermediateCertError, CertificateRevokedError,\n                                      MaxDepthExceededError, SignatureVerificationError)"
    content = content.replace(import_line, new_import_line)
    
    # Write back the file
    with open('/app/pysim/osmo-smdpp.py', 'w') as f:
        f.write(content)
    
    print("Updated error handling in osmo-smdpp.py")

if __name__ == "__main__":
    refactor_verify_error()