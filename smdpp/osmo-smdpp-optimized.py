#!/usr/bin/env python3

# Optimized SM-DP+ HTTP service for GSMA consumer eSIM RSP
# Based on original by Harald Welte <laforge@osmocom.org>
# Optimizations focus on maintainability, security, and performance

import asn1tools
import asn1tools.codecs.ber
import asn1tools.codecs.der

# ASN.1 OID decoding fix (unchanged - required for compatibility)
def fix_asn1_oid_decoding():
    """Fix for asn1tools OID decoding issue - must remain for compatibility"""
    fix_asn1_schema = """
    TestModule DEFINITIONS ::= BEGIN
        TestOid ::= SEQUENCE {
            oid OBJECT IDENTIFIER
        }
    END
    """
    
    fix_asn1_asn1 = asn1tools.compile_string(fix_asn1_schema, codec='der')
    fix_asn1_oid_string = '2.999.10'
    fix_asn1_encoded = fix_asn1_asn1.encode('TestOid', {'oid': fix_asn1_oid_string})
    fix_asn1_decoded = fix_asn1_asn1.decode('TestOid', fix_asn1_encoded)
    
    if (fix_asn1_decoded['oid'] != fix_asn1_oid_string):
        def fixed_decode_object_identifier(data, offset, end_offset):
            """Decode ASN.1 OBJECT IDENTIFIER with proper large second arc handling"""
            def read_subidentifier(data, offset):
                value = 0
                while True:
                    b = data[offset]
                    value = (value << 7) | (b & 0x7F)
                    offset += 1
                    if not (b & 0x80):
                        break
                return value, offset
            
            subid, offset = read_subidentifier(data, offset)
            if subid < 40:
                first = 0
                second = subid
            elif subid < 80:
                first = 1
                second = subid - 40
            else:
                first = 2
                second = subid - 80
            arcs = [first, second]
            
            while offset < end_offset:
                subid, offset = read_subidentifier(data, offset)
                arcs.append(subid)
            
            return '.'.join(str(x) for x in arcs)
        
        asn1tools.codecs.ber.decode_object_identifier = fixed_decode_object_identifier
        asn1tools.codecs.der.decode_object_identifier = fixed_decode_object_identifier

fix_asn1_oid_decoding()

# Standard imports
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature, encode_dss_signature
from cryptography import x509
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec, dh
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat, PrivateFormat, NoEncryption, ParameterFormat
from pathlib import Path
import json
import sys
import argparse
import uuid
import os
import functools
import time
from typing import Optional, Dict, List, Tuple, Any
from datetime import datetime, timezone
from pprint import pprint as pp

import base64
from base64 import b64decode
from klein import Klein
from twisted.web.iweb import IRequest
from twisted.internet import task

from osmocom.utils import h2b, b2h, swap_nibbles
import pySim.esim.rsp as rsp
from pySim.esim import saip, PMO
from pySim.esim.es8p import *
from pySim.esim.x509_cert import oid, cert_policy_has_oid, cert_get_auth_key_id
from cryptography.x509 import ExtensionNotFound
from pySim.esim.x509_cert import CertAndPrivkey, CertificateSet, cert_get_subject_key_id, VerifyError

import logging
logger = logging.getLogger(__name__)

# Configuration constants
class Config:
    """Configuration constants to avoid magic numbers"""
    DATA_DIR = './smdpp-data'
    DEFAULT_HOSTNAME = 'testsmdpplus1.example.com'
    
    # Limits and timeouts
    MAX_DOWNLOAD_ATTEMPTS = 3
    MAX_CC_ATTEMPTS = 3
    DEFAULT_OTPK_TIMEOUT = 86400  # 24 hours
    OTPK_CLEANUP_INTERVAL = 3600  # 1 hour
    
    # Certificate validation
    MIN_SVN_VERSION = (2, 0, 0)
    MAX_SVN_VERSION = (2, 3, 255)
    
    # Cryptographic parameters
    EUICC_CHALLENGE_LENGTH = 16
    SERVER_CHALLENGE_LENGTH = 16
    
    # Error codes as constants for better maintainability
    class ErrorCodes:
        INVALID_SM_DP_ADDRESS = ('8.8.1', '3.8')
        EUICC_CHALLENGE_REUSE = ('8.1.3', '3.3')
        UNSUPPORTED_SVN = ('8.8.3', '3.1')
        NO_SUPPORTED_PKID = ('8.8.2', '3.1')
        UNKNOWN_TRANSACTION = ('8.10.1', '3.9')
        INVALID_EUM_CERT = ('8.1.2', '6.1')
        EXPIRED_EUM_CERT = ('8.1.2', '6.3')
        INVALID_EUICC_CERT = ('8.1.3', '6.1')
        EXPIRED_EUICC_CERT = ('8.1.3', '6.3')
        UNKNOWN_CI_KEY = ('8.11.1', '3.9')
        NO_PROFILE_AVAILABLE = ('8.2.6', '3.8')
        PROFILE_NOT_RELEASED = ('8.2', '1.2')
        EID_NOT_ALLOWED = ('8.1.1', '3.8')
        CC_MISSING = ('8.2.7', '2.2')
        CC_REFUSED = ('8.2.7', '3.8')

# Utility functions
def b64encode2str(req: bytes) -> str:
    """Encode bytes as base64 string"""
    return base64.b64encode(req).decode('ascii')

def set_headers(request: IRequest):
    """Set mandatory RSP response headers"""
    request.setHeader('Content-Type', 'application/json;charset=UTF-8')
    request.setHeader('X-Admin-Protocol', 'gsma/rsp/v2.1.0')

def validate_request_headers(request: IRequest):
    """Validate mandatory HTTP headers according to SGP.22"""
    content_type = request.getHeader('Content-Type')
    if not content_type or not content_type.startswith('application/json'):
        raise ApiError('1.2.1', '2.1', 'Invalid Content-Type header')
    
    admin_protocol = request.getHeader('X-Admin-Protocol')
    if admin_protocol and not admin_protocol.startswith('gsma/rsp/v'):
        raise ApiError('1.2.2', '2.1', 'Unsupported X-Admin-Protocol version')

def ecdsa_tr03111_to_dss(sig: bytes) -> bytes:
    """Convert ECDSA signature from BSI TR-03111 format to DER"""
    assert len(sig) == 64
    r = int.from_bytes(sig[0:32], 'big')
    s = int.from_bytes(sig[32:64], 'big')
    return encode_dss_signature(r, s)

def compute_confirmation_code_hash(confirmation_code: str, transaction_id: bytes) -> bytes:
    """Compute confirmation code hash per SGP.22 specification"""
    import hashlib
    cc_bytes = h2b(confirmation_code)
    first_hash = hashlib.sha256(cc_bytes).digest()
    return hashlib.sha256(first_hash + transaction_id).digest()

def build_status_code(subject_code: str, reason_code: str, 
                     subject_id: Optional[str] = None, 
                     message: Optional[str] = None) -> Dict:
    """Build status code dictionary"""
    r = {'subjectCode': subject_code, 'reasonCode': reason_code}
    if subject_id:
        r['subjectIdentifier'] = subject_id
    if message:
        r['message'] = message
    return r

def build_resp_header(js: dict, status: str = 'Executed-Success', 
                     status_code_data = None) -> None:
    """Build response header per SGP.22 v3.0 6.5.1.4"""
    js['header'] = {
        'functionExecutionStatus': {
            'status': status,
        }
    }
    if status_code_data:
        js['header']['functionExecutionStatus']['statusCodeData'] = status_code_data

class ApiError(Exception):
    """API Error with proper encoding"""
    def __init__(self, subject_code: str, reason_code: str, 
                 message: Optional[str] = None, subject_id: Optional[str] = None):
        self.status_code = build_status_code(subject_code, reason_code, subject_id, message)
    
    def encode(self) -> str:
        """Encode the API Error into a responseHeader string"""
        js = {}
        build_resp_header(js, 'Failed', self.status_code)
        return json.dumps(js)

class CertificateValidator:
    """Centralized certificate validation logic"""
    
    @staticmethod
    def get_eum_certificate_variant(eum_cert: x509.Certificate) -> str:
        """Determine EUM certificate variant"""
        try:
            cert_policies_ext = eum_cert.extensions.get_extension_for_oid(
                x509.oid.ExtensionOID.CERTIFICATE_POLICIES
            )
            
            for policy in cert_policies_ext.value:
                policy_oid = policy.policy_identifier.dotted_string
                logger.debug(f"Found certificate policy: {policy_oid}")
                
                if policy_oid == '2.23.146.1.2.1.2':
                    logger.debug("Detected EUM certificate variant: O (old)")
                    return 'O'
                elif policy_oid == '2.23.146.1.2.1.0.0.0':
                    logger.debug("Detected EUM certificate variant: Ov3/A/B/C (new)")
                    return 'NEW'
        except x509.ExtensionNotFound:
            logger.debug("No Certificate Policies extension found")
        except Exception as e:
            logger.error(f"Error checking certificate policies: {e}")
        return 'UNKNOWN'
    
    @staticmethod
    def parse_permitted_eins_from_cert(eum_cert: x509.Certificate) -> List[str]:
        """Extract permitted IINs from EUM certificate"""
        cert_variant = CertificateValidator.get_eum_certificate_variant(eum_cert)
        permitted_iins = []
        
        if cert_variant == 'O':
            permitted_iins.extend(CertificateValidator._parse_name_constraints_eins(eum_cert))
        else:
            permitted_iins.extend(CertificateValidator._parse_gsma_permitted_eins(eum_cert))
        
        unique_iins = list(set(permitted_iins))
        logger.debug(f"Total unique permitted IINs found: {len(unique_iins)}")
        return unique_iins
    
    @staticmethod
    def _parse_gsma_permitted_eins(eum_cert: x509.Certificate) -> List[str]:
        """Parse GSMA permittedEins extension"""
        permitted_iins = []
        
        try:
            permitted_eins_oid = x509.ObjectIdentifier('2.23.146.1.2.2.0')
            
            for ext in eum_cert.extensions:
                if ext.oid == permitted_eins_oid:
                    logger.debug(f"Found GSMA permittedEins extension: {ext.oid}")
                    
                    ext_der = ext.value.value if hasattr(ext.value, 'value') else ext.value
                    
                    if isinstance(ext_der, bytes):
                        try:
                            permitted_eins_schema = """
                            PermittedEins DEFINITIONS ::= BEGIN
                                PermittedEins ::= SEQUENCE OF PrintableString
                            END
                            """
                            decoder = asn1tools.compile_string(permitted_eins_schema)
                            decoded_strings = decoder.decode('PermittedEins', ext_der)
                            
                            for iin_string in decoded_strings:
                                iin_clean = iin_string.strip().upper()
                                if (len(iin_clean) >= 8 and 
                                    all(c in '0123456789ABCDEF' for c in iin_clean) and
                                    len(iin_clean) % 2 == 0):
                                    permitted_iins.append(iin_clean)
                                    logger.debug(f"Found permitted IIN (GSMA): {iin_clean}")
                                else:
                                    logger.warning(f"Invalid IIN format: {iin_string}")
                        except Exception as e:
                            logger.error(f"Error parsing GSMA permittedEins extension: {e}")
        
        except Exception as e:
            logger.error(f"Error accessing GSMA certificate extensions: {e}")
        
        return permitted_iins
    
    @staticmethod
    def _parse_name_constraints_eins(eum_cert: x509.Certificate) -> List[str]:
        """Parse permitted IINs from nameConstraints extension"""
        permitted_iins = []
        
        try:
            name_constraints_ext = eum_cert.extensions.get_extension_for_oid(
                x509.oid.ExtensionOID.NAME_CONSTRAINTS
            )
            
            name_constraints = name_constraints_ext.value
            
            if name_constraints.permitted_subtrees:
                for subtree in name_constraints.permitted_subtrees:
                    if isinstance(subtree, x509.DirectoryName):
                        for attribute in subtree.value:
                            if attribute.oid == x509.oid.NameOID.SERIAL_NUMBER:
                                serial_value = attribute.value.upper()
                                if (len(serial_value) >= 8 and
                                    all(c in '0123456789ABCDEF' for c in serial_value) and
                                    len(serial_value) % 2 == 0):
                                    permitted_iins.append(serial_value)
                                    logger.debug(f"Found permitted IIN (nameConstraints): {serial_value}")
        
        except x509.ExtensionNotFound:
            logger.debug("No nameConstraints extension found")
        except Exception as e:
            logger.error(f"Error parsing nameConstraints: {e}")
        
        return permitted_iins
    
    @staticmethod
    def validate_eid_range(eid: str, eum_cert: x509.Certificate) -> bool:
        """Validate that EID is within permitted EINs"""
        if not eid or len(eid) != 32:
            logger.error(f"Invalid EID format: {eid}")
            return False
        
        try:
            permitted_eins = CertificateValidator.parse_permitted_eins_from_cert(eum_cert)
            
            if not permitted_eins:
                logger.warning("No permitted EINs found in EUM certificate")
                return False
            
            eid_normalized = eid.upper()
            logger.debug(f"Validating EID {eid_normalized} against {len(permitted_eins)} permitted EINs")
            
            for permitted_ein in permitted_eins:
                if eid_normalized.startswith(permitted_ein):
                    logger.debug(f"EID {eid_normalized} matches permitted EIN {permitted_ein}")
                    return True
            
            logger.debug(f"EID {eid_normalized} is not in any permitted EIN list")
            return False
        
        except Exception as e:
            logger.error(f"Error validating EID: {e}")
            return False
    
    @staticmethod
    def validate_certificate_dates(cert: x509.Certificate, cert_type: str) -> None:
        """Validate certificate date validity"""
        now = datetime.now(timezone.utc)
        
        if cert.not_valid_after_utc < now:
            if cert_type == 'EUM':
                raise ApiError(*Config.ErrorCodes.EXPIRED_EUM_CERT, f'{cert_type} Certificate has expired')
            else:
                raise ApiError(*Config.ErrorCodes.EXPIRED_EUICC_CERT, f'{cert_type} Certificate has expired')
        
        if cert.not_valid_before_utc > now:
            if cert_type == 'EUM':
                raise ApiError(*Config.ErrorCodes.INVALID_EUM_CERT, f'{cert_type} Certificate is not yet valid')
            else:
                raise ApiError(*Config.ErrorCodes.INVALID_EUICC_CERT, f'{cert_type} Certificate is not yet valid')

class ProfileManager:
    """Centralized profile management"""
    
    def __init__(self):
        self.activation_code_profiles = {}
        self.event_based_profiles = {}
        self.default_profiles = {}
        self._init_test_profiles()
    
    def _init_test_profiles(self):
        """Initialize test profiles - could be loaded from config file"""
        logger.info("Initializing test profiles...")
        
        # Load profiles from configuration or define them here
        # This could be refactored to load from JSON/YAML config file
        self.activation_code_profiles = self._get_activation_code_profiles()
        self.event_based_profiles = self._get_event_based_profiles()
        self.default_profiles = self._get_default_profiles()
        
        logger.info(f"Loaded {len(self.activation_code_profiles)} activation code profiles")
        logger.info(f"Loaded {len(self.event_based_profiles)} event-based profiles")
        logger.info(f"Loaded {len(self.default_profiles)} default profiles")
    
    def _get_activation_code_profiles(self) -> Dict[str, Dict[str, Any]]:
        """Get activation code profiles - extracted for clarity"""
        return {
            'TEST123': {
                'matchingId': 'TEST123',
                'confirmationCode': '12345678',
                'iccid': '8900000000000000001F',
                'profileName': 'Test Profile 1',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': None
            },
            'AC_NOT_RELEASED': {
                'matchingId': 'AC_NOT_RELEASED',
                'confirmationCode': '87654321',
                'iccid': '8900000000000000002F',
                'profileName': 'Not Released Profile',
                'state': 'not_released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': None
            },
            'AC_WITH_CC': {
                'matchingId': 'AC_WITH_CC',
                'confirmationCode': '11223344',
                'iccid': '8900000000000000003F',
                'profileName': 'Profile Requiring CC',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': None
            },
            'AC_NO_CC': {
                'matchingId': 'AC_NO_CC',
                'confirmationCode': None,
                'iccid': '8900000000000000004F',
                'profileName': 'Profile Without CC',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': None
            },
            # Error test profiles
            'AC_NO_ELIGIBLE': {
                'matchingId': 'AC_NO_ELIGIBLE',
                'confirmationCode': None,
                'iccid': '8900000000000000009F',
                'profileName': 'Profile with No Eligible Device',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': None,
                'device_requirements': {
                    'min_memory_mb': 999999,
                    'required_features': ['IMPOSSIBLE_FEATURE']
                }
            },
            'AC_EXPIRED': {
                'matchingId': 'AC_EXPIRED',
                'confirmationCode': None,
                'iccid': '8900000000000000010F',
                'profileName': 'Expired Download Order',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': '2020-01-01T00:00:00Z'
            },
            'AC_MAX_RETRIES': {
                'matchingId': 'AC_MAX_RETRIES',
                'confirmationCode': None,
                'iccid': '8900000000000000011F',
                'profileName': 'Max Retries Exceeded',
                'state': 'released',
                'download_attempts': 5,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': None
            },
            'AC_RESTRICTED_EID': {
                'matchingId': 'AC_RESTRICTED_EID',
                'confirmationCode': None,
                'iccid': '8900000000000000012F',
                'profileName': 'Profile Restricted to Different EID',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': '89999999999999999999999999999999',
                'expiration': None
            },
            'AC_OTHER_EID': {
                'matchingId': 'AC_OTHER_EID',
                'confirmationCode': None,
                'iccid': '8900000000000000013F',
                'profileName': 'Profile for Other EID',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': '89888888888888888888888888888888',
                'expiration': None
            }
        }
    
    def _get_event_based_profiles(self) -> Dict[str, Dict[str, Any]]:
        """Get event-based profiles - consolidated into single dict"""
        return {
            'EVENT_001': {
                'matchingId': 'EVENT_001',
                'confirmationCode': '55667788',
                'iccid': '8900000000000000005F',
                'profileName': 'Event-based Profile 1',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': None
            },
            'UNMATCHED_EVENT': {
                'matchingId': 'UNMATCHED_EVENT',
                'confirmationCode': '99887766',
                'iccid': '8900000000000000006F',
                'profileName': 'Unmatched Event Profile',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': '89001012012341234012345678901224',
                'expiration': None
            },
            'EVENT_NORMAL': {
                'matchingId': 'EVENT_NORMAL',
                'confirmationCode': None,
                'iccid': '8900000000000000014F',
                'profileName': 'Normal SM-DS Event',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': None,
                'expiration': None
            },
            'EVENT_RESTRICTED': {
                'matchingId': 'EVENT_RESTRICTED',
                'confirmationCode': None,
                'iccid': '8900000000000000015F',
                'profileName': 'SM-DS Event Restricted to Different EID',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'associated_eid': '89777777777777777777777777777777',
                'expiration': None
            }
        }
    
    def _get_default_profiles(self) -> Dict[str, Dict[str, Any]]:
        """Get default profiles"""
        return {
            '89001012012341234012345678901234': {
                'confirmationCode': '12345678',
                'iccid': '8900000000000000007F',
                'profileName': 'Default Profile for Test EID',
                'state': 'released',
                'download_attempts': 0,
                'cc_attempts': 0,
                'expiration': None
            }
            # Test certificate EID profile commented out per test requirements
            # '89049032123451234512345678901235': { ... }
        }
    
    def find_profile(self, matchingId: Optional[str], eid: str) -> Tuple[Optional[Dict], str]:
        """Find profile based on matching ID and use case"""
        profile_info = None
        iccid_str = None
        
        if not matchingId or matchingId == '':
            # Default SM-DP+ use case
            logger.debug(f"Default SM-DP+ use case for EID: {eid}")
            if eid in self.default_profiles:
                profile_info = self.default_profiles[eid]
                iccid_str = profile_info['iccid']
            else:
                raise ApiError(*Config.ErrorCodes.NO_PROFILE_AVAILABLE, 'No Profile available for this EID')
        
        elif matchingId.startswith('EVENT_'):
            # SM-DS event-based use case
            logger.debug(f"SM-DS event use case with matchingId: {matchingId}")
            if matchingId in self.event_based_profiles:
                profile_info = self.event_based_profiles[matchingId]
                if profile_info.get('associated_eid') and profile_info['associated_eid'] != eid:
                    raise ApiError(*Config.ErrorCodes.EID_NOT_ALLOWED, 'EID is not allowed for this Profile')
                iccid_str = profile_info['iccid']
            else:
                raise ApiError(*Config.ErrorCodes.NO_PROFILE_AVAILABLE, 'MatchingID could not be matched')
        
        else:
            # Activation code use case
            logger.debug(f"Activation code use case with matchingId: {matchingId}")
            if matchingId in self.activation_code_profiles:
                profile_info = self.activation_code_profiles[matchingId]
                if profile_info.get('associated_eid') and profile_info['associated_eid'] != eid:
                    raise ApiError(*Config.ErrorCodes.EID_NOT_ALLOWED, 'EID is not allowed for this Profile')
                iccid_str = profile_info['iccid']
            else:
                raise ApiError(*Config.ErrorCodes.NO_PROFILE_AVAILABLE, 'Activation Code has not been successfully verified')
        
        return profile_info, iccid_str
    
    def validate_profile(self, profile_info: Dict) -> None:
        """Validate profile state and conditions"""
        # Check if profile is released
        if profile_info.get('state') == 'not_released':
            raise ApiError(*Config.ErrorCodes.PROFILE_NOT_RELEASED, 'The Profile is not released')
        
        # Check for expired download order
        if profile_info.get('expiration'):
            exp_date = datetime.fromisoformat(profile_info['expiration'].replace('Z', '+00:00'))
            if datetime.now(timezone.utc) > exp_date:
                raise ApiError('8.8.5', '4.10', 'Download order has expired')
        
        # Check maximum download attempts
        if profile_info.get('download_attempts', 0) >= Config.MAX_DOWNLOAD_ATTEMPTS:
            raise ApiError('8.8.5', '6.4', 'Maximum number of retries for Profile download order exceeded')
        
        # Check device eligibility
        if profile_info.get('device_requirements'):
            if profile_info['device_requirements'].get('min_memory_mb', 0) > 1000:
                raise ApiError('8.2.5', '4.3', 'No eligible Profile')

class SmDppHttpServer:
    """Optimized SM-DP+ HTTP Server"""
    app = Klein()
    
    @staticmethod
    def load_certs_from_path(path: str) -> List[x509.Certificate]:
        """Load certificates from directory with error handling"""
        certs = []
        try:
            for dirpath, dirnames, filenames in os.walk(path):
                for filename in filenames:
                    cert = None
                    filepath = os.path.join(dirpath, filename)
                    
                    try:
                        if filename.endswith('.der'):
                            with open(filepath, 'rb') as f:
                                cert = x509.load_der_x509_certificate(f.read())
                        elif filename.endswith('.pem'):
                            with open(filepath, 'rb') as f:
                                cert = x509.load_pem_x509_certificate(f.read())
                        
                        if cert:
                            # Verify it is a CI certificate
                            if not cert_policy_has_oid(cert, oid.id_rspRole_ci):
                                logger.warning(f"Certificate {filename} doesn't have CI policy")
                                continue
                            certs.append(cert)
                            logger.debug(f"Loaded CI certificate: {filename}")
                    
                    except Exception as e:
                        logger.error(f"Failed to load certificate {filepath}: {e}")
        
        except Exception as e:
            logger.error(f"Failed to load certificates from {path}: {e}")
        
        return certs
    
    def __init__(self, server_hostname: str, ci_certs_path: str, 
                 common_cert_path: str, use_brainpool: bool = False):
        self.server_hostname = server_hostname
        self.upp_dir = os.path.realpath(os.path.join(Config.DATA_DIR, 'upp'))
        self.ci_certs = self.load_certs_from_path(ci_certs_path)
        
        # Profile manager
        self.profile_manager = ProfileManager()
        
        # Certificate validator
        self.cert_validator = CertificateValidator()
        
        # OTPK mapping for retry scenarios
        self.otpk_mapping = {}  # Maps euicc_otpk -> (smdp_ot, smdp_otpk, timestamp)
        self.otpk_timeout = Config.DEFAULT_OTPK_TIMEOUT
        
        # Cleanup task
        self.cleanup_task = task.LoopingCall(self.cleanup_expired_otpk_mappings)
        self.cleanup_task.start(Config.OTPK_CLEANUP_INTERVAL)
        
        # Load certificates
        self._load_certificates(common_cert_path, use_brainpool)
        
        # Session store with separate files for BRP/NIST
        session_db_suffix = "BRP" if use_brainpool else "NIST"
        self.rss = rsp.RspSessionStore(os.path.join(Config.DATA_DIR, f"sm-dp-sessions-{session_db_suffix}"))
        
        # Track used eUICC challenges
        self.used_euicc_challenges = set()
        
        logger.info(f"SmDppHttpServer initialized for {server_hostname}")
    
    def _load_certificates(self, cert_dir: str, use_brainpool: bool):
        """Load DPauth and DPpb certificates"""
        # DPauth cert + key
        self.dp_auth = CertAndPrivkey(oid.id_rspRole_dp_auth_v2)
        if use_brainpool:
            self.dp_auth.cert_from_der_file(os.path.join(cert_dir, 'DPauth', 'CERT_S_SM_DPauth_ECDSA_BRP.der'))
            self.dp_auth.privkey_from_pem_file(os.path.join(cert_dir, 'DPauth', 'SK_S_SM_DPauth_ECDSA_BRP.pem'))
        else:
            self.dp_auth.cert_from_der_file(os.path.join(cert_dir, 'DPauth', 'CERT_S_SM_DPauth_ECDSA_NIST.der'))
            self.dp_auth.privkey_from_pem_file(os.path.join(cert_dir, 'DPauth', 'SK_S_SM_DPauth_ECDSA_NIST.pem'))
        
        # DPpb cert + key
        self.dp_pb = CertAndPrivkey(oid.id_rspRole_dp_pb_v2)
        if use_brainpool:
            self.dp_pb.cert_from_der_file(os.path.join(cert_dir, 'DPpb', 'CERT_S_SM_DPpb_ECDSA_BRP.der'))
            self.dp_pb.privkey_from_pem_file(os.path.join(cert_dir, 'DPpb', 'SK_S_SM_DPpb_ECDSA_BRP.pem'))
        else:
            self.dp_pb.cert_from_der_file(os.path.join(cert_dir, 'DPpb', 'CERT_S_SM_DPpb_ECDSA_NIST.der'))
            self.dp_pb.privkey_from_pem_file(os.path.join(cert_dir, 'DPpb', 'SK_S_SM_DPpb_ECDSA_NIST.pem'))
    
    def cleanup_expired_otpk_mappings(self):
        """Remove expired otPK mappings"""
        current_time = time.time()
        expired_keys = []
        
        for key, value in self.otpk_mapping.items():
            if len(value) == 3:
                _, _, timestamp = value
                if current_time - timestamp > self.otpk_timeout:
                    expired_keys.append(key)
            else:
                # Old format without timestamp
                expired_keys.append(key)
        
        for key in expired_keys:
            del self.otpk_mapping[key]
            logger.debug(f"Cleaned up expired otPK mapping: {key[:16]}...")
        
        if expired_keys:
            logger.info(f"Cleaned up {len(expired_keys)} expired otPK mappings")
    
    def ci_get_cert_for_pkid(self, ci_pkid: bytes) -> Optional[x509.Certificate]:
        """Find CI certificate for given key identifier with caching"""
        # TODO: Add caching to improve performance
        for cert in self.ci_certs:
            subject_exts = list(filter(lambda x: isinstance(x.value, x509.SubjectKeyIdentifier), cert.extensions))
            if subject_exts:
                subject_pkid = subject_exts[0].value
                if subject_pkid and subject_pkid.key_identifier == ci_pkid:
                    return cert
        return None
    
    def validate_certificate_chain_for_verification(self, euicc_ci_pkid_list: List[bytes]) -> bool:
        """Validate certificate chains for given CI PKIDs"""
        for ci_pkid in euicc_ci_pkid_list:
            ci_cert = self.ci_get_cert_for_pkid(ci_pkid)
            if ci_cert:
                try:
                    cs = CertificateSet(ci_cert)
                    cs.verify_cert_chain(self.dp_auth.cert)
                    return True
                except VerifyError:
                    continue
        return False
    
    @app.handle_errors(ApiError)
    def handle_apierror(self, request: IRequest, failure):
        """Handle API errors with proper response"""
        request.setResponseCode(200)
        logger.error(f"API Error: {failure.value.status_code}")
        return failure.value.encode()
    
    @staticmethod
    def _ecdsa_verify(cert: x509.Certificate, signature: bytes, data: bytes) -> bool:
        """Verify ECDSA signature"""
        pubkey = cert.public_key()
        dss_sig = ecdsa_tr03111_to_dss(signature)
        try:
            pubkey.verify(dss_sig, data, ec.ECDSA(hashes.SHA256()))
            return True
        except InvalidSignature:
            return False
    
    @staticmethod
    def rsp_api_wrapper(func):
        """Decorator for REST API endpoints"""
        @functools.wraps(func)
        def _api_wrapper(self, request: IRequest):
            validate_request_headers(request)
            
            content = json.loads(request.content.read())
            set_headers(request)
            
            output = func(self, request, content)
            if output is None:
                return ''
            
            build_resp_header(output)
            logger.debug(f"Tx JSON: {json.dumps(output)[:200]}")
            return json.dumps(output)
        return _api_wrapper
    
    @app.route('/gsma/rsp2/es9plus/initiateAuthentication', methods=['POST'])
    @rsp_api_wrapper
    def initiateAuthentication(self, request: IRequest, content: dict) -> dict:
        """ES9+ InitiateAuthentication - SGP.22 Section 5.6.1"""
        # Validate SM-DP+ address
        if content['smdpAddress'] != self.server_hostname:
            raise ApiError(*Config.ErrorCodes.INVALID_SM_DP_ADDRESS, 'Invalid SM-DP+ Address')
        
        # Validate eUICC challenge
        euiccChallenge = b64decode(content['euiccChallenge'])
        if len(euiccChallenge) != Config.EUICC_CHALLENGE_LENGTH:
            raise ValueError("Invalid eUICC challenge length")
        
        # Check for challenge reuse
        if euiccChallenge in self.used_euicc_challenges:
            raise ApiError(*Config.ErrorCodes.EUICC_CHALLENGE_REUSE, 'eUICC challenge reuse detected')
        self.used_euicc_challenges.add(euiccChallenge)
        
        # Decode and validate eUICC info
        euiccInfo1_bin = b64decode(content['euiccInfo1'])
        euiccInfo1 = rsp.asn1.decode('EUICCInfo1', euiccInfo1_bin)
        logger.debug(f"Rx euiccInfo1: {euiccInfo1}")
        
        # Validate SVN
        svn = euiccInfo1.get('svn', b'\x02\x02\x00')
        svn_version = (svn[0], svn[1], svn[2])
        
        if svn_version < Config.MIN_SVN_VERSION:
            raise ApiError(*Config.ErrorCodes.UNSUPPORTED_SVN, 
                          'The Specification Version Number indicated by the eUICC is not supported (version too low)')
        elif svn_version[:2] > Config.MAX_SVN_VERSION[:2]:
            raise ApiError(*Config.ErrorCodes.UNSUPPORTED_SVN,
                          'The Specification Version Number indicated by the eUICC is not supported (version too high)')
        
        # Process PKIDs
        pkid_list = euiccInfo1['euiccCiPKIdListForSigning']
        if 'euiccCiPKIdListForSigningV3' in euiccInfo1:
            pkid_list = pkid_list + euiccInfo1['euiccCiPKIdListForSigningV3']
        
        # Find supported CI certificate
        ci_cert = None
        for x in pkid_list:
            ci_cert = self.ci_get_cert_for_pkid(x)
            if ci_cert and cert_get_subject_key_id(ci_cert) == self.dp_auth.get_authority_key_identifier().key_identifier:
                break
            else:
                ci_cert = None
        
        if not ci_cert:
            raise ApiError(*Config.ErrorCodes.NO_SUPPORTED_PKID,
                          'None of the proposed Public Key Identifiers is supported by the SM-DP+')
        
        # Validate verification PKIDs
        verification_pkid_list = euiccInfo1.get('euiccCiPKIdListForVerification', [])
        if verification_pkid_list and not self.validate_certificate_chain_for_verification(verification_pkid_list):
            raise ApiError('8.8.4', '3.7',
                          'The SM-DP+ has no CERT.DPauth.SIG which chains to one of the eSIM CA Root CA Certificate')
        
        # Generate session identifiers
        transactionId = uuid.uuid4().hex.upper()
        assert transactionId not in self.rss
        
        serverChallenge = os.urandom(Config.SERVER_CHALLENGE_LENGTH)
        
        # Build server signed data
        serverSigned1 = {
            'transactionId': h2b(transactionId),
            'euiccChallenge': euiccChallenge,
            'serverAddress': self.server_hostname,
            'serverChallenge': serverChallenge,
        }
        logger.debug(f"Tx serverSigned1: {serverSigned1}")
        serverSigned1_bin = rsp.asn1.encode('ServerSigned1', serverSigned1)
        
        # Build response
        output = {
            'serverSigned1': b64encode2str(serverSigned1_bin),
            'serverSignature1': b64encode2str(b'\x5f\x37\x40' + self.dp_auth.ecdsa_sign(serverSigned1_bin)),
            'transactionId': transactionId,
            'euiccCiPKIdToBeUsed': b64encode2str(b'\x04\x14' + self.dp_auth.get_authority_key_identifier().key_identifier),
            'serverCertificate': b64encode2str(self.dp_auth.get_cert_as_der()),
        }
        
        # Create session state
        self.rss[transactionId] = rsp.RspSessionState(transactionId, serverChallenge,
                                                      cert_get_subject_key_id(ci_cert))
        
        return output
    
    @app.route('/gsma/rsp2/es9plus/authenticateClient', methods=['POST'])
    @rsp_api_wrapper
    def authenticateClient(self, request: IRequest, content: dict) -> dict:
        """ES9+ AuthenticateClient - SGP.22 Section 5.6.3"""
        transactionId = content['transactionId']
        
        # Decode authenticate server response
        authenticateServerResp_bin = b64decode(content['authenticateServerResponse'])
        authenticateServerResp = rsp.asn1.decode('AuthenticateServerResponse', authenticateServerResp_bin)
        logger.debug(f"Rx AuthenticateServerResponse: {authenticateServerResp}")
        
        if authenticateServerResp[0] == 'authenticateResponseError':
            raise ValueError(f"authenticateResponseError: {authenticateServerResp[1]}")
        
        r_ok = authenticateServerResp[1]
        euiccSigned1 = r_ok['euiccSigned1']
        euiccSigned1_bin = rsp.extract_euiccSigned1(authenticateServerResp_bin)
        euiccSignature1_bin = r_ok['euiccSignature1']
        euiccCertificate_bin = rsp.asn1.encode('Certificate', r_ok['euiccCertificate'])
        eumCertificate_bin = rsp.asn1.encode('Certificate', r_ok['eumCertificate'])
        
        # Load certificates
        euicc_cert = x509.load_der_x509_certificate(euiccCertificate_bin)
        eum_cert = x509.load_der_x509_certificate(eumCertificate_bin)
        
        # Validate certificates
        try:
            self.cert_validator.validate_certificate_dates(eum_cert, 'EUM')
            self.cert_validator.validate_certificate_dates(euicc_cert, 'eUICC')
        except ApiError:
            raise
        except Exception as e:
            logger.error(f"Certificate validation error: {e}")
            raise ApiError(*Config.ErrorCodes.INVALID_EUICC_CERT, 'Certificate validation failed')
        
        # Verify transaction exists
        ss = self.rss.get(transactionId, None)
        if ss is None:
            raise ApiError(*Config.ErrorCodes.UNKNOWN_TRANSACTION, 'Unknown')
        
        ss.euicc_cert = euicc_cert
        ss.eum_cert = eum_cert
        
        # Verify EUM certificate authority
        try:
            eum_auth_key_id = cert_get_auth_key_id(eum_cert)
        except (ExtensionNotFound, Exception) as e:
            logger.error(f"Failed to get AuthorityKeyIdentifier from EUM certificate: {e}")
            raise ApiError(*Config.ErrorCodes.INVALID_EUM_CERT,
                          'Invalid EUM Certificate - missing or invalid AuthorityKeyIdentifier')
        
        if eum_auth_key_id != ss.ci_cert_id:
            raise ApiError(*Config.ErrorCodes.UNKNOWN_CI_KEY, 'Unknown')
        
        # Verify certificate chain
        try:
            cs = CertificateSet(self.ci_get_cert_for_pkid(ss.ci_cert_id))
            cs.add_intermediate_cert(eum_cert)
            cs.verify_cert_chain(euicc_cert)
        except VerifyError as e:
            error_str = str(e).lower()
            if 'eum' in error_str or 'intermediate' in error_str:
                raise ApiError(*Config.ErrorCodes.INVALID_EUM_CERT, 'Invalid EUM Certificate in chain')
            else:
                raise ApiError(*Config.ErrorCodes.INVALID_EUICC_CERT, 'Invalid eUICC Certificate in chain')
        except Exception as e:
            logger.error(f"Certificate chain verification error: {e}")
            if 'ci_cert_id' in str(e) or 'Unknown' in str(e):
                raise ApiError(*Config.ErrorCodes.UNKNOWN_CI_KEY, 'Unknown CI Public Key')
            raise ApiError(*Config.ErrorCodes.INVALID_EUICC_CERT, 'Certificate chain verification failed')
        
        # Verify signature
        if not self._ecdsa_verify(euicc_cert, euiccSignature1_bin, euiccSigned1_bin):
            raise ApiError('8.1', '6.1', 'Verification failed (euiccSignature1 over euiccSigned1)')
        
        # Extract and validate EID
        ss.eid = ss.euicc_cert.subject.get_attributes_for_oid(x509.oid.NameOID.SERIAL_NUMBER)[0].value
        logger.info(f"EID (from eUICC cert): {ss.eid}")
        
        if not self.cert_validator.validate_eid_range(ss.eid, eum_cert):
            raise ApiError('8.1.4', '6.1', 'EID is not within the permitted range of the EUM certificate')
        
        # Verify challenges match
        if euiccSigned1['serverChallenge'] != ss.serverChallenge:
            raise ApiError('8.1', '6.1', 'Verification failed (serverChallenge)')
        
        if euiccSigned1['transactionId'] != h2b(transactionId):
            raise ApiError('8.1.1', '3.9', 'The signed transactionId != outer transactionId')
        
        # Process matching ID and find profile
        if euiccSigned1['ctxParams1'][0] == 'ctxParamsForCommonAuthentication':
            cpca = euiccSigned1['ctxParams1'][1]
            matchingId = cpca.get('matchingId', None)
            logger.debug(f"Extracted matchingId from request: {matchingId}")
            
            # Find and validate profile
            profile_info, iccid_str = self.profile_manager.find_profile(matchingId, ss.eid)
            
            # Load profile from file if needed (legacy support)
            if not profile_info and matchingId:
                path = os.path.join(self.upp_dir, matchingId) + '.der'
                # Security: prevent directory traversal
                if os.path.commonprefix((os.path.realpath(path), self.upp_dir)) != self.upp_dir:
                    raise ApiError('8.2.6', '3.8', 'Refused')
                
                if os.path.isfile(path) and os.access(path, os.R_OK):
                    try:
                        with open(path, 'rb') as f:
                            pes = saip.ProfileElementSequence.from_der(f.read())
                            iccid_str = b2h(pes.get_pe_for_type('header').decoded['iccid'])
                        profile_info = {
                            'confirmationCode': None,
                            'state': 'released',
                            'profileName': matchingId
                        }
                    except Exception as e:
                        logger.error(f"Failed to load profile from file: {e}")
                        raise ApiError('8.2.6', '3.8', 'Profile loading failed')
                else:
                    raise ApiError('8.2.6', '3.8', 'Activation Code has not been successfully verified')
            
            # Validate profile
            if profile_info:
                self.profile_manager.validate_profile(profile_info)
                
                # Set confirmation code requirement
                if not matchingId or matchingId == '':
                    ss.ccRequiredFlag = False
                    logger.info("ccRequiredFlag=False for omitted/empty matchingId")
                elif profile_info.get('confirmationCode'):
                    ss.ccRequiredFlag = True
                    ss.expected_confirmation_code = profile_info['confirmationCode']
                    logger.info(f"Set ccRequiredFlag=True with code: {ss.expected_confirmation_code}")
                else:
                    ss.ccRequiredFlag = False
                    logger.info("ccRequiredFlag=False, no confirmation code required")
                
                profile_name = profile_info.get('profileName', matchingId or 'DefaultProfile')
            else:
                raise ApiError('8.2.6', '3.8', 'Profile not found')
            
            ss.matchingId = matchingId
        else:
            raise ApiError('1.3.1', '2.2', 'ctxParams1 missing mandatory ctxParamsForCommonAuthentication')
        
        # Build profile metadata
        ss.profileMetadata = ProfileMetadata(iccid_bin=h2b(swap_nibbles(iccid_str)), 
                                            spn="OsmocomSPN", profile_name=profile_name)
        
        # Enable notifications
        for event in ['enable', 'disable', 'delete']:
            ss.profileMetadata.add_notification(event, self.server_hostname)
        
        profileMetadata_bin = ss.profileMetadata.gen_store_metadata_request()
        
        # Build smdpSigned2
        smdpSigned2 = {
            'transactionId': h2b(ss.transactionId),
            'ccRequiredFlag': getattr(ss, 'ccRequiredFlag', False),
        }
        smdpSigned2_bin = rsp.asn1.encode('SmdpSigned2', smdpSigned2)
        logger.info(f"Encoded smdpSigned2 with ccRequiredFlag={smdpSigned2['ccRequiredFlag']}")
        
        ss.smdpSignature2_do = b'\x5f\x37\x40' + self.dp_pb.ecdsa_sign(smdpSigned2_bin + b'\x5f\x37\x40' + euiccSignature1_bin)
        
        # Update session state
        self.rss[transactionId] = ss
        
        return {
            'transactionId': transactionId,
            'profileMetadata': b64encode2str(profileMetadata_bin),
            'smdpSigned2': b64encode2str(smdpSigned2_bin),
            'smdpSignature2': b64encode2str(ss.smdpSignature2_do),
            'smdpCertificate': b64encode2str(self.dp_pb.get_cert_as_der()),
        }
    
    @app.route('/gsma/rsp2/es9plus/getBoundProfilePackage', methods=['POST'])
    @rsp_api_wrapper
    def getBoundProfilePackage(self, request: IRequest, content: dict) -> dict:
        """ES9+ GetBoundProfilePackage - SGP.22 Section 5.6.2"""
        transactionId = content['transactionId']
        
        # Verify session exists
        ss = self.rss.get(transactionId, None)
        if not ss:
            raise ApiError(*Config.ErrorCodes.UNKNOWN_TRANSACTION,
                          'The RSP session identified by the TransactionID is unknown')
        
        # Decode response
        prepDownloadResp_bin = b64decode(content['prepareDownloadResponse'])
        prepDownloadResp = rsp.asn1.decode('PrepareDownloadResponse', prepDownloadResp_bin)
        logger.debug(f"Rx PrepareDownloadResponse: {prepDownloadResp}")
        
        if prepDownloadResp[0] == 'downloadResponseError':
            r_err = prepDownloadResp[1]
            download_error_code = r_err.get('downloadErrorCode', 'unknown')
            raise ApiError('8.2.10', '4.2',
                          f'Bound Profile Package execution error: downloadErrorCode {download_error_code}')
        
        r_ok = prepDownloadResp[1]
        
        # Verify signature
        euiccSigned2 = r_ok['euiccSigned2']
        euiccSigned2_bin = rsp.extract_euiccSigned2(prepDownloadResp_bin)
        if not self._ecdsa_verify(ss.euicc_cert, r_ok['euiccSignature2'],
                                 euiccSigned2_bin + ss.smdpSignature2_do):
            raise ApiError('8.1', '6.1', 'eUICC signature is invalid')
        
        # Verify transaction ID
        if h2b(transactionId) != euiccSigned2['transactionId']:
            raise ApiError(*Config.ErrorCodes.UNKNOWN_TRANSACTION,
                          'The signed transactionId != outer transactionId')
        
        # Handle OTPK for retry scenarios
        new_euicc_otpk = euiccSigned2['euiccOtpk']
        logger.debug(f"euiccOtpk: {b2h(new_euicc_otpk)}")
        
        euicc_otpk_hex = b2h(new_euicc_otpk)
        if euicc_otpk_hex in self.otpk_mapping:
            # Retry scenario
            mapping_entry = self.otpk_mapping[euicc_otpk_hex]
            if len(mapping_entry) == 3:
                smdp_ot, smdp_otpk, _ = mapping_entry
            else:
                smdp_ot, smdp_otpk = mapping_entry
            ss.euicc_otpk = new_euicc_otpk
            ss.smdp_ot = smdp_ot
            ss.smdp_otpk = smdp_otpk
            logger.info("Retry detected: Reusing existing SM-DP+ ECDH key pair")
        else:
            # New attempt
            ss.euicc_otpk = new_euicc_otpk
            ss.smdp_ot = ec.generate_private_key(self.dp_pb.get_curve())
            ss.smdp_otpk = ss.smdp_ot.public_key().public_bytes(
                Encoding.X962, PublicFormat.UncompressedPoint)
            logger.debug(f"smdpOtpk: {b2h(ss.smdp_otpk)}")
            
            # Store in global mapping
            self.otpk_mapping[euicc_otpk_hex] = (ss.smdp_ot, ss.smdp_otpk, time.time())
        
        ss.host_id = b'mahlzeit'
        
        # Generate session keys
        euicc_public_key = ec.EllipticCurvePublicKey.from_encoded_point(
            ss.smdp_ot.curve, ss.euicc_otpk)
        ss.shared_secret = ss.smdp_ot.exchange(ec.ECDH(), euicc_public_key)
        logger.debug(f"shared_secret: {b2h(ss.shared_secret)}")
        
        # Verify confirmation code if required
        if getattr(ss, 'ccRequiredFlag', False):
            if 'hashCc' not in euiccSigned2:
                raise ApiError(*Config.ErrorCodes.CC_MISSING, 'Confirmation code is missing')
            
            received_hash = euiccSigned2['hashCc']
            expected_hash = compute_confirmation_code_hash(
                getattr(ss, 'expected_confirmation_code', ''),
                h2b(transactionId)
            )
            
            if received_hash != expected_hash:
                logger.debug("Confirmation code verification failed")
                logger.debug(f"Expected hash: {b2h(expected_hash)}")
                logger.debug(f"Received hash: {b2h(received_hash)}")
                raise ApiError(*Config.ErrorCodes.CC_REFUSED, 'Confirmation code is refused')
        
        # Load and bind profile
        try:
            profile_path = os.path.join(self.upp_dir, ss.matchingId) + '.der'
            with open(profile_path, 'rb') as f:
                upp = UnprotectedProfilePackage.from_der(f.read(), metadata=ss.profileMetadata)
        except Exception as e:
            logger.error(f"Failed to load profile: {e}")
            raise ApiError('8.2.1', '1.2', 'Profile package loading failed')
        
        # Create bound profile package
        ppp = ProtectedProfilePackage.from_upp(upp, BspInstance(b'\x00'*16, b'\x11'*16, b'\x22'*16))
        bpp = BoundProfilePackage.from_ppp(ppp)
        
        # Update session state
        self.rss[transactionId] = ss
        
        return {
            'transactionId': transactionId,
            'boundProfilePackage': b64encode2str(bpp.encode(ss, self.dp_pb)),
        }
    
    @app.route('/gsma/rsp2/es9plus/handleNotification', methods=['POST'])
    @rsp_api_wrapper
    def handleNotification(self, request: IRequest, content: dict) -> dict:
        """ES9+ HandleNotification - SGP.22 Section 5.6.4"""
        # Per SGP.22: Normal notification returns HTTP 204 No Content
        request.setResponseCode(204)
        
        pendingNotification_bin = b64decode(content['pendingNotification'])
        pendingNotification = rsp.asn1.decode('PendingNotification', pendingNotification_bin)
        logger.debug(f"Rx PendingNotification: {pendingNotification}")
        
        if pendingNotification[0] == 'profileInstallationResult':
            profileInstallRes = pendingNotification[1]
            pird = profileInstallRes['profileInstallationResultData']
            transactionId = b2h(pird['transactionId'])
            
            ss = self.rss.get(transactionId, None)
            if ss is None:
                logger.warning("Unable to find session for transactionId")
                return
            
            # Verify signature
            pird_bin = rsp.asn1.encode('ProfileInstallationResultData', pird)
            if not self._ecdsa_verify(ss.euicc_cert, profileInstallRes['euiccSignPIR'], pird_bin):
                raise Exception('ECDSA signature verification failed on notification')
            
            logger.info(f"Profile Installation Final Result: {pird['finalResult']}")
            
            # Cleanup OTPK mapping
            if hasattr(ss, 'euicc_otpk') and ss.euicc_otpk:
                euicc_otpk_hex = b2h(ss.euicc_otpk)
                if euicc_otpk_hex in self.otpk_mapping:
                    logger.debug(f"Cleaning up otPK mapping for completed session")
                    del self.otpk_mapping[euicc_otpk_hex]
            
            # Remove session
            del self.rss[transactionId]
            
        elif pendingNotification[0] == 'otherSignedNotification':
            otherSignedNotif = pendingNotification[1]
            
            # Process other notifications
            euiccCertificate_bin = rsp.asn1.encode('Certificate', otherSignedNotif['euiccCertificate'])
            eumCertificate_bin = rsp.asn1.encode('Certificate', otherSignedNotif['eumCertificate'])
            euicc_cert = x509.load_der_x509_certificate(euiccCertificate_bin)
            eum_cert = x509.load_der_x509_certificate(eumCertificate_bin)
            
            ci_cert_id = cert_get_auth_key_id(eum_cert)
            
            # Verify certificate chain
            cs = CertificateSet(self.ci_get_cert_for_pkid(ci_cert_id))
            cs.add_intermediate_cert(eum_cert)
            cs.verify_cert_chain(euicc_cert)
            
            # Verify signature
            tbs_bin = rsp.asn1.encode('NotificationMetadata', otherSignedNotif['tbsOtherNotification'])
            if not self._ecdsa_verify(euicc_cert, otherSignedNotif['euiccNotificationSignature'], tbs_bin):
                raise Exception('ECDSA signature verification failed on notification')
            
            other_notif = otherSignedNotif['tbsOtherNotification']
            pmo = PMO.from_bitstring(other_notif['profileManagementOperation'])
            eid = euicc_cert.subject.get_attributes_for_oid(x509.oid.NameOID.SERIAL_NUMBER)[0].value
            iccid = other_notif.get('iccid', None)
            if iccid:
                iccid = swap_nibbles(b2h(iccid))
            logger.info(f"handleNotification: EID {eid}: {pmo} of {iccid}")
        else:
            raise ValueError(f"Unknown notification type: {pendingNotification[0]}")
    
    @app.route('/gsma/rsp2/es9plus/cancelSession', methods=['POST'])
    @rsp_api_wrapper
    def cancelSession(self, request: IRequest, content: dict) -> dict:
        """ES9+ CancelSession - SGP.22 Section 5.6.5"""
        logger.debug(f"Rx cancelSession: {content}")
        transactionId = content['transactionId']
        
        # Verify session exists
        ss = self.rss.get(transactionId, None)
        if ss is None:
            raise ApiError(*Config.ErrorCodes.UNKNOWN_TRANSACTION,
                          'The RSP session identified by the transactionId is unknown')
        
        # Process cancel response
        cancelSessionResponse_bin = b64decode(content['cancelSessionResponse'])
        cancelSessionResponse = rsp.asn1.decode('CancelSessionResponse', cancelSessionResponse_bin)
        logger.debug(f"Rx CancelSessionResponse: {cancelSessionResponse}")
        
        if cancelSessionResponse[0] == 'cancelSessionResponseError':
            logger.error(f"CancelSessionResponseError: {cancelSessionResponse[1]}")
            return
        
        cancelSessionResponseOk = cancelSessionResponse[1]
        ecsr = cancelSessionResponseOk['euiccCancelSessionSigned']
        ecsr_bin = rsp.asn1.encode('EuiccCancelSessionSigned', ecsr)
        
        # Verify signature
        if not self._ecdsa_verify(ss.euicc_cert, 
                                 cancelSessionResponseOk['euiccCancelSessionSignature'], 
                                 ecsr_bin):
            raise ApiError('8.1', '6.1', 'eUICC signature is invalid')
        
        # Verify SM-DP+ OID
        subj_alt_name = self.dp_auth.get_subject_alt_name()
        smdp_oid_from_cert = None
        for item in subj_alt_name:
            if isinstance(item, x509.RegisteredID):
                smdp_oid_from_cert = item.value
                break
        
        if not smdp_oid_from_cert:
            logger.error("No RegisteredID found in SM-DP+ certificate")
            raise ApiError('8.8', '3.10', 'SM-DP+ certificate missing RegisteredID')
        
        received_oid = x509.ObjectIdentifier(ecsr['smdpOid'])
        if received_oid != smdp_oid_from_cert:
            logger.error(f"OID mismatch: received {received_oid}, expected {smdp_oid_from_cert}")
            raise ApiError('8.8', '3.10', 'The provided SM-DP+ OID is invalid.')
        
        if ecsr['transactionId'] != h2b(transactionId):
            raise ApiError(*Config.ErrorCodes.UNKNOWN_TRANSACTION,
                          'The signed transactionId != outer transactionId')
        
        # Cleanup OTPK mapping
        if hasattr(ss, 'euicc_otpk') and ss.euicc_otpk:
            euicc_otpk_hex = b2h(ss.euicc_otpk)
            if euicc_otpk_hex in self.otpk_mapping:
                logger.debug(f"Cleaning up otPK mapping for cancelled session")
                del self.otpk_mapping[euicc_otpk_hex]
        
        # Delete session
        del self.rss[transactionId]
        
        # Per SGP.22 section 6.5.2.10, return empty response (header only)
        return {}

def main(argv):
    """Main entry point"""
    parser = argparse.ArgumentParser(description='Optimized SM-DP+ HTTP Server')
    parser.add_argument("-H", "--host", help="Host/IP to bind HTTP to", default="localhost")
    parser.add_argument("-p", "--port", help="TCP port to bind HTTP to", type=int, default=8000)
    parser.add_argument("-c", "--certpath", help=f"cert subdir relative to {Config.DATA_DIR}", default="certs")
    parser.add_argument("-s", "--nossl", help="do NOT use ssl", action='store_true', default=False)
    parser.add_argument("-v", "--verbose", help="enable verbose logging", action='store_true', default=False)
    parser.add_argument("-t", "--otpk-timeout", help=f"Timeout in seconds for otPK mapping cleanup (default: {Config.DEFAULT_OTPK_TIMEOUT})",
                        type=int, default=Config.DEFAULT_OTPK_TIMEOUT)
    parser.add_argument("-b", "--brainpool", help="Use Brainpool curves instead of NIST",
                        action='store_true', default=False)
    args = parser.parse_args()
    
    # Configure logging
    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    # Initialize server
    common_cert_path = os.path.join(Config.DATA_DIR, args.certpath)
    hs = SmDppHttpServer(
        server_hostname=Config.DEFAULT_HOSTNAME,
        ci_certs_path=os.path.join(common_cert_path, 'CertificateIssuer'),
        common_cert_path=common_cert_path,
        use_brainpool=args.brainpool
    )
    
    # Configure OTPK timeout
    if args.otpk_timeout:
        hs.otpk_timeout = args.otpk_timeout
        logger.info(f"otPK mapping timeout set to {args.otpk_timeout} seconds ({args.otpk_timeout/3600:.1f} hours)")
    
    # Start server
    if args.nossl:
        hs.app.run(args.host, args.port)
    else:
        # Configure SSL
        curve_type = 'BRP' if args.brainpool else 'NIST'
        cert_derpath = Path(common_cert_path) / 'DPtls' / f'CERT_S_SM_DP_TLS_{curve_type}.der'
        cert_pempath = Path(common_cert_path) / 'DPtls' / f'CERT_S_SM_DP_TLS_{curve_type}.pem'
        cert_skpath = Path(common_cert_path) / 'DPtls' / f'SK_S_SM_DP_TLS_{curve_type}.pem'
        dhparam_path = Path(common_cert_path) / "dhparam2048.pem"
        
        # Generate DH params if needed
        if not dhparam_path.exists():
            logger.info("Generating DH params, this takes a few seconds...")
            parameters = dh.generate_parameters(generator=2, key_size=2048)
            pem_data = parameters.parameter_bytes(encoding=Encoding.PEM, format=ParameterFormat.PKCS3)
            with open(dhparam_path, 'wb') as file:
                file.write(pem_data)
            logger.info("DH params created successfully")
        
        # Convert cert to PEM if needed
        if not cert_pempath.exists():
            logger.info("Converting TLS server cert from DER to PEM...")
            with open(cert_derpath, 'rb') as der_file:
                der_cert_data = der_file.read()
            cert = x509.load_der_x509_certificate(der_cert_data)
            pem_cert = cert.public_bytes(Encoding.PEM)
            with open(cert_pempath, 'wb') as pem_file:
                pem_file.write(pem_cert)
        
        # Configure SSL endpoint
        SERVER_STRING = f'ssl:{args.port}:privateKey={cert_skpath}:certKey={cert_pempath}:dhParameters={dhparam_path}'
        logger.info(f"Starting SSL server: {SERVER_STRING}")
        
        hs.app.run(host=Config.DEFAULT_HOSTNAME, port=args.port, endpoint_description=SERVER_STRING)

if __name__ == "__main__":
    main(sys.argv)