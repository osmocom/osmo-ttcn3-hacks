#!/usr/bin/env python3
"""
Patch the Python SM-DP+ server to properly handle certificate validation errors.
This adds try-except blocks around certificate operations to return proper error codes
instead of crashing with HTTP 500.
"""

import sys
import re

def patch_file(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    # Patch 1: Add import for ExtensionNotFound
    if 'from cryptography.x509.oid import ExtensionOID' not in content:
        content = content.replace(
            'from pySim.esim.x509_cert import oid, cert_policy_has_oid, cert_get_auth_key_id',
            'from pySim.esim.x509_cert import oid, cert_policy_has_oid, cert_get_auth_key_id\nfrom cryptography.x509 import ExtensionNotFound'
        )
    
    # Patch 2: Wrap cert_get_auth_key_id calls in try-except
    # First occurrence around line 858
    old_block1 = """        # Verify that the Root Certificate of the eUICC certificate chain corresponds to the
        # euiccCiPKIdToBeUsed or TODO: euiccCiPKIdToBeUsedV3
        if cert_get_auth_key_id(eum_cert) != ss.ci_cert_id:
            raise ApiError('8.11.1', '3.9', 'Unknown')"""
    
    new_block1 = """        # Verify that the Root Certificate of the eUICC certificate chain corresponds to the
        # euiccCiPKIdToBeUsed or TODO: euiccCiPKIdToBeUsedV3
        try:
            eum_auth_key_id = cert_get_auth_key_id(eum_cert)
        except (ExtensionNotFound, Exception) as e:
            logger.error(f"Failed to get AuthorityKeyIdentifier from EUM certificate: {e}")
            raise ApiError('8.1.2', '6.1', 'Invalid EUM Certificate - missing or invalid AuthorityKeyIdentifier')
        
        if eum_auth_key_id != ss.ci_cert_id:
            raise ApiError('8.11.1', '3.9', 'Unknown')"""
    
    content = content.replace(old_block1, new_block1)
    
    # Patch 3: Add certificate validation checks
    # Find the authenticateClient method and add validation
    auth_client_pattern = r'(def authenticateClient\(self.*?\n.*?# load certificate\n.*?euicc_cert = x509\.load_der_x509_certificate\(euiccCertificate_bin\)\n.*?eum_cert = x509\.load_der_x509_certificate\(eumCertificate_bin\))'
    
    def auth_client_replacement(match):
        original = match.group(0)
        return original + """
        
        # Certificate validation checks
        try:
            # Check if certificates are valid (not expired, proper format, etc.)
            from datetime import datetime, timezone
            now = datetime.now(timezone.utc)
            
            # Check EUM certificate validity
            if eum_cert.not_valid_after_utc < now:
                raise ApiError('8.1.2', '6.3', 'EUM Certificate has expired')
            if eum_cert.not_valid_before_utc > now:
                raise ApiError('8.1.2', '6.1', 'EUM Certificate is not yet valid')
            
            # Check eUICC certificate validity  
            if euicc_cert.not_valid_after_utc < now:
                raise ApiError('8.1.3', '6.3', 'eUICC Certificate has expired')
            if euicc_cert.not_valid_before_utc > now:
                raise ApiError('8.1.3', '6.1', 'eUICC Certificate is not yet valid')
                
        except ApiError:
            raise  # Re-raise our API errors
        except Exception as e:
            logger.error(f"Certificate validation error: {e}")
            raise ApiError('8.1.3', '6.1', 'Certificate validation failed')"""
    
    content = re.sub(auth_client_pattern, auth_client_replacement, content, flags=re.DOTALL)
    
    # Patch 4: Wrap the certificate chain verification in better error handling
    old_verify = """        # Verify the validity of the eUICC certificate chain
        cs = CertificateSet(self.ci_get_cert_for_pkid(ss.ci_cert_id))
        cs.add_intermediate_cert(eum_cert)
        # TODO v3: otherCertsInChain
        try:
            cs.verify_cert_chain(euicc_cert)
        except VerifyError:
            raise ApiError('8.1.3', '6.1', 'Verification failed (certificate chain)')"""
    
    new_verify = """        # Verify the validity of the eUICC certificate chain
        try:
            cs = CertificateSet(self.ci_get_cert_for_pkid(ss.ci_cert_id))
            cs.add_intermediate_cert(eum_cert)
            # TODO v3: otherCertsInChain
            cs.verify_cert_chain(euicc_cert)
        except VerifyError as e:
            # Determine if it's an EUM or eUICC certificate issue
            error_str = str(e).lower()
            if 'eum' in error_str or 'intermediate' in error_str:
                raise ApiError('8.1.2', '6.1', 'Invalid EUM Certificate in chain')
            else:
                raise ApiError('8.1.3', '6.1', 'Invalid eUICC Certificate in chain')
        except Exception as e:
            logger.error(f"Certificate chain verification error: {e}")
            # Check if it's a missing CI certificate issue
            if 'ci_cert_id' in str(e) or 'Unknown' in str(e):
                raise ApiError('8.11.1', '3.9', 'Unknown CI Public Key')
            raise ApiError('8.1.3', '6.1', 'Certificate chain verification failed')"""
    
    content = content.replace(old_verify, new_verify)
    
    # Save the patched file
    with open(filename, 'w') as f:
        f.write(content)
    
    print(f"Patched {filename} successfully")

if __name__ == "__main__":
    patch_file("/app/pysim/osmo-smdpp.py")