#!/usr/bin/env python3
"""
Refactor to use x509_err module for cleaner imports
"""

import re

def refactor_to_error_module():
    # Read the x509_cert.py file
    with open('/app/pysim/pySim/esim/x509_cert.py', 'r') as f:
        content = f.read()
    
    # Remove the exception classes from x509_cert.py
    # Find and remove all the exception class definitions
    exception_block = r'class VerifyError\(Exception\):.*?class SignatureVerificationError\(VerifyError\):.*?super\(\).__init__\(msg\)\n\n'
    content = re.sub(exception_block, '', content, flags=re.DOTALL)
    
    # Add import for x509_err module after the other imports
    import_section = "from pySim.utils import b2h"
    new_import = "from pySim.utils import b2h\nfrom . import x509_err"
    content = content.replace(import_section, new_import)
    
    # Update all the raise statements to use x509_err prefix
    replacements = [
        ("raise CertificateRevokedError()", "raise x509_err.CertificateRevoked()"),
        ("raise MissingIntermediateCertError(b2h(aki))", "raise x509_err.MissingIntermediateCert(b2h(aki))"),
        ("raise MaxDepthExceededError(max_depth, depth)", "raise x509_err.MaxDepthExceeded(max_depth, depth)"),
    ]
    
    for old, new in replacements:
        content = content.replace(old, new)
    
    # Write back the file
    with open('/app/pysim/pySim/esim/x509_cert.py', 'w') as f:
        f.write(content)
    
    print("Updated x509_cert.py to use x509_err module")
    
    # Now update osmo-smdpp.py
    with open('/app/pysim/osmo-smdpp.py', 'r') as f:
        content = f.read()
    
    # Replace the long import line with cleaner version
    old_import = """from pySim.esim.x509_cert import (CertAndPrivkey, CertificateSet, cert_get_subject_key_id, 
                                      VerifyError, MissingIntermediateCertError, CertificateRevokedError,
                                      MaxDepthExceededError, SignatureVerificationError)"""
    new_import = """from pySim.esim.x509_cert import CertAndPrivkey, CertificateSet, cert_get_subject_key_id
from pySim.esim import x509_err"""
    
    content = content.replace(old_import, new_import)
    
    # Update the exception handling to use x509_err prefix
    old_except_block = """        except MissingIntermediateCertError as e:
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
            raise ApiError('8.1.3', '6.1', f'Certificate chain verification failed: {e}')"""
    
    new_except_block = """        except x509_err.MissingIntermediateCert as e:
            # Check if the missing certificate is the EUM cert
            if e.auth_key_id == b2h(cert_get_auth_key_id(eum_cert)):
                raise ApiError('8.1.2', '6.1', 'Invalid EUM Certificate in chain')
            else:
                raise ApiError('8.1.3', '6.1', 'Invalid eUICC Certificate in chain')
        except x509_err.CertificateRevoked as e:
            raise ApiError('8.1.3', '6.3', 'Certificate revoked')
        except x509_err.MaxDepthExceeded as e:
            raise ApiError('8.1.3', '6.1', f'Certificate chain too deep: {e}')
        except x509_err.SignatureVerification as e:
            raise ApiError('8.1.3', '6.1', f'Certificate signature verification failed: {e}')
        except x509_err.VerifyError as e:
            # Generic certificate chain error for any other x509_err subclasses
            raise ApiError('8.1.3', '6.1', f'Certificate chain verification failed: {e}')"""
    
    content = content.replace(old_except_block, new_except_block)
    
    # Write back the file
    with open('/app/pysim/osmo-smdpp.py', 'w') as f:
        f.write(content)
    
    print("Updated osmo-smdpp.py to use x509_err module")

if __name__ == "__main__":
    refactor_to_error_module()