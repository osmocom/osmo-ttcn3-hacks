#pragma once

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ec.h>
#include <openssl/rand.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/sha.h>
#include <openssl/safestack.h>
#include <openssl/stack.h>
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>




/* Basic OID definitions */
#define ID_RSP                         "2.23.146.1"
#define ID_RSP_CERT_OBJECTS            ID_RSP".2"
#define ID_RSPEXT                      ID_RSP_CERT_OBJECTS".0"
#define ID_RSPROLE                     ID_RSP_CERT_OBJECTS".1"

/* Role OIDs */
#define ID_RSPROLE_CI                  ID_RSPROLE".0"
#define ID_RSPROLE_EUICC               ID_RSPROLE".1"
#define ID_RSPROLE_EUM                 ID_RSPROLE".2"
#define ID_RSPROLE_DP_TLS              ID_RSPROLE".3"
#define ID_RSPROLE_DP_AUTH             ID_RSPROLE".4"
#define ID_RSPROLE_DP_PB               ID_RSPROLE".5"
#define ID_RSPROLE_DS_TLS              ID_RSPROLE".6"
#define ID_RSPROLE_DS_AUTH             ID_RSPROLE".7"

/* Extension OIDs */
#define ID_RSP_EXPDATE                 ID_RSPEXT".1"
#define ID_RSP_TOTAL_PARTIAL_CRL_NUMBER ID_RSPEXT".2"
#define ID_RSP_PARTIAL_CRL_NUMBER      ID_RSPEXT".3"

/* Basic types - using OpenSSL primitive types with size constraints */
/* These would be enforced during encoding/decoding */
#define Octet8                         ASN1_OCTET_STRING
#define Octet4                         ASN1_OCTET_STRING
#define Octet16                        ASN1_OCTET_STRING
#define OctetTo16                      ASN1_OCTET_STRING
#define Octet32                        ASN1_OCTET_STRING
#define Octet1                         ASN1_OCTET_STRING
#define Octet2                         ASN1_OCTET_STRING
#define VersionType                    ASN1_OCTET_STRING
#define Iccid                          ASN1_OCTET_STRING
#define TransactionId                  ASN1_OCTET_STRING

/* Time handling */
#define ExpirationDate                 ASN1_TIME
#define TotalPartialCrlNumber          ASN1_INTEGER
#define PartialCrlNumber               ASN1_INTEGER

/* RemoteOpId enumeration */
typedef struct {
    long value;
} REMOTE_OP_ID;

#define REMOTE_OP_ID_INSTALL_BOUND_PROFILE_PACKAGE 1

DECLARE_ASN1_FUNCTIONS(REMOTE_OP_ID)

/* Profile State enumeration */
typedef struct {
    long value;
} PROFILE_STATE;

#define PROFILE_STATE_DISABLED 0
#define PROFILE_STATE_ENABLED  1

DECLARE_ASN1_FUNCTIONS(PROFILE_STATE)

/* IconType enumeration */
typedef struct {
    long value;
} ICON_TYPE;

#define ICON_TYPE_JPG 0
#define ICON_TYPE_PNG 1

DECLARE_ASN1_FUNCTIONS(ICON_TYPE)

/* ProfileClass enumeration */
typedef struct {
    long value;
} PROFILE_CLASS;

#define PROFILE_CLASS_TEST         0
#define PROFILE_CLASS_PROVISIONING 1
#define PROFILE_CLASS_OPERATIONAL  2

DECLARE_ASN1_FUNCTIONS(PROFILE_CLASS)

/* DownloadErrorCode enumeration */
typedef struct {
    long value;
} DOWNLOAD_ERROR_CODE;

#define DOWNLOAD_ERROR_INVALID_CERTIFICATE  1
#define DOWNLOAD_ERROR_INVALID_SIGNATURE    2
#define DOWNLOAD_ERROR_UNSUPPORTED_CURVE    3
#define DOWNLOAD_ERROR_NO_SESSION_CONTEXT   4
#define DOWNLOAD_ERROR_INVALID_TRANSACTION_ID 5
#define DOWNLOAD_ERROR_UNDEFINED_ERROR      127

DECLARE_ASN1_FUNCTIONS(DOWNLOAD_ERROR_CODE)

/* AuthenticateErrorCode enumeration */
typedef struct {
    long value;
} AUTHENTICATE_ERROR_CODE;

#define AUTHENTICATE_ERROR_INVALID_CERTIFICATE      1
#define AUTHENTICATE_ERROR_INVALID_SIGNATURE        2
#define AUTHENTICATE_ERROR_UNSUPPORTED_CURVE        3
#define AUTHENTICATE_ERROR_NO_SESSION_CONTEXT       4
#define AUTHENTICATE_ERROR_INVALID_OID              5
#define AUTHENTICATE_ERROR_EUICC_CHALLENGE_MISMATCH 6
#define AUTHENTICATE_ERROR_CI_PK_UNKNOWN            7
#define AUTHENTICATE_ERROR_UNDEFINED_ERROR          127

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_ERROR_CODE)

/* CancelSessionReason enumeration */
typedef struct {
    long value;
} CANCEL_SESSION_REASON;

#define CANCEL_SESSION_END_USER_REJECTION   0
#define CANCEL_SESSION_POSTPONED            1
#define CANCEL_SESSION_TIMEOUT              2
#define CANCEL_SESSION_PPR_NOT_ALLOWED      3
#define CANCEL_SESSION_METADATA_MISMATCH    4
#define CANCEL_SESSION_LOAD_BPP_EXECUTION_ERROR 5
#define CANCEL_SESSION_UNDEFINED_REASON     127

DECLARE_ASN1_FUNCTIONS(CANCEL_SESSION_REASON)

/* BppCommandId enumeration */
typedef struct {
    long value;
} BPP_COMMAND_ID;

#define BPP_COMMAND_INITIALISE_SECURE_CHANNEL 0
#define BPP_COMMAND_CONFIGURE_ISDP            1
#define BPP_COMMAND_STORE_METADATA            2
#define BPP_COMMAND_STORE_METADATA2           3
#define BPP_COMMAND_REPLACE_SESSION_KEYS      4
#define BPP_COMMAND_LOAD_PROFILE_ELEMENTS     5

DECLARE_ASN1_FUNCTIONS(BPP_COMMAND_ID)

/* ErrorReason enumeration */
typedef struct {
    long value;
} ERROR_REASON;

#define ERROR_REASON_INCORRECT_INPUT_VALUES                  1
#define ERROR_REASON_INVALID_SIGNATURE                       2
#define ERROR_REASON_INVALID_TRANSACTION_ID                  3
#define ERROR_REASON_UNSUPPORTED_CRT_VALUES                  4
#define ERROR_REASON_UNSUPPORTED_REMOTE_OPERATION_TYPE       5
#define ERROR_REASON_UNSUPPORTED_PROFILE_CLASS               6
#define ERROR_REASON_SCP03T_STRUCTURE_ERROR                  7
#define ERROR_REASON_SCP03T_SECURITY_ERROR                   8
#define ERROR_REASON_INSTALL_FAILED_ICCID_ALREADY_EXISTS     9
#define ERROR_REASON_INSTALL_FAILED_INSUFFICIENT_MEMORY      10
#define ERROR_REASON_INSTALL_FAILED_INTERRUPTION             11
#define ERROR_REASON_INSTALL_FAILED_PE_PROCESSING_ERROR      12
#define ERROR_REASON_INSTALL_FAILED_DATA_MISMATCH            13
#define ERROR_REASON_TEST_PROFILE_INSTALL_FAILED_INVALID_NAA_KEY 14
#define ERROR_REASON_PPR_NOT_ALLOWED                         15
#define ERROR_REASON_INSTALL_FAILED_UNKNOWN_ERROR            127

DECLARE_ASN1_FUNCTIONS(ERROR_REASON)

typedef struct EUICC_INFO1_INNER_st EUICC_INFO1_INNER;
typedef struct EUICC_INFO2_INNER_st EUICC_INFO2_INNER;
typedef struct GET_EUICC_INFO1_REQUEST_INNER_st GET_EUICC_INFO1_REQUEST_INNER;
typedef struct GET_EUICC_INFO2_REQUEST_INNER_st GET_EUICC_INFO2_REQUEST_INNER;
typedef struct PROFILE_INFO_INNER_st PROFILE_INFO_INNER;

typedef struct EUICC_SIGNED2_INNER_st EUICC_SIGNED2_INNER;
typedef struct PREPARE_DOWNLOAD_RESPONSE_OK_INNER_st PREPARE_DOWNLOAD_RESPONSE_OK_INNER;
typedef struct PREPARE_DOWNLOAD_RESPONSE_ERROR_INNER_st PREPARE_DOWNLOAD_RESPONSE_ERROR_INNER;
typedef struct PREPARE_DOWNLOAD_RESPONSE_INNER_st PREPARE_DOWNLOAD_RESPONSE_INNER;
typedef struct GET_BOUND_PROFILE_PACKAGE_REQUEST_INNER_st GET_BOUND_PROFILE_PACKAGE_REQUEST_INNER;

/* EUICC_INFO1_INNER structure */
struct EUICC_INFO1_INNER_st {
    VersionType *svn;
    STACK_OF(ASN1_OCTET_STRING) *euiccCiPKIdListForVerification;
    STACK_OF(ASN1_OCTET_STRING) *euiccCiPKIdListForSigning;
};
DECLARE_ASN1_FUNCTIONS(EUICC_INFO1_INNER)

/* EUICCInfo1 with [32] tag - Tag 'BF20' */
typedef EUICC_INFO1_INNER EUICC_INFO1;
DECLARE_ASN1_FUNCTIONS(EUICC_INFO1)


/* GET_EUICC_INFO1_REQUEST_INNER structure */
struct GET_EUICC_INFO1_REQUEST_INNER_st {
    int dummy; /* OpenSSL doesn't like empty structures */
};
DECLARE_ASN1_FUNCTIONS(GET_EUICC_INFO1_REQUEST_INNER)

/* GetEuiccInfo1Request with [32] tag - Tag 'BF20' */
typedef GET_EUICC_INFO1_REQUEST_INNER GET_EUICC_INFO1_REQUEST;
DECLARE_ASN1_FUNCTIONS(GET_EUICC_INFO1_REQUEST)


/* RSP Capability definition */
typedef struct {
    ASN1_BIT_STRING *flags;
} RSP_CAPABILITY;

/* Define constants for bit positions */
#define RSP_CAPABILITY_ADDITIONAL_PROFILE     0
#define RSP_CAPABILITY_CRL_SUPPORT            1
#define RSP_CAPABILITY_RPM_SUPPORT            2
#define RSP_CAPABILITY_TEST_PROFILE_SUPPORT   3
#define RSP_CAPABILITY_DEVICE_INFO_EXTENSIBILITY_SUPPORT 4
#define RSP_CAPABILITY_SERVICE_SPECIFIC_DATA_SUPPORT 5

DECLARE_ASN1_FUNCTIONS(RSP_CAPABILITY)

/* CertificationDataObject */
typedef struct {
    ASN1_UTF8STRING *platformLabel;
    ASN1_UTF8STRING *discoveryBaseURL;
} CERTIFICATION_DATA_OBJECT;

DECLARE_ASN1_FUNCTIONS(CERTIFICATION_DATA_OBJECT)

/* Forward declaration needed for EUICCInfo2 */
typedef struct uicc_capability_st UICC_CAPABILITY;

/* EUICCInfo2 */
typedef struct {
    VersionType *profileVersion;
    VersionType *svn;
    VersionType *euiccFirmwareVer;
    ASN1_OCTET_STRING *extCardResource;
    UICC_CAPABILITY *uiccCapability;
    VersionType *ts102241Version;
    VersionType *globalplatformVersion;
    RSP_CAPABILITY *rspCapability;
    STACK_OF(ASN1_OCTET_STRING) *euiccCiPKIdListForVerification;
    STACK_OF(ASN1_OCTET_STRING) *euiccCiPKIdListForSigning;
    ASN1_INTEGER *euiccCategory;
    ASN1_BIT_STRING *forbiddenProfilePolicyRules;
    VersionType *ppVersion;
    ASN1_UTF8STRING *sasAcreditationNumber;
    CERTIFICATION_DATA_OBJECT *certificationDataObject;
    ASN1_BIT_STRING *treProperties;
    ASN1_UTF8STRING *treProductReference;
    STACK_OF(VersionType) *additionalEuiccProfilePackageVersions;
} EUICC_INFO2;

DECLARE_ASN1_FUNCTIONS(EUICC_INFO2)

/* GetEuiccInfo2Request */
typedef struct {
    int dummy; /* OpenSSL doesn't like empty structures */
} GET_EUICC_INFO2_REQUEST;

DECLARE_ASN1_FUNCTIONS(GET_EUICC_INFO2_REQUEST)

/* CertificateInfo */
typedef struct {
    ASN1_BIT_STRING *flags;
} CERTIFICATE_INFO;

/* Define constants for bit positions */
#define CERTIFICATE_INFO_RESERVED          0
#define CERTIFICATE_INFO_CERT_SIGNING_X509 1
#define CERTIFICATE_INFO_RFU2              2
#define CERTIFICATE_INFO_RFU3              3
#define CERTIFICATE_INFO_RESERVED2         4
#define CERTIFICATE_INFO_CERT_VERIFICATION_X509 5

DECLARE_ASN1_FUNCTIONS(CERTIFICATE_INFO)

/* DeviceAdditionalFeatureSupport */
typedef struct {
    VersionType *naiSupport;
} DEVICE_ADDITIONAL_FEATURE_SUPPORT;

DECLARE_ASN1_FUNCTIONS(DEVICE_ADDITIONAL_FEATURE_SUPPORT)

/* DeviceCapabilities */
typedef struct {
    VersionType *gsmSupportedRelease;
    VersionType *utranSupportedRelease;
    VersionType *cdma2000onexSupportedRelease;
    VersionType *cdma2000hrpdSupportedRelease;
    VersionType *cdma2000ehrpdSupportedRelease;
    VersionType *eutranEpcSupportedRelease;
    VersionType *contactlessSupportedRelease;
    VersionType *rspCrlSupportedVersion;
    VersionType *nrEpcSupportedRelease;
    VersionType *nr5gcSupportedRelease;
    VersionType *eutran5gcSupportedRelease;
    VersionType *lpaSvn;
    ASN1_BIT_STRING *catSupportedClasses;
    ASN1_INTEGER *euiccFormFactorType;
    DEVICE_ADDITIONAL_FEATURE_SUPPORT *deviceAdditionalFeatureSupport;
} DEVICE_CAPABILITIES;

DECLARE_ASN1_FUNCTIONS(DEVICE_CAPABILITIES)

/* DeviceInfo */
typedef struct {
    Octet4 *tac;
    DEVICE_CAPABILITIES *deviceCapabilities;
    Octet8 *imei;
} DEVICE_INFO;

DECLARE_ASN1_FUNCTIONS(DEVICE_INFO)

/* OperatorId */
typedef struct {
    ASN1_OCTET_STRING *mccMnc;
    ASN1_OCTET_STRING *gid1;
    ASN1_OCTET_STRING *gid2;
} OPERATOR_ID;

DECLARE_ASN1_FUNCTIONS(OPERATOR_ID)

/* NotificationEvent bit string positions */
#define NOTIFICATION_EVENT_NOTIFICATION_INSTALL   0
#define NOTIFICATION_EVENT_NOTIFICATION_ENABLE    1
#define NOTIFICATION_EVENT_NOTIFICATION_DISABLE   2
#define NOTIFICATION_EVENT_NOTIFICATION_DELETE    3

/* NotificationConfigurationInformation */
typedef struct {
    ASN1_BIT_STRING *profileManagementOperation;
    ASN1_UTF8STRING *notificationAddress;
} NOTIFICATION_CONFIGURATION_INFORMATION;

DECLARE_ASN1_FUNCTIONS(NOTIFICATION_CONFIGURATION_INFORMATION)

/* Wrapper for SEQUENCE OF NOTIFICATION_CONFIGURATION_INFORMATION */
typedef struct {
    STACK_OF(NOTIFICATION_CONFIGURATION_INFORMATION) *list;
} NOTIFICATION_CONFIG_INFO_SEQUENCE;

DECLARE_ASN1_FUNCTIONS(NOTIFICATION_CONFIG_INFO_SEQUENCE)

/* VendorSpecificExtension element structure */
typedef struct {
    ASN1_OBJECT *vendorOid;
    void* *vendorSpecificData;
} VENDOR_SPECIFIC_EXTENSION_ELEMENT;

DECLARE_ASN1_FUNCTIONS(VENDOR_SPECIFIC_EXTENSION_ELEMENT)

/* Define VENDOR_SPECIFIC_EXTENSION as a sequence of elements */
typedef struct {
    STACK_OF(VENDOR_SPECIFIC_EXTENSION_ELEMENT) *elements;
} VENDOR_SPECIFIC_EXTENSION;

DECLARE_ASN1_FUNCTIONS(VENDOR_SPECIFIC_EXTENSION)

/* DpProprietaryData */
typedef struct {
    ASN1_OBJECT *dpOid;
    /* Additional data objects would be added here */
} DP_PROPRIETARY_DATA;

DECLARE_ASN1_FUNCTIONS(DP_PROPRIETARY_DATA)

/* PprIds bit string positions */
#define PPR_IDS_PPR_UPDATE_CONTROL  0
#define PPR_IDS_PPR1                1
#define PPR_IDS_PPR2                2

/* ProfileInfo */
typedef struct {
    Iccid *iccid;
    ASN1_OCTET_STRING *isdpAid;
    PROFILE_STATE *profileState;
    ASN1_UTF8STRING *profileNickname;
    ASN1_UTF8STRING *serviceProviderName;
    ASN1_UTF8STRING *profileName;
    ICON_TYPE *iconType;
    ASN1_OCTET_STRING *icon;
    PROFILE_CLASS *profileClass;
    NOTIFICATION_CONFIG_INFO_SEQUENCE *notificationConfigurationInfo;
    OPERATOR_ID *profileOwner;
    DP_PROPRIETARY_DATA *dpProprietaryData;
    ASN1_BIT_STRING *profilePolicyRules;
    VENDOR_SPECIFIC_EXTENSION *serviceSpecificDataStoredInEuicc;
} PROFILE_INFO;

DECLARE_ASN1_FUNCTIONS(PROFILE_INFO)

/* ProfileInfoListError */
typedef struct {
    ASN1_INTEGER *value;
} PROFILE_INFO_LIST_ERROR;

#define PROFILE_INFO_LIST_ERROR_INCORRECT_INPUT_VALUES 1
#define PROFILE_INFO_LIST_ERROR_UNDEFINED_ERROR        127

DECLARE_ASN1_FUNCTIONS(PROFILE_INFO_LIST_ERROR)

/* Wrapper for SEQUENCE OF PROFILE_INFO */
typedef struct {
    STACK_OF(PROFILE_INFO) *list;
} PROFILE_INFO_SEQUENCE;

DECLARE_ASN1_FUNCTIONS(PROFILE_INFO_SEQUENCE)

/* ProfileInfoListResponse - choice structure */
typedef struct {
    int type;
    union {
        PROFILE_INFO_SEQUENCE *profileInfoListOk;
        PROFILE_INFO_LIST_ERROR *profileInfoListError;
    } value;
} PROFILE_INFO_LIST_RESPONSE;

DECLARE_ASN1_FUNCTIONS(PROFILE_INFO_LIST_RESPONSE)

/* Create a separate type for the CHOICE of search criteria */
typedef struct {
    int type;
    union {
        ASN1_OCTET_STRING *isdpAid;  /* APPLICATION 15 */
        Iccid *iccid;                /* APPLICATION 26 */
        PROFILE_CLASS *profileClass; /* [21] explicit tag */
    } value;
} PROFILE_INFO_LIST_SEARCH_CRITERIA;

DECLARE_ASN1_FUNCTIONS(PROFILE_INFO_LIST_SEARCH_CRITERIA)

/* ProfileInfoListRequest */
typedef struct {
    PROFILE_INFO_LIST_SEARCH_CRITERIA *searchCriteria;
    ASN1_OCTET_STRING *tagList;  /* APPLICATION 28 */
} PROFILE_INFO_LIST_REQUEST;

DECLARE_ASN1_FUNCTIONS(PROFILE_INFO_LIST_REQUEST)

/* StoreMetadataRequest */
typedef struct {
    Iccid *iccid;
    ASN1_UTF8STRING *serviceProviderName;
    ASN1_UTF8STRING *profileName;
    ICON_TYPE *iconType;
    ASN1_OCTET_STRING *icon;
    PROFILE_CLASS *profileClass;
    NOTIFICATION_CONFIG_INFO_SEQUENCE *notificationConfigurationInfo;
    OPERATOR_ID *profileOwner;
    ASN1_BIT_STRING *profilePolicyRules;
    VENDOR_SPECIFIC_EXTENSION *serviceSpecificDataStoredInEuicc;
    VENDOR_SPECIFIC_EXTENSION *serviceSpecificDataNotStoredInEuicc;
} STORE_METADATA_REQUEST;

DECLARE_ASN1_FUNCTIONS(STORE_METADATA_REQUEST)

/* UpdateMetadataRequest */
typedef struct {
    ASN1_UTF8STRING *serviceProviderName;
    ASN1_UTF8STRING *profileName;
    ICON_TYPE *iconType;
    ASN1_OCTET_STRING *icon;
    ASN1_BIT_STRING *profilePolicyRules;
    VENDOR_SPECIFIC_EXTENSION *serviceSpecificDataStoredInEuicc;
} UPDATE_METADATA_REQUEST;

DECLARE_ASN1_FUNCTIONS(UPDATE_METADATA_REQUEST)

/* SmdpSigned2 */
typedef struct {
    TransactionId *transactionId;
    ASN1_BOOLEAN ccRequiredFlag;
    ASN1_OCTET_STRING *bppEuiccOtpk;
} SMDP_SIGNED2;

DECLARE_ASN1_FUNCTIONS(SMDP_SIGNED2)

/* PrepareDownloadRequest */
typedef struct {
    SMDP_SIGNED2 *smdpSigned2;
    ASN1_OCTET_STRING *smdpSignature2;
    Octet32 *hashCc;
    X509 *smdpCertificate;
} PREPARE_DOWNLOAD_REQUEST;

DECLARE_ASN1_FUNCTIONS(PREPARE_DOWNLOAD_REQUEST)


typedef ASN1_OCTET_STRING TransactionId;
typedef ASN1_OCTET_STRING Octet32;



// Forward declarations for structures WITH tags (need INNER pattern)
typedef struct PREPARE_DOWNLOAD_RESPONSE_INNER_st PREPARE_DOWNLOAD_RESPONSE_INNER;
typedef struct GET_BOUND_PROFILE_PACKAGE_REQUEST_INNER_st GET_BOUND_PROFILE_PACKAGE_REQUEST_INNER;

// TransactionId is typically an OCTET STRING of 16 bytes
typedef ASN1_OCTET_STRING TransactionId;
typedef ASN1_OCTET_STRING Octet32;



/* EUICCSigned2 inner structure */
typedef struct EUICC_SIGNED2_st {
    TransactionId *transactionId;
    ASN1_OCTET_STRING *euiccOtpk;
    ASN1_OCTET_STRING *hashCc;  /* OPTIONAL */
} EUICC_SIGNED2;
DECLARE_ASN1_FUNCTIONS(EUICC_SIGNED2)

/* PrepareDownloadResponseOk inner structure */
typedef struct PREPARE_DOWNLOAD_RESPONSE_OK_st {
    EUICC_SIGNED2 *euiccSigned2;
    ASN1_OCTET_STRING *euiccSignature2;
} PREPARE_DOWNLOAD_RESPONSE_OK;
DECLARE_ASN1_FUNCTIONS(PREPARE_DOWNLOAD_RESPONSE_OK)

/* PrepareDownloadResponseError structure */
typedef struct PREPARE_DOWNLOAD_RESPONSE_ERROR_st {
    TransactionId *transactionId;
    ASN1_INTEGER *downloadErrorCode;
} PREPARE_DOWNLOAD_RESPONSE_ERROR;
DECLARE_ASN1_FUNCTIONS(PREPARE_DOWNLOAD_RESPONSE_ERROR)


/* Define the CHOICE structure WITHOUT any tagging first */
typedef struct PREPARE_DOWNLOAD_RESPONSE_CHOICE_st {
    int type;
    union {
        PREPARE_DOWNLOAD_RESPONSE_OK *downloadResponseOk;
        PREPARE_DOWNLOAD_RESPONSE_ERROR *downloadResponseError;
    } d;
} PREPARE_DOWNLOAD_RESPONSE_CHOICE;

/* GetBoundProfilePackageRequest inner structure */
typedef struct GET_BOUND_PROFILE_PACKAGE_REQUEST_INNER_st {
    TransactionId *transactionId;
    PREPARE_DOWNLOAD_RESPONSE_CHOICE *prepareDownloadResponse;
} GET_BOUND_PROFILE_PACKAGE_REQUEST_INNER;
DECLARE_ASN1_FUNCTIONS(GET_BOUND_PROFILE_PACKAGE_REQUEST_INNER)


/* CtxParamsForCommonAuthentication */
typedef struct {
    ASN1_UTF8STRING *matchingId;
    DEVICE_INFO *deviceInfo;
} CTX_PARAMS_FOR_COMMON_AUTHENTICATION;

DECLARE_ASN1_FUNCTIONS(CTX_PARAMS_FOR_COMMON_AUTHENTICATION)

/* CtxParams1 */
typedef struct {
    int type;
    union {
        CTX_PARAMS_FOR_COMMON_AUTHENTICATION *ctxParamsForCommonAuthentication;
    } value;
} CTX_PARAMS1;

DECLARE_ASN1_FUNCTIONS(CTX_PARAMS1)

/* ServerSigned1 */
typedef struct {
    TransactionId *transactionId;
    Octet16 *euiccChallenge;
    ASN1_UTF8STRING *serverAddress;
    Octet16 *serverChallenge;
} SERVER_SIGNED1;

DECLARE_ASN1_FUNCTIONS(SERVER_SIGNED1)


/* AuthenticateServerRequest */
typedef struct {
    SERVER_SIGNED1 *serverSigned1;
    ASN1_OCTET_STRING *serverSignature1;
    ASN1_OCTET_STRING *euiccCiPKIdToBeUsed;
    X509 *serverCertificate;
    CTX_PARAMS1 *ctxParams1;
} AUTHENTICATE_SERVER_REQUEST;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_SERVER_REQUEST)

/* EuiccSigned1 */
typedef struct {
    TransactionId *transactionId;
    ASN1_UTF8STRING *serverAddress;
    Octet16 *serverChallenge;
    EUICC_INFO2 *euiccInfo2;
    CTX_PARAMS1 *ctxParams1;
} EUICC_SIGNED1;

DECLARE_ASN1_FUNCTIONS(EUICC_SIGNED1)

/* AuthenticateResponseOk */
typedef struct {
    EUICC_SIGNED1 *euiccSigned1;
    ASN1_OCTET_STRING *euiccSignature1;
    X509 *euiccCertificate;
    X509 *eumCertificate;
} AUTHENTICATE_RESPONSE_OK;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_RESPONSE_OK)

/* AuthenticateResponseError */
typedef struct {
    TransactionId *transactionId;
    AUTHENTICATE_ERROR_CODE *authenticateErrorCode;
} AUTHENTICATE_RESPONSE_ERROR;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_RESPONSE_ERROR)

/* AuthenticateServerResponse */
typedef struct {
    int type;
    union {
        AUTHENTICATE_RESPONSE_OK *authenticateResponseOk;
        AUTHENTICATE_RESPONSE_ERROR *authenticateResponseError;
    } value;
} AUTHENTICATE_SERVER_RESPONSE;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_SERVER_RESPONSE)

/* CancelSessionRequest */
typedef struct {
    TransactionId *transactionId;
    CANCEL_SESSION_REASON *reason;
} CANCEL_SESSION_REQUEST;

DECLARE_ASN1_FUNCTIONS(CANCEL_SESSION_REQUEST)

/* EuiccCancelSessionSigned */
typedef struct {
    TransactionId *transactionId;
    ASN1_OBJECT *smdpOid;
    CANCEL_SESSION_REASON *reason;
} EUICC_CANCEL_SESSION_SIGNED;

DECLARE_ASN1_FUNCTIONS(EUICC_CANCEL_SESSION_SIGNED)

/* CancelSessionResponseOk */
typedef struct {
    EUICC_CANCEL_SESSION_SIGNED *euiccCancelSessionSigned;
    ASN1_OCTET_STRING *euiccCancelSessionSignature;
} CANCEL_SESSION_RESPONSE_OK;

DECLARE_ASN1_FUNCTIONS(CANCEL_SESSION_RESPONSE_OK)

/* CancelSessionResponse */
typedef struct {
    int type;
    union {
        CANCEL_SESSION_RESPONSE_OK *cancelSessionResponseOk;
        ASN1_INTEGER *cancelSessionResponseError;
    } value;
} CANCEL_SESSION_RESPONSE;

DECLARE_ASN1_FUNCTIONS(CANCEL_SESSION_RESPONSE)

/* BoundProfilePackage */
typedef struct {
    struct InitialiseSecureChannelRequest_st *initialiseSecureChannelRequest;
    STACK_OF(ASN1_OCTET_STRING) *firstSequenceOf87;
    STACK_OF(ASN1_OCTET_STRING) *sequenceOf88;
    STACK_OF(ASN1_OCTET_STRING) *secondSequenceOf87;
    STACK_OF(ASN1_OCTET_STRING) *sequenceOf86;
} BOUND_PROFILE_PACKAGE;

DECLARE_ASN1_FUNCTIONS(BOUND_PROFILE_PACKAGE)

/* GetEuiccChallengeRequest */
typedef struct {
    int dummy; /* Empty structure */
} GET_EUICC_CHALLENGE_REQUEST;

DECLARE_ASN1_FUNCTIONS(GET_EUICC_CHALLENGE_REQUEST)

/* GetEuiccChallengeResponse */
typedef struct {
    Octet16 *euiccChallenge;
} GET_EUICC_CHALLENGE_RESPONSE;

DECLARE_ASN1_FUNCTIONS(GET_EUICC_CHALLENGE_RESPONSE)

/* SuccessResult */
typedef struct {
    ASN1_OCTET_STRING *aid;
    ASN1_OCTET_STRING *simaResponse;
} SUCCESS_RESULT;

DECLARE_ASN1_FUNCTIONS(SUCCESS_RESULT)

/* ErrorResult */
typedef struct {
    BPP_COMMAND_ID *bppCommandId;
    ERROR_REASON *errorReason;
    ASN1_OCTET_STRING *simaResponse;
} ERROR_RESULT;

DECLARE_ASN1_FUNCTIONS(ERROR_RESULT)

/* ProfileInstallationResultData */
typedef struct {
    TransactionId *transactionId;
    struct NotificationMetadata_st *notificationMetadata;
    ASN1_OBJECT *smdpOid;
    int finalResultType;
    union {
        SUCCESS_RESULT *successResult;
        ERROR_RESULT *errorResult;
    } finalResult;
} PROFILE_INSTALLATION_RESULT_DATA;

DECLARE_ASN1_FUNCTIONS(PROFILE_INSTALLATION_RESULT_DATA)

/* ProfileInstallationResult */
typedef struct {
    PROFILE_INSTALLATION_RESULT_DATA *profileInstallationResultData;
    ASN1_OCTET_STRING *euiccSignPIR;
} PROFILE_INSTALLATION_RESULT;

DECLARE_ASN1_FUNCTIONS(PROFILE_INSTALLATION_RESULT)

/* NotificationMetadata */
typedef struct {
    ASN1_INTEGER *seqNumber;
    ASN1_BIT_STRING *profileManagementOperation;
    ASN1_UTF8STRING *notificationAddress;
    Iccid *iccid;
} NOTIFICATION_METADATA;

DECLARE_ASN1_FUNCTIONS(NOTIFICATION_METADATA)

/* ListNotificationRequest */
typedef struct {
    ASN1_BIT_STRING *profileManagementOperation;
} LIST_NOTIFICATION_REQUEST;

DECLARE_ASN1_FUNCTIONS(LIST_NOTIFICATION_REQUEST)

/* ListNotificationResponse */
typedef struct {
    int type;
    union {
        STACK_OF(NOTIFICATION_METADATA) *notificationMetadataList;
        ASN1_INTEGER *listNotificationsResultError;
    } value;
} LIST_NOTIFICATION_RESPONSE;

DECLARE_ASN1_FUNCTIONS(LIST_NOTIFICATION_RESPONSE)

/* SetNicknameRequest */
typedef struct {
    Iccid *iccid;
    ASN1_UTF8STRING *profileNickname;
} SET_NICKNAME_REQUEST;

DECLARE_ASN1_FUNCTIONS(SET_NICKNAME_REQUEST)

/* SetNicknameResponse */
typedef struct {
    ASN1_INTEGER *setNicknameResult;
} SET_NICKNAME_RESPONSE;

DECLARE_ASN1_FUNCTIONS(SET_NICKNAME_RESPONSE)

/* ControlRefTemplate */
typedef struct {
    Octet1 *keyType;
    Octet1 *keyLen;
    OctetTo16 *hostId;
} CONTROL_REF_TEMPLATE;

DECLARE_ASN1_FUNCTIONS(CONTROL_REF_TEMPLATE)

/* InitialiseSecureChannelRequest */
typedef struct {
    REMOTE_OP_ID *remoteOpId;
    TransactionId *transactionId;
    CONTROL_REF_TEMPLATE *controlRefTemplate;
    ASN1_OCTET_STRING *smdpOtpk;
    ASN1_OCTET_STRING *smdpSign;
} INITIALISE_SECURE_CHANNEL_REQUEST;

DECLARE_ASN1_FUNCTIONS(INITIALISE_SECURE_CHANNEL_REQUEST)

/* ConfigureISDPRequest */
typedef struct {
    DP_PROPRIETARY_DATA *dpProprietaryData;
} CONFIGURE_ISDP_REQUEST;

DECLARE_ASN1_FUNCTIONS(CONFIGURE_ISDP_REQUEST)

/* ReplaceSessionKeysRequest */
typedef struct {
    ASN1_OCTET_STRING *initialMacChainingValue;
    ASN1_OCTET_STRING *ppkEnc;
    ASN1_OCTET_STRING *ppkCmac;
} REPLACE_SESSION_KEYS_REQUEST;

DECLARE_ASN1_FUNCTIONS(REPLACE_SESSION_KEYS_REQUEST)

/* RetrieveNotificationsListRequest */
typedef struct {
    int type;
    union {
        ASN1_INTEGER *seqNumber;
        ASN1_BIT_STRING *profileManagementOperation;
    } searchCriteria;
} RETRIEVE_NOTIFICATIONS_LIST_REQUEST;

DECLARE_ASN1_FUNCTIONS(RETRIEVE_NOTIFICATIONS_LIST_REQUEST)

/* OtherSignedNotification */
typedef struct {
    NOTIFICATION_METADATA *tbsOtherNotification;
    ASN1_OCTET_STRING *euiccNotificationSignature;
    X509 *euiccCertificate;
    X509 *eumCertificate;
} OTHER_SIGNED_NOTIFICATION;

DECLARE_ASN1_FUNCTIONS(OTHER_SIGNED_NOTIFICATION)

/* PendingNotification */
typedef struct {
    int type;
    union {
        PROFILE_INSTALLATION_RESULT *profileInstallationResult;
        OTHER_SIGNED_NOTIFICATION *otherSignedNotification;
    } value;
} PENDING_NOTIFICATION;

DECLARE_ASN1_FUNCTIONS(PENDING_NOTIFICATION)

/* RetrieveNotificationsListResponse */
typedef struct {
    int type;
    union {
        STACK_OF(PENDING_NOTIFICATION) *notificationList;
        ASN1_INTEGER *notificationsListResultError;
    } value;
} RETRIEVE_NOTIFICATIONS_LIST_RESPONSE;

DECLARE_ASN1_FUNCTIONS(RETRIEVE_NOTIFICATIONS_LIST_RESPONSE)

/* NotificationSentRequest */
typedef struct {
    ASN1_INTEGER *seqNumber;
} NOTIFICATION_SENT_REQUEST;

DECLARE_ASN1_FUNCTIONS(NOTIFICATION_SENT_REQUEST)

/* NotificationSentResponse */
typedef struct {
    ASN1_INTEGER *deleteNotificationStatus;
} NOTIFICATION_SENT_RESPONSE;

DECLARE_ASN1_FUNCTIONS(NOTIFICATION_SENT_RESPONSE)

/* EnableProfileRequest */
typedef struct {
    int type;
    union {
        ASN1_OCTET_STRING *isdpAid;
        Iccid *iccid;
    } profileIdentifier;
    ASN1_BOOLEAN refreshFlag;
} ENABLE_PROFILE_REQUEST;

DECLARE_ASN1_FUNCTIONS(ENABLE_PROFILE_REQUEST)

/* EnableProfileResponse */
typedef struct {
    ASN1_INTEGER *enableResult;
} ENABLE_PROFILE_RESPONSE;

DECLARE_ASN1_FUNCTIONS(ENABLE_PROFILE_RESPONSE)

/* DisableProfileRequest */
typedef struct {
    int type;
    union {
        ASN1_OCTET_STRING *isdpAid;
        Iccid *iccid;
    } profileIdentifier;
    ASN1_BOOLEAN refreshFlag;
} DISABLE_PROFILE_REQUEST;

DECLARE_ASN1_FUNCTIONS(DISABLE_PROFILE_REQUEST)

/* DisableProfileResponse */
typedef struct {
    ASN1_INTEGER *disableResult;
} DISABLE_PROFILE_RESPONSE;

DECLARE_ASN1_FUNCTIONS(DISABLE_PROFILE_RESPONSE)

/* DeleteProfileRequest */
typedef struct {
    int type;
    union {
        ASN1_OCTET_STRING *isdpAid;
        Iccid *iccid;
    } value;
} DELETE_PROFILE_REQUEST;

DECLARE_ASN1_FUNCTIONS(DELETE_PROFILE_REQUEST)

/* DeleteProfileResponse */
typedef struct {
    ASN1_INTEGER *deleteResult;
} DELETE_PROFILE_RESPONSE;

DECLARE_ASN1_FUNCTIONS(DELETE_PROFILE_RESPONSE)

/* EuiccMemoryResetRequest */
typedef struct {
    ASN1_BIT_STRING *resetOptions;
} EUICC_MEMORY_RESET_REQUEST;

#define RESET_OPTIONS_DELETE_OPERATIONAL_PROFILES    0
#define RESET_OPTIONS_DELETE_FIELD_LOADED_TEST_PROFILES 1
#define RESET_OPTIONS_RESET_DEFAULT_SMDP_ADDRESS     2

DECLARE_ASN1_FUNCTIONS(EUICC_MEMORY_RESET_REQUEST)

/* EuiccMemoryResetResponse */
typedef struct {
    ASN1_INTEGER *resetResult;
} EUICC_MEMORY_RESET_RESPONSE;

DECLARE_ASN1_FUNCTIONS(EUICC_MEMORY_RESET_RESPONSE)

/* GetEuiccDataRequest */
typedef struct {
    Octet1 *tagList;
} GET_EUICC_DATA_REQUEST;

DECLARE_ASN1_FUNCTIONS(GET_EUICC_DATA_REQUEST)

/* GetEuiccDataResponse */
typedef struct {
    Octet16 *eidValue;
} GET_EUICC_DATA_RESPONSE;

DECLARE_ASN1_FUNCTIONS(GET_EUICC_DATA_RESPONSE)

/* ProfilePolicyAuthorisationRule */
typedef struct {
    ASN1_BIT_STRING *pprIds;
    STACK_OF(OPERATOR_ID) *allowedOperators;
    ASN1_BIT_STRING *pprFlags;
} PROFILE_POLICY_AUTHORISATION_RULE;

#define PPR_FLAGS_CONSENT_REQUIRED 0

DECLARE_ASN1_FUNCTIONS(PROFILE_POLICY_AUTHORISATION_RULE)

/* RulesAuthorisationTable */
typedef struct {
    STACK_OF(PROFILE_POLICY_AUTHORISATION_RULE) *rules;
} RULES_AUTHORISATION_TABLE;

DECLARE_ASN1_FUNCTIONS(RULES_AUTHORISATION_TABLE)

/* GetRatRequest */
typedef struct {
    int dummy; /* Empty structure */
} GET_RAT_REQUEST;

DECLARE_ASN1_FUNCTIONS(GET_RAT_REQUEST)

/* GetRatResponse */
typedef struct {
    RULES_AUTHORISATION_TABLE *rat;
} GET_RAT_RESPONSE;

DECLARE_ASN1_FUNCTIONS(GET_RAT_RESPONSE)

/* SegmentedCrlList */
typedef struct {
    STACK_OF(X509_CRL) *list;
} SEGMENTED_CRL_LIST;

DECLARE_ASN1_FUNCTIONS(SEGMENTED_CRL_LIST)

/* LoadCRLRequest */
typedef struct {
    X509_CRL *crl;
} LOAD_CRL_REQUEST;

DECLARE_ASN1_FUNCTIONS(LOAD_CRL_REQUEST)

/* LoadCRLResponseOk */
typedef struct {
    STACK_OF(ASN1_INTEGER) *missingParts;
} LOAD_CRL_RESPONSE_OK;

DECLARE_ASN1_FUNCTIONS(LOAD_CRL_RESPONSE_OK)

/* LoadCRLResponse */
typedef struct {
    int type;
    union {
        LOAD_CRL_RESPONSE_OK *loadCRLResponseOk;
        ASN1_INTEGER *loadCRLResponseError;
    } value;
} LOAD_CRL_RESPONSE;

DECLARE_ASN1_FUNCTIONS(LOAD_CRL_RESPONSE)

/* InitiateAuthenticationRequest */
typedef struct {
    Octet16 *euiccChallenge;
    ASN1_UTF8STRING *smdpAddress;
    EUICC_INFO1 *euiccInfo1;
} INITIATE_AUTHENTICATION_REQUEST;

DECLARE_ASN1_FUNCTIONS(INITIATE_AUTHENTICATION_REQUEST)

/* InitiateAuthenticationOkEs9 */
typedef struct {
    TransactionId *transactionId;
    SERVER_SIGNED1 *serverSigned1;
    ASN1_OCTET_STRING *serverSignature1;
    ASN1_OCTET_STRING *euiccCiPKIdToBeUsed;
    X509 *serverCertificate;
} INITIATE_AUTHENTICATION_OK_ES9;

DECLARE_ASN1_FUNCTIONS(INITIATE_AUTHENTICATION_OK_ES9)

/* InitiateAuthenticationResponse */
typedef struct {
    int type;
    union {
        INITIATE_AUTHENTICATION_OK_ES9 *initiateAuthenticationOk;
        ASN1_INTEGER *initiateAuthenticationError;
    } value;
} INITIATE_AUTHENTICATION_RESPONSE;

DECLARE_ASN1_FUNCTIONS(INITIATE_AUTHENTICATION_RESPONSE)

/* AuthenticateClientRequest */
typedef struct {
    TransactionId *transactionId;
    AUTHENTICATE_SERVER_RESPONSE *authenticateServerResponse;
} AUTHENTICATE_CLIENT_REQUEST;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_CLIENT_REQUEST)

/* AuthenticateClientOk */
typedef struct {
    TransactionId *transactionId;
    STORE_METADATA_REQUEST *profileMetaData;
    SMDP_SIGNED2 *smdpSigned2;
    ASN1_OCTET_STRING *smdpSignature2;
    X509 *smdpCertificate;
} AUTHENTICATE_CLIENT_OK;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_CLIENT_OK)

/* AuthenticateClientResponseEs9 */
typedef struct {
    int type;
    union {
        AUTHENTICATE_CLIENT_OK *authenticateClientOk;
        ASN1_INTEGER *authenticateClientError;
    } value;
} AUTHENTICATE_CLIENT_RESPONSE_ES9;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_CLIENT_RESPONSE_ES9)


/* GetBoundProfilePackageOk */
typedef struct {
    TransactionId *transactionId;
    BOUND_PROFILE_PACKAGE *boundProfilePackage;
} GET_BOUND_PROFILE_PACKAGE_OK;

DECLARE_ASN1_FUNCTIONS(GET_BOUND_PROFILE_PACKAGE_OK)

/* GetBoundProfilePackageResponse */
typedef struct {
    int type;
    union {
        GET_BOUND_PROFILE_PACKAGE_OK *getBoundProfilePackageOk;
        ASN1_INTEGER *getBoundProfilePackageError;
    } value;
} GET_BOUND_PROFILE_PACKAGE_RESPONSE;

DECLARE_ASN1_FUNCTIONS(GET_BOUND_PROFILE_PACKAGE_RESPONSE)

/* HandleNotification */
typedef struct {
    PENDING_NOTIFICATION *pendingNotification;
} HANDLE_NOTIFICATION;

DECLARE_ASN1_FUNCTIONS(HANDLE_NOTIFICATION)

/* CancelSessionRequestEs9 */
typedef struct {
    TransactionId *transactionId;
    CANCEL_SESSION_RESPONSE *cancelSessionResponse;
} CANCEL_SESSION_REQUEST_ES9;

DECLARE_ASN1_FUNCTIONS(CANCEL_SESSION_REQUEST_ES9)

/* CancelSessionOk */
typedef struct {
    int dummy; /* Empty structure */
} CANCEL_SESSION_OK;

DECLARE_ASN1_FUNCTIONS(CANCEL_SESSION_OK)

/* CancelSessionResponseEs9 */
typedef struct {
    int type;
    union {
        CANCEL_SESSION_OK *cancelSessionOk;
        ASN1_INTEGER *cancelSessionError;
    } value;
} CANCEL_SESSION_RESPONSE_ES9;

DECLARE_ASN1_FUNCTIONS(CANCEL_SESSION_RESPONSE_ES9)

/* EuiccConfiguredAddressesRequest */
typedef struct {
    int dummy; /* Empty structure */
} EUICC_CONFIGURED_ADDRESSES_REQUEST;

DECLARE_ASN1_FUNCTIONS(EUICC_CONFIGURED_ADDRESSES_REQUEST)

/* EuiccConfiguredAddressesResponse */
typedef struct {
    ASN1_UTF8STRING *defaultDpAddress;
    ASN1_UTF8STRING *rootDsAddress;
} EUICC_CONFIGURED_ADDRESSES_RESPONSE;

DECLARE_ASN1_FUNCTIONS(EUICC_CONFIGURED_ADDRESSES_RESPONSE)

/* ISDRProprietaryApplicationTemplate */
typedef struct {
    VersionType *svn;
    ASN1_BIT_STRING *lpaeSupport;
} ISDR_PROPRIETARY_APPLICATION_TEMPLATE;

#define LPAE_SUPPORT_LPAE_USING_CAT  0
#define LPAE_SUPPORT_LPAE_USING_SCWS 1

DECLARE_ASN1_FUNCTIONS(ISDR_PROPRIETARY_APPLICATION_TEMPLATE)

/* LpaeActivationRequest */
typedef struct {
    ASN1_BIT_STRING *lpaeOption;
} LPAE_ACTIVATION_REQUEST;

#define LPAE_OPTION_ACTIVATE_CAT_BASED_LPAE  0
#define LPAE_OPTION_ACTIVATE_SCWS_BASED_LPAE 1

DECLARE_ASN1_FUNCTIONS(LPAE_ACTIVATION_REQUEST)

/* LpaeActivationResponse */
typedef struct {
    ASN1_INTEGER *lpaeActivationResult;
} LPAE_ACTIVATION_RESPONSE;

DECLARE_ASN1_FUNCTIONS(LPAE_ACTIVATION_RESPONSE)

/* SetDefaultDpAddressRequest */
typedef struct {
    ASN1_UTF8STRING *defaultDpAddress;
} SET_DEFAULT_DP_ADDRESS_REQUEST;

DECLARE_ASN1_FUNCTIONS(SET_DEFAULT_DP_ADDRESS_REQUEST)

/* SetDefaultDpAddressResponse */
typedef struct {
    ASN1_INTEGER *setDefaultDpAddressResult;
} SET_DEFAULT_DP_ADDRESS_RESPONSE;

DECLARE_ASN1_FUNCTIONS(SET_DEFAULT_DP_ADDRESS_RESPONSE)

/* EventEntries */
typedef struct {
    ASN1_UTF8STRING *eventId;
    ASN1_UTF8STRING *rspServerAddress;
} EVENT_ENTRIES;

DECLARE_ASN1_FUNCTIONS(EVENT_ENTRIES)

/* AuthenticateClientOkEs11 */
typedef struct {
    TransactionId *transactionId;
    STACK_OF(EVENT_ENTRIES) *eventEntries;
} AUTHENTICATE_CLIENT_OK_ES11;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_CLIENT_OK_ES11)

/* AuthenticateClientResponseEs11 */
typedef struct {
    int type;
    union {
        AUTHENTICATE_CLIENT_OK_ES11 *authenticateClientOk;
        ASN1_INTEGER *authenticateClientError;
    } value;
} AUTHENTICATE_CLIENT_RESPONSE_ES11;

DECLARE_ASN1_FUNCTIONS(AUTHENTICATE_CLIENT_RESPONSE_ES11)

/* RemoteProfileProvisioningRequest */
typedef struct {
    int type;
    union {
        INITIATE_AUTHENTICATION_REQUEST *initiateAuthenticationRequest;
        AUTHENTICATE_CLIENT_REQUEST *authenticateClientRequest;
        GET_BOUND_PROFILE_PACKAGE_REQUEST_INNER *getBoundProfilePackageRequest;
        CANCEL_SESSION_REQUEST_ES9 *cancelSessionRequestEs9;
        HANDLE_NOTIFICATION *handleNotification;
    } value;
} REMOTE_PROFILE_PROVISIONING_REQUEST;

DECLARE_ASN1_FUNCTIONS(REMOTE_PROFILE_PROVISIONING_REQUEST)

/* RemoteProfileProvisioningResponse */
typedef struct {
    int type;
    union {
        INITIATE_AUTHENTICATION_RESPONSE *initiateAuthenticationResponse;
        AUTHENTICATE_CLIENT_RESPONSE_ES9 *authenticateClientResponseEs9;
        GET_BOUND_PROFILE_PACKAGE_RESPONSE *getBoundProfilePackageResponse;
        CANCEL_SESSION_RESPONSE_ES9 *cancelSessionResponseEs9;
        AUTHENTICATE_CLIENT_RESPONSE_ES11 *authenticateClientResponseEs11;
    } value;
} REMOTE_PROFILE_PROVISIONING_RESPONSE;

DECLARE_ASN1_FUNCTIONS(REMOTE_PROFILE_PROVISIONING_RESPONSE)


// fixme?
typedef struct final_result_st {
    int type;
    union {
        SUCCESS_RESULT *successResult;
        ERROR_RESULT *errorResult;
    } value;
} FINAL_RESULT;

DECLARE_ASN1_FUNCTIONS(FINAL_RESULT)

struct profile_installation_result_data_st {
    TransactionId *transactionId;
    NOTIFICATION_METADATA *notificationMetadata;
    ASN1_OBJECT *smdpOid;
    FINAL_RESULT *finalResult;  /* Separate CHOICE type */
};

/* Add these new type declarations to your header file (rsp_ossl.h) */

/* Forward declarations for new types */
typedef struct RETRIEVE_NOTIFICATIONS_LIST_SEARCH_CRITERIA_st RETRIEVE_NOTIFICATIONS_LIST_SEARCH_CRITERIA;
typedef struct PROFILE_IDENTIFIER_st PROFILE_IDENTIFIER;
typedef struct LOAD_CRL_RESPONSE_ERROR_st LOAD_CRL_RESPONSE_ERROR;
// typedef ASN1_ANY UICC_CAPABILITY;  /* or define properly when you have the imported module */

/* New structure definitions */
struct RETRIEVE_NOTIFICATIONS_LIST_SEARCH_CRITERIA_st {
    int searchCriteriaType;
    union {
        ASN1_INTEGER *seqNumber;
        ASN1_BIT_STRING *profileManagementOperation;
    } searchCriteria;
};

struct PROFILE_IDENTIFIER_st {
    int type;
    union {
        ASN1_OCTET_STRING *isdpAid;  /* APPLICATION 15 */
        Iccid *iccid;                /* APPLICATION 26 */
    } value;
};

struct LOAD_CRL_RESPONSE_ERROR_st {
    long value;
};

/* Update existing structure definitions */
struct RETRIEVE_NOTIFICATIONS_LIST_REQUEST_st {
    RETRIEVE_NOTIFICATIONS_LIST_SEARCH_CRITERIA *searchCriteria;
};

struct ENABLE_PROFILE_REQUEST_st {
    PROFILE_IDENTIFIER *profileIdentifier;
    ASN1_BOOLEAN *refreshFlag;
};

struct DISABLE_PROFILE_REQUEST_st {
    PROFILE_IDENTIFIER *profileIdentifier;
    ASN1_BOOLEAN *refreshFlag;
};

/* Add function declarations for new types */
DECLARE_ASN1_FUNCTIONS(RETRIEVE_NOTIFICATIONS_LIST_SEARCH_CRITERIA)
DECLARE_ASN1_FUNCTIONS(PROFILE_IDENTIFIER)
DECLARE_ASN1_FUNCTIONS(LOAD_CRL_RESPONSE_ERROR)
DECLARE_ASN1_FUNCTIONS(UICC_CAPABILITY)

/* APPLICATION tag wrapper types */
typedef ASN1_OCTET_STRING AID_APPLICATION;
typedef ASN1_OCTET_STRING TAG_LIST_APPLICATION;
typedef ASN1_OCTET_STRING SIGNATURE_APPLICATION;
typedef ASN1_OCTET_STRING PUBLIC_KEY_APPLICATION;

DECLARE_ASN1_FUNCTIONS(AID_APPLICATION)
DECLARE_ASN1_FUNCTIONS(TAG_LIST_APPLICATION)
DECLARE_ASN1_FUNCTIONS(SIGNATURE_APPLICATION)
DECLARE_ASN1_FUNCTIONS(PUBLIC_KEY_APPLICATION)

/* Error codes for LoadCRLResponseError */
#define LOAD_CRL_RESPONSE_ERROR_INVALID_SIGNATURE        1
#define LOAD_CRL_RESPONSE_ERROR_INVALID_CRL_FORMAT       2
#define LOAD_CRL_RESPONSE_ERROR_NOT_ENOUGH_MEMORY_SPACE  3
#define LOAD_CRL_RESPONSE_ERROR_VERIFICATION_KEY_NOT_FOUND 4
#define LOAD_CRL_RESPONSE_ERROR_FRESHER_CRL_ALREADY_LOADED 5
#define LOAD_CRL_RESPONSE_ERROR_BASE_CRL_MISSING         6
#define LOAD_CRL_RESPONSE_ERROR_UNDEFINED_ERROR          127

/* Constants for PROFILE_IDENTIFIER choice types */
#define PROFILE_IDENTIFIER_TYPE_ISDP_AID  0
#define PROFILE_IDENTIFIER_TYPE_ICCID     1

/* Constants for RETRIEVE_NOTIFICATIONS_LIST_REQUEST search criteria types */
#define RETRIEVE_SEARCH_CRITERIA_SEQ_NUMBER  0
#define RETRIEVE_SEARCH_CRITERIA_PROFILE_MGMT_OP 1


#define PREPARE_DOWNLOAD_RESPONSE_CHOICE_OK 0
#define PREPARE_DOWNLOAD_RESPONSE_CHOICE_ERROR 1
