 /*
 * pk11mode.c - Test FIPS or NONFIPS Modes for the NSS PKCS11 api.
 *              The goal of this program is to test every function
 *              entry point of the PKCS11 api at least once.
 *              To test in FIPS mode: pk11mode
 *              To test in NONFIPS mode: pk11mode nonFIPS
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#include <windows.h>
#define LIB_NAME "softokn3.dll"
#include "cryptoki.h"
#else
#include <prprf.h>
#include "prlink.h"
#include "../../lib/softoken/pkcs11.h"
#endif


#define NUM_ELEM(array) (sizeof(array)/sizeof(array[0]))

struct tuple_str {
    CK_RV         errNum;
    const char * errString;
};


typedef struct tuple_str tuple_str;

static const tuple_str errStrings[] = {
CKR_OK                              , "CKR_OK                              " ,
CKR_CANCEL                          , "CKR_CANCEL                          " ,
CKR_HOST_MEMORY                     , "CKR_HOST_MEMORY                     " ,
CKR_SLOT_ID_INVALID                 , "CKR_SLOT_ID_INVALID                 " ,
CKR_GENERAL_ERROR                   , "CKR_GENERAL_ERROR                   " ,
CKR_FUNCTION_FAILED                 , "CKR_FUNCTION_FAILED                 " ,
CKR_ARGUMENTS_BAD                   , "CKR_ARGUMENTS_BAD                   " ,
CKR_NO_EVENT                        , "CKR_NO_EVENT                        " ,
CKR_NEED_TO_CREATE_THREADS          , "CKR_NEED_TO_CREATE_THREADS          " ,
CKR_CANT_LOCK                       , "CKR_CANT_LOCK                       " ,
CKR_ATTRIBUTE_READ_ONLY             , "CKR_ATTRIBUTE_READ_ONLY             " ,
CKR_ATTRIBUTE_SENSITIVE             , "CKR_ATTRIBUTE_SENSITIVE             " ,
CKR_ATTRIBUTE_TYPE_INVALID          , "CKR_ATTRIBUTE_TYPE_INVALID          " ,
CKR_ATTRIBUTE_VALUE_INVALID         , "CKR_ATTRIBUTE_VALUE_INVALID         " ,
CKR_DATA_INVALID                    , "CKR_DATA_INVALID                    " ,
CKR_DATA_LEN_RANGE                  , "CKR_DATA_LEN_RANGE                  " ,
CKR_DEVICE_ERROR                    , "CKR_DEVICE_ERROR                    " ,
CKR_DEVICE_MEMORY                   , "CKR_DEVICE_MEMORY                   " ,
CKR_DEVICE_REMOVED                  , "CKR_DEVICE_REMOVED                  " ,
CKR_ENCRYPTED_DATA_INVALID          , "CKR_ENCRYPTED_DATA_INVALID          " ,
CKR_ENCRYPTED_DATA_LEN_RANGE        , "CKR_ENCRYPTED_DATA_LEN_RANGE        " ,
CKR_FUNCTION_CANCELED               , "CKR_FUNCTION_CANCELED               " ,
CKR_FUNCTION_NOT_PARALLEL           , "CKR_FUNCTION_NOT_PARALLEL           " ,
CKR_FUNCTION_NOT_SUPPORTED          , "CKR_FUNCTION_NOT_SUPPORTED          " ,
CKR_KEY_HANDLE_INVALID              , "CKR_KEY_HANDLE_INVALID              " ,
CKR_KEY_SIZE_RANGE                  , "CKR_KEY_SIZE_RANGE                  " ,
CKR_KEY_TYPE_INCONSISTENT           , "CKR_KEY_TYPE_INCONSISTENT           " ,
CKR_KEY_NOT_NEEDED                  , "CKR_KEY_NOT_NEEDED                  " ,
CKR_KEY_CHANGED                     , "CKR_KEY_CHANGED                     " ,
CKR_KEY_NEEDED                      , "CKR_KEY_NEEDED                      " ,
CKR_KEY_INDIGESTIBLE                , "CKR_KEY_INDIGESTIBLE                " ,
CKR_KEY_FUNCTION_NOT_PERMITTED      , "CKR_KEY_FUNCTION_NOT_PERMITTED      " ,
CKR_KEY_NOT_WRAPPABLE               , "CKR_KEY_NOT_WRAPPABLE               " ,
CKR_KEY_UNEXTRACTABLE               , "CKR_KEY_UNEXTRACTABLE               " ,
CKR_MECHANISM_INVALID               , "CKR_MECHANISM_INVALID               " ,
CKR_MECHANISM_PARAM_INVALID         , "CKR_MECHANISM_PARAM_INVALID         " ,
CKR_OBJECT_HANDLE_INVALID           , "CKR_OBJECT_HANDLE_INVALID           " ,
CKR_OPERATION_ACTIVE                , "CKR_OPERATION_ACTIVE                " ,
CKR_OPERATION_NOT_INITIALIZED       , "CKR_OPERATION_NOT_INITIALIZED       " ,
CKR_PIN_INCORRECT                   , "CKR_PIN_INCORRECT                   " ,
CKR_PIN_INVALID                     , "CKR_PIN_INVALID                     " ,
CKR_PIN_LEN_RANGE                   , "CKR_PIN_LEN_RANGE                   " ,
CKR_PIN_EXPIRED                     , "CKR_PIN_EXPIRED                     " ,
CKR_PIN_LOCKED                      , "CKR_PIN_LOCKED                      " ,
CKR_SESSION_CLOSED                  , "CKR_SESSION_CLOSED                  " ,
CKR_SESSION_COUNT                   , "CKR_SESSION_COUNT                   " ,
CKR_SESSION_HANDLE_INVALID          , "CKR_SESSION_HANDLE_INVALID          " ,
CKR_SESSION_PARALLEL_NOT_SUPPORTED  , "CKR_SESSION_PARALLEL_NOT_SUPPORTED  " ,
CKR_SESSION_READ_ONLY               , "CKR_SESSION_READ_ONLY               " ,
CKR_SESSION_EXISTS                  , "CKR_SESSION_EXISTS                  " ,
CKR_SESSION_READ_ONLY_EXISTS        , "CKR_SESSION_READ_ONLY_EXISTS        " ,
CKR_SESSION_READ_WRITE_SO_EXISTS    , "CKR_SESSION_READ_WRITE_SO_EXISTS    " ,
CKR_SIGNATURE_INVALID               , "CKR_SIGNATURE_INVALID               " ,
CKR_SIGNATURE_LEN_RANGE             , "CKR_SIGNATURE_LEN_RANGE             " ,
CKR_TEMPLATE_INCOMPLETE             , "CKR_TEMPLATE_INCOMPLETE             " ,
CKR_TEMPLATE_INCONSISTENT           , "CKR_TEMPLATE_INCONSISTENT           " ,
CKR_TOKEN_NOT_PRESENT               , "CKR_TOKEN_NOT_PRESENT               " ,
CKR_TOKEN_NOT_RECOGNIZED            , "CKR_TOKEN_NOT_RECOGNIZED            " ,
CKR_TOKEN_WRITE_PROTECTED           , "CKR_TOKEN_WRITE_PROTECTED           " ,
CKR_UNWRAPPING_KEY_HANDLE_INVALID   , "CKR_UNWRAPPING_KEY_HANDLE_INVALID   " ,
CKR_UNWRAPPING_KEY_SIZE_RANGE       , "CKR_UNWRAPPING_KEY_SIZE_RANGE       " ,
CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT, "CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT" ,
CKR_USER_ALREADY_LOGGED_IN          , "CKR_USER_ALREADY_LOGGED_IN          " ,
CKR_USER_NOT_LOGGED_IN              , "CKR_USER_NOT_LOGGED_IN              " ,
CKR_USER_PIN_NOT_INITIALIZED        , "CKR_USER_PIN_NOT_INITIALIZED        " ,
CKR_USER_TYPE_INVALID               , "CKR_USER_TYPE_INVALID               " ,
CKR_USER_ANOTHER_ALREADY_LOGGED_IN  , "CKR_USER_ANOTHER_ALREADY_LOGGED_IN  " ,
CKR_USER_TOO_MANY_TYPES             , "CKR_USER_TOO_MANY_TYPES             " ,
CKR_WRAPPED_KEY_INVALID             , "CKR_WRAPPED_KEY_INVALID             " ,
CKR_WRAPPED_KEY_LEN_RANGE           , "CKR_WRAPPED_KEY_LEN_RANGE           " ,
CKR_WRAPPING_KEY_HANDLE_INVALID     , "CKR_WRAPPING_KEY_HANDLE_INVALID     " ,
CKR_WRAPPING_KEY_SIZE_RANGE         , "CKR_WRAPPING_KEY_SIZE_RANGE         " ,
CKR_WRAPPING_KEY_TYPE_INCONSISTENT  , "CKR_WRAPPING_KEY_TYPE_INCONSISTENT  " ,
CKR_RANDOM_SEED_NOT_SUPPORTED       , "CKR_RANDOM_SEED_NOT_SUPPORTED       " ,
CKR_RANDOM_NO_RNG                   , "CKR_RANDOM_NO_RNG                   " ,
CKR_DOMAIN_PARAMS_INVALID           , "CKR_DOMAIN_PARAMS_INVALID           " ,
CKR_BUFFER_TOO_SMALL                , "CKR_BUFFER_TOO_SMALL                " ,
CKR_SAVED_STATE_INVALID             , "CKR_SAVED_STATE_INVALID             " ,
CKR_INFORMATION_SENSITIVE           , "CKR_INFORMATION_SENSITIVE           " ,
CKR_STATE_UNSAVEABLE                , "CKR_STATE_UNSAVEABLE                " ,
CKR_CRYPTOKI_NOT_INITIALIZED        , "CKR_CRYPTOKI_NOT_INITIALIZED        " ,
CKR_CRYPTOKI_ALREADY_INITIALIZED    , "CKR_CRYPTOKI_ALREADY_INITIALIZED    " ,
CKR_MUTEX_BAD                       , "CKR_MUTEX_BAD                       " ,
CKR_MUTEX_NOT_LOCKED                , "CKR_MUTEX_NOT_LOCKED                " ,
CKR_FUNCTION_REJECTED               , "CKR_FUNCTION_REJECTED               " ,
CKR_VENDOR_DEFINED                  , "CKR_VENDOR_DEFINED                  "
};

static const CK_ULONG numStrings = sizeof(errStrings) / sizeof(tuple_str);

/* Returns constant error string for "CRV".
 * Returns NULL of errNum is unknown.
 */
const char *
PKM_CK_RVtoStr(CK_RV errNum) {
    CK_ULONG low  = 1;
    CK_ULONG high = numStrings - 1;
    CK_ULONG i;
    CK_RV num;
    static int initDone;

    /* make sure table is in  ascending order.
     * binary search depends on it.
     */
    if (!initDone) {
        CK_RV lastNum = CKR_OK;
        for (i = low; i <= high; ++i) {
            num = errStrings[i].errNum;
            if (num <= lastNum) {
                    fprintf(stderr,
"sequence error in error strings at item %d\n"
"error %d (%s)\n"
"should come after \n"
"error %d (%s)\n",
                        i, lastNum, errStrings[i-1].errString,
                        num, errStrings[i].errString);
            }
            lastNum = num;
        }
        initDone = 1;
    }

    /* Do binary search of table. */
    while (low + 1 < high) {
        i = (low + high) / 2;
        num = errStrings[i].errNum;
        if (errNum == num)
            return errStrings[i].errString;
        if (errNum < num)
            high = i;
        else
            low = i;
    }
    if (errNum == errStrings[low].errNum)
            return errStrings[low].errString;
    if (errNum == errStrings[high].errNum)
            return errStrings[high].errString;
    return NULL;
}


typedef struct CK_C_INITIALIZE_ARGS_NSS {
    CK_CREATEMUTEX CreateMutex;
    CK_DESTROYMUTEX DestroyMutex;
    CK_LOCKMUTEX LockMutex;
    CK_UNLOCKMUTEX UnlockMutex;
    CK_FLAGS flags;
    /* The official PKCS #11 spec does not have a 'LibraryParameters' field, but
     * a reserved field. NSS needs a way to pass instance-specific information
     * to the library (like where to find its config files, etc). This
     * information is usually provided by the installer and passed uninterpreted
     * by NSS to the library, though NSS does know the specifics of the softoken
     * version of this parameter. Most compliant PKCS#11 modules expect this
     * parameter to be NULL, and will return CKR_ARGUMENTS_BAD from
     * C_Initialize if Library parameters is supplied. */
    CK_CHAR_PTR *LibraryParameters;
    /* This field is only present if the LibraryParameters is not NULL. It must
     * be NULL in all cases */
    CK_VOID_PTR pReserved;
} CK_C_INITIALIZE_ARGS_NSS;

static CK_ATTRIBUTE_TYPE all_known_attribute_types[] = {
    CKA_CLASS,
    CKA_TOKEN,
    CKA_PRIVATE,
    CKA_LABEL,
    CKA_APPLICATION,
    CKA_VALUE,
    CKA_CERTIFICATE_TYPE,
    CKA_ISSUER,
    CKA_SERIAL_NUMBER,
    CKA_KEY_TYPE,
    CKA_SUBJECT,
    CKA_ID,
    CKA_SENSITIVE,
    CKA_ENCRYPT,
    CKA_DECRYPT,
    CKA_WRAP,
    CKA_UNWRAP,
    CKA_SIGN,
    CKA_SIGN_RECOVER,
    CKA_VERIFY,
    CKA_VERIFY_RECOVER,
    CKA_DERIVE,
    CKA_START_DATE,
    CKA_END_DATE,
    CKA_MODULUS,
    CKA_MODULUS_BITS,
    CKA_PUBLIC_EXPONENT,
    CKA_PRIVATE_EXPONENT,
    CKA_PRIME_1,
    CKA_PRIME_2,
    CKA_EXPONENT_1,
    CKA_EXPONENT_2,
    CKA_COEFFICIENT,
    CKA_PRIME,
    CKA_SUBPRIME,
    CKA_BASE,
    CKA_VALUE_BITS,
    CKA_VALUE_LEN,
    CKA_EXTRACTABLE,
    CKA_LOCAL,
    CKA_NEVER_EXTRACTABLE,
    CKA_ALWAYS_SENSITIVE,
    CKA_MODIFIABLE,
#ifdef CKA_NETSCAPE
    CKA_NETSCAPE_URL,
    CKA_NETSCAPE_EMAIL,
    CKA_NETSCAPE_SMIME_INFO,
    CKA_NETSCAPE_SMIME_TIMESTAMP,
    CKA_NETSCAPE_PKCS8_SALT,
    CKA_NETSCAPE_PASSWORD_CHECK,
    CKA_NETSCAPE_EXPIRES,
#endif /* CKA_NETSCAPE */
#ifdef CKA_TRUST
    CKA_TRUST_DIGITAL_SIGNATURE,
    CKA_TRUST_NON_REPUDIATION,
    CKA_TRUST_KEY_ENCIPHERMENT,
    CKA_TRUST_DATA_ENCIPHERMENT,
    CKA_TRUST_KEY_AGREEMENT,
    CKA_TRUST_KEY_CERT_SIGN,
    CKA_TRUST_CRL_SIGN,
    CKA_TRUST_SERVER_AUTH,
    CKA_TRUST_CLIENT_AUTH,
    CKA_TRUST_CODE_SIGNING,
    CKA_TRUST_EMAIL_PROTECTION,
    CKA_TRUST_IPSEC_END_SYSTEM,
    CKA_TRUST_IPSEC_TUNNEL,
    CKA_TRUST_IPSEC_USER,
    CKA_TRUST_TIME_STAMPING,
#endif /* CKA_TRUST */
};

static int number_of_all_known_attribute_types =
(sizeof(all_known_attribute_types)/sizeof(all_known_attribute_types[0]));

#define MAX_SIG_SZ 128
#define DIGESTLEN       64
//#define DATASIZE        8192
#define MAX_CIPHER_SZ 128
#define MAX_DATA_SZ 64
#define MAX_DIGEST_LEN 64
#define HMAC_MAX_LENGTH 64
#define FIPSMODE 0
#define NONFIPSMODE 1
#define HYBRIDMODE 2
#define NOMODE 3
int MODE = FIPSMODE;

CK_BBOOL true = CK_TRUE;
CK_BBOOL false = CK_FALSE;

static const char * slotFlagName[] = {
    "CKF_TOKEN_PRESENT",
    "CKF_REMOVABLE_DEVICE",
    "CKF_HW_SLOT",
    "unknown token flag 0x00000008",
    "unknown token flag 0x00000010",
    "unknown token flag 0x00000020",
    "unknown token flag 0x00000040",
    "unknown token flag 0x00000080",
    "unknown token flag 0x00000100",
    "unknown token flag 0x00000200",
    "unknown token flag 0x00000400",
    "unknown token flag 0x00000800",
    "unknown token flag 0x00001000",
    "unknown token flag 0x00002000",
    "unknown token flag 0x00004000",
    "unknown token flag 0x00008000"
    "unknown token flag 0x00010000",
    "unknown token flag 0x00020000",
    "unknown token flag 0x00040000",
    "unknown token flag 0x00080000",
    "unknown token flag 0x00100000",
    "unknown token flag 0x00200000",
    "unknown token flag 0x00400000",
    "unknown token flag 0x00800000"
    "unknown token flag 0x01000000",
    "unknown token flag 0x02000000",
    "unknown token flag 0x04000000",
    "unknown token flag 0x08000000",
    "unknown token flag 0x10000000",
    "unknown token flag 0x20000000",
    "unknown token flag 0x40000000",
    "unknown token flag 0x80000000"
};

static const char * tokenFlagName[] = {
    "CKF_PKM_RNG",
    "CKF_WRITE_PROTECTED",
    "CKF_LOGIN_REQUIRED",
    "CKF_USER_PIN_INITIALIZED",
    "unknown token flag 0x00000010",
    "CKF_RESTORE_KEY_NOT_NEEDED",
    "CKF_CLOCK_ON_TOKEN",
    "unknown token flag 0x00000080",
    "CKF_PROTECTED_AUTHENTICATION_PATH",
    "CKF_DUAL_CRYPTO_OPERATIONS",
    "CKF_TOKEN_INITIALIZED",
    "CKF_SECONDARY_AUTHENTICATION",
    "unknown token flag 0x00001000",
    "unknown token flag 0x00002000",
    "unknown token flag 0x00004000",
    "CKF_THREAD_SAFE (nonstandard 0x00008000)",
    "CKF_USER_PIN_COUNT_LOW",
    "CKF_USER_PIN_FINAL_TRY",
    "CKF_USER_PIN_LOCKED",
    "CKF_USER_PIN_TO_BE_CHANGED",
    "CKF_SO_PIN_COUNT_LOW",
    "CKF_SO_PIN_FINAL_TRY",
    "CKF_SO_PIN_LOCKED",
    "CKF_SO_PIN_TO_BE_CHANGED",
    "unknown token flag 0x01000000",
    "unknown token flag 0x02000000",
    "unknown token flag 0x04000000",
    "unknown token flag 0x08000000",
    "unknown token flag 0x10000000",
    "unknown token flag 0x20000000",
    "unknown token flag 0x40000000",
    "unknown token flag 0x80000000"
};

static const unsigned char TLSClientRandom[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0d, 0x90, 0xbb, 0x5e, 0xc6, 0xe1, 0x3f, 0x71,
    0x0a, 0xa2, 0x70, 0x5a, 0x4f, 0xbc, 0x3f, 0x0d
};
static const unsigned char TLSServerRandom[] = {
    0x00, 0x00, 0x1d, 0x4a, 0x7a, 0x0a, 0xa5, 0x01,
    0x8e, 0x79, 0x72, 0xde, 0x9e, 0x2f, 0x8a, 0x0d,
    0xed, 0xb2, 0x5d, 0xf1, 0x14, 0xc2, 0xc6, 0x66,
    0x95, 0x86, 0xb0, 0x0d, 0x87, 0x2a, 0x2a, 0xc9
};

typedef enum {
    CORRECT,
    BOGUS_CLIENT_RANDOM,
    BOGUS_CLIENT_RANDOM_LEN,
    BOGUS_SERVER_RANDOM,
    BOGUS_SERVER_RANDOM_LEN
} enum_random_t;

void
dumpToHash64(const unsigned char *buf, unsigned int bufLen)
{
    unsigned int i;
    for (i = 0; i < bufLen; i += 8) {
        if (i % 32 == 0)
            printf("\n");
        printf(" 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,",
               buf[i  ], buf[i+1], buf[i+2], buf[i+3],
               buf[i+4], buf[i+5], buf[i+6], buf[i+7]);
    }
    printf("\n");
}


#ifdef _WIN32
HMODULE hModule;
#else
PRLibrary *lib;
#endif

/*
* All api that belongs to pk11mode.c layer start with the prefix PKM_
*/
void PKM_LogIt(const char *fmt, ...);
void PKM_Error(const char *fmt, ...);
CK_SLOT_ID *PKM_GetSlotList(CK_FUNCTION_LIST_PTR pFunctionList,
                            CK_ULONG slotID);
CK_RV PKM_ShowInfo(CK_FUNCTION_LIST_PTR pFunctionList, CK_ULONG slotID);
CK_RV PKM_InitPWforDB(CK_FUNCTION_LIST_PTR pFunctionList,
                      CK_SLOT_ID *pSlotList, CK_ULONG slotID,
                      CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_Mechanism(CK_FUNCTION_LIST_PTR pFunctionList,
                    CK_SLOT_ID * pSlotList, CK_ULONG slotID);
CK_RV PKM_RNG(CK_FUNCTION_LIST_PTR pFunctionList, CK_SLOT_ID * pSlotList,
              CK_ULONG slotID);
CK_RV PKM_SessionLogin(CK_FUNCTION_LIST_PTR pFunctionList,
                       CK_SLOT_ID *pSlotList, CK_ULONG slotID,
                       CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_SecretKey(CK_FUNCTION_LIST_PTR pFunctionList, CK_SLOT_ID *pSlotList,
                    CK_ULONG slotID, CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_DualFuncDigest(CK_FUNCTION_LIST_PTR pFunctionList,
                         CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                         CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_PublicKey(CK_FUNCTION_LIST_PTR pFunctionList, CK_SLOT_ID *pSlotList,
                    CK_ULONG slotID, CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_HybridMode(CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_FindAllObjects (CK_FUNCTION_LIST_PTR pFunctionList,
                          CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                          CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_MultiObjectManagement (CK_FUNCTION_LIST_PTR pFunctionList,
                                 CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                                 CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_OperationalState (CK_FUNCTION_LIST_PTR pFunctionList,
                            CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                            CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_LegacyFunctions(CK_FUNCTION_LIST_PTR pFunctionList,
                          CK_SLOT_ID *pSlotList, CK_ULONG slotID,
                          CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_RecoverFunctions(CK_FUNCTION_LIST_PTR pFunctionList,
                           CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                           CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_wrapUnwrap(CK_FUNCTION_LIST_PTR pFunctionList,
                     CK_SLOT_ID *pSlotList, CK_ULONG slotID,
                     CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_AttributeCheck(CK_FUNCTION_LIST_PTR pFunctionList,
                         CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE obj,
                         CK_ATTRIBUTE_PTR expected_attrs,
                         CK_ULONG expected_attrs_count);
CK_RV PKM_MechCheck(CK_FUNCTION_LIST_PTR pFunctionList,
                    CK_SESSION_HANDLE hSession, CK_MECHANISM_TYPE mechType,
                    CK_FLAGS flags, CK_BBOOL check_sizes,
                    CK_ULONG minkeysize, CK_ULONG maxkeysize);
CK_RV PKM_TLSKeyAndMacDerive(CK_FUNCTION_LIST_PTR pFunctionList,
                            CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                            CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen,
                            CK_MECHANISM_TYPE mechType, enum_random_t rnd);
CK_RV PKM_TLSMasterKeyDerive(CK_FUNCTION_LIST_PTR pFunctionList,
                            CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                            CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen,
                            CK_MECHANISM_TYPE mechType,
                            enum_random_t rnd);
CK_RV PKM_KeyTests(CK_FUNCTION_LIST_PTR pFunctionList,
                       CK_SLOT_ID *pSlotList, CK_ULONG slotID,
                       CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen);
CK_RV PKM_DualFuncSign(CK_FUNCTION_LIST_PTR pFunctionList,
                      CK_SESSION_HANDLE hRwSession,
                      CK_OBJECT_HANDLE publicKey, CK_OBJECT_HANDLE privateKey,
                      CK_MECHANISM *sigMech, CK_OBJECT_HANDLE secretKey,
                      CK_MECHANISM *cryptMech, 
                      CK_BYTE *pData, CK_ULONG pDataLen);
CK_RV PKM_PubKeySign(CK_FUNCTION_LIST_PTR pFunctionList, 
                     CK_SESSION_HANDLE hRwSession,
                     CK_OBJECT_HANDLE hPubKey, CK_OBJECT_HANDLE hPrivKey,
                     CK_MECHANISM *signMech, CK_BYTE *pData, CK_ULONG dataLen);
CK_RV PKM_SecKeyCrypt(CK_FUNCTION_LIST_PTR pFunctionList, 
                      CK_SESSION_HANDLE hSession,
                      CK_OBJECT_HANDLE hSymKey, CK_MECHANISM *cryptMech,
                      CK_BYTE *pData, CK_ULONG dataLen);
CK_RV PKM_Hmac(CK_FUNCTION_LIST_PTR pFunctionList, CK_SESSION_HANDLE hSession,
                CK_OBJECT_HANDLE sKey, CK_MECHANISM *hmacMech, 
                CK_BYTE *pData, CK_ULONG pDataLen);
CK_RV PKM_Digest(CK_FUNCTION_LIST_PTR pFunctionList, 
                 CK_SESSION_HANDLE hRwSession,
                 CK_MECHANISM *digestMech, CK_OBJECT_HANDLE hSecretKey,
                 CK_BYTE *pData, CK_ULONG pDataLen);

int main(int argc, char **argv)
{
    CK_C_GetFunctionList pC_GetFunctionList;
    CK_FUNCTION_LIST_PTR pFunctionList;
    CK_RV crv = CKR_OK;
    CK_C_INITIALIZE_ARGS_NSS initArgs;
    CK_SLOT_ID *pSlotList = NULL;
    CK_UTF8CHAR pwd[] ="1Mozilla";
    CK_TOKEN_INFO tokenInfo;
    CK_ULONG slotID;

    if (argc == 2) {
        if (strcmp(argv[1], "FIPS") == 0) {
            MODE = FIPSMODE;
        } else {
            MODE = NONFIPSMODE;
        }
    } else MODE = FIPSMODE;

#ifdef _WIN32
    hModule = LoadLibrary(LIB_NAME);
    if (hModule == NULL) {
        PKM_Error( "cannot load %s\n", LIB_NAME);
        exit(1);
    }
    if (MODE == FIPSMODE) {
        /* FIPS mode == FC_GetFunctionList */
        pC_GetFunctionList = (CK_C_GetFunctionList)
                             GetProcAddress(hModule, "FC_GetFunctionList");
        slotID = 0;
        PKM_LogIt("loading FC_GetFunctionList for FIPS Mode; slotID %d \n",
                  slotID);
        PKM_LogIt("pFunctionList->C_Foo == pFunctionList->FC_Foo\n");

    } else {
        /* NON FIPS mode  == C_GetFunctionList */
        pC_GetFunctionList = (CK_C_GetFunctionList)
                             GetProcAddress(hModule, "C_GetFunctionList");
        slotID = 1;
        PKM_LogIt("loading C_GetFunctionList for Non FIPS Mode; slotID %d \n",
                  slotID);
    }
    if (pC_GetFunctionList == NULL) {
        PKM_Error( "cannot load %s\n", LIB_NAME);
        exit(1);
    }
#else
    char *libname;
    /* Get the platform-dependent library name of the NSS cryptographic module */
    libname = PR_GetLibraryName(NULL, "softokn3");
    assert(libname != NULL);
    lib = PR_LoadLibrary(libname);
    assert(lib != NULL);
    PR_FreeLibraryName(libname);
    
    if (MODE == FIPSMODE) {
        pC_GetFunctionList = (CK_C_GetFunctionList) PR_FindFunctionSymbol(lib,
        "FC_GetFunctionList");
        assert(pC_GetFunctionList != NULL);
    } else {
        pC_GetFunctionList = (CK_C_GetFunctionList) PR_FindFunctionSymbol(lib,
        "C_GetFunctionList");
        assert(pC_GetFunctionList != NULL);
    }
#endif

    crv = (*pC_GetFunctionList)(&pFunctionList);
    assert(crv == CKR_OK);

    initArgs.CreateMutex = NULL;
    initArgs.DestroyMutex = NULL;
    initArgs.LockMutex = NULL;
    initArgs.UnlockMutex = NULL;
    initArgs.flags = CKF_OS_LOCKING_OK;
    initArgs.LibraryParameters = (CK_CHAR_PTR *)
    "configdir='.' certPrefix='' keyPrefix='' secmod='secmod.db' flags= ";
    initArgs.pReserved = NULL;


    /*DebugBreak();*/
    /* FIPSMODE invokes FC_Initialize as pFunctionList->C_Initialize */
    /* NSS cryptographic module library initialization for the FIPS  */
    /* Approved mode when FC_Initialize is envoked will perfom       */
    /* software integrity test, and power-up self-tests before       */
    /* FC_Initialize returns                                         */
    crv = pFunctionList->C_Initialize(&initArgs);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Initialize succeeded\n");
    } else {
        PKM_Error( "C_Initialize failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = PKM_ShowInfo(pFunctionList, slotID);
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_ShowInfo succeeded\n");
    } else {
        PKM_Error( "PKM_ShowInfo failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    pSlotList = PKM_GetSlotList(pFunctionList, slotID);
    if (pSlotList == NULL) {
        PKM_Error( "PKM_GetSlotList failed with \n");
        exit(1);
    }
    crv = pFunctionList->C_GetTokenInfo(pSlotList[slotID], &tokenInfo);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GetTokenInfo succeeded\n");
    } else {
        PKM_Error( "C_GetTokenInfo failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }

    if (!(tokenInfo.flags & CKF_USER_PIN_INITIALIZED)) {
        PKM_LogIt("Initing PW for DB\n");
        PKM_InitPWforDB(pFunctionList, pSlotList, slotID,
                        pwd, sizeof(pwd));
    } else {
        PKM_LogIt("using existing DB\n");
    }

    /* general mechanism by token  
    crv = PKM_Mechanism(pFunctionList, pSlotList, slotID);
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_Mechanism succeeded\n");
    } else {
        PKM_Error( "PKM_Mechanism failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    } */
    /* RNG example without Login */
    crv = PKM_RNG(pFunctionList, pSlotList, slotID);
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_RNG succeeded\n");
    } else {
        PKM_Error( "PKM_RNG failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }

    crv = PKM_SessionLogin(pFunctionList, pSlotList, slotID,
                           pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_SessionLogin succeeded\n");
    } else {
        PKM_Error( "PKM_SessionLogin failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }

    crv = PKM_DualFuncDigest(pFunctionList, pSlotList, slotID,
                         pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_DualFuncDigest succeeded\n");
    } else {
        PKM_Error( "PKM_DualFuncDigest failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    /*
     * PKM_KeyTest creates RSA,DSA public keys 
     * and AES, DES3 secret keys.
     * then does digest, hmac, encrypt/decrypt, signing operations. 
     */
    crv = PKM_KeyTests(pFunctionList, pSlotList, slotID,
                            pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_DualFuncSign succeeded\n");
    } else {
        PKM_Error( "PKM_DualFuncSign failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }

    crv = PKM_SecretKey(pFunctionList, pSlotList, slotID, pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_SecretKey succeeded\n");
    } else {
        PKM_Error( "PKM_SecretKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }

    crv = PKM_PublicKey(pFunctionList, pSlotList, slotID,
                        pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_PublicKey succeeded\n");
    } else {
        PKM_Error( "PKM_PublicKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = PKM_OperationalState(pFunctionList, pSlotList, slotID,
                               pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_OperationalState succeeded\n");
    } else {
        PKM_Error( "PKM_OperationalState failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = PKM_MultiObjectManagement(pFunctionList, pSlotList, slotID,
                                    pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_MultiObjectManagement succeeded\n");
    } else {
        PKM_Error( "PKM_MultiObjectManagement failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = PKM_RecoverFunctions(pFunctionList, pSlotList, slotID,
                               pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_RecoverFunctions succeeded\n");
    } else {
        PKM_Error( "PKM_RecoverFunctions failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }

    crv = PKM_wrapUnwrap(pFunctionList, pSlotList, slotID,
                         pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_wrapUnwrap succeeded\n");
    } else {
        PKM_Error( "PKM_wrapUnwrap failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }

    crv = PKM_LegacyFunctions(pFunctionList, pSlotList, slotID,
                              pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_LegacyFunctions succeeded\n");
    } else {
        PKM_Error( "PKM_LegacyFunctions failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = PKM_TLSKeyAndMacDerive(pFunctionList, pSlotList, slotID,
                                 pwd, sizeof(pwd),
                                 CKM_TLS_KEY_AND_MAC_DERIVE, CORRECT);

    if (crv == CKR_OK) {
        PKM_LogIt("PKM_TLSKeyAndMacDerive succeeded\n");
    } else {
        PKM_Error( "PKM_TLSKeyAndMacDerive failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = PKM_TLSMasterKeyDerive(pFunctionList, pSlotList, slotID,
                                 pwd, sizeof(pwd),CKM_TLS_MASTER_KEY_DERIVE,
                                 CORRECT);
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_TLSMasterKeyDerive succeeded\n");
    } else {
        PKM_Error( "PKM_TLSMasterKeyDerive failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = PKM_TLSMasterKeyDerive(pFunctionList, pSlotList, slotID,
                                 pwd, sizeof(pwd),CKM_TLS_MASTER_KEY_DERIVE_DH,
                                 CORRECT);
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_TLSMasterKeyDerive succeeded\n");
    } else {
        PKM_Error( "PKM_TLSMasterKeyDerive failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = PKM_FindAllObjects(pFunctionList, pSlotList, slotID,
                             pwd, sizeof(pwd));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_FindAllObjects succeeded\n");
    } else {
        PKM_Error( "PKM_FindAllObjects failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }
    crv = pFunctionList->C_Finalize(NULL);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Finalize succeeded\n");
    } else {
        PKM_Error( "C_Finalize failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        exit(1);
    }

    if (pSlotList) free(pSlotList);

    /* demostrate how an application can be in Hybrid mode */
    /* PKM_HybridMode shows how to switch between NONFIPS */
    /* mode to FIPS mode */

    PKM_LogIt("Testing Hybrid mode \n");

    PKM_HybridMode(pwd, sizeof(pwd));

    PKM_LogIt("unloading NSS PKCS # 11 softoken \n");
#ifdef _WIN32
    FreeLibrary(hModule);
#else 
    if (PR_UnloadLibrary(lib) != PR_SUCCESS) {
        PKM_Error("unable to unload softoken");
    }
#endif
    return 0;
}
void PKM_LogIt(const char *fmt, ...) {
    va_list args;
    va_start (args, fmt);

    if (MODE == FIPSMODE) {
        printf("FIPS MODE: ");
    } else if (MODE == NONFIPSMODE) {
        printf("NON FIPS MODE: ");
    } else if (MODE == HYBRIDMODE) {
        printf("Hybrid MODE: ");
    } else printf ("NO MODE: ");
    vprintf(fmt, args);
    va_end(args);
}

void PKM_Error(const char *fmt, ...) {
    va_list args;
    va_start (args, fmt);

    if (MODE == FIPSMODE) {
        fprintf(stderr, "FIPS MODE PKM_Error: ");
    } else if (MODE == NONFIPSMODE) {
        fprintf(stderr, "NON FIPS MODE PKM_Error: ");
    } else if (MODE == HYBRIDMODE) {
        fprintf(stderr, "Hybrid MODE PKM_Error: ");
    } else fprintf(stderr, "NOMODE PKM_Error: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
}
CK_SLOT_ID *PKM_GetSlotList(CK_FUNCTION_LIST_PTR pFunctionList,
                            CK_ULONG slotID) {
    CK_RV crv = CKR_OK;
    CK_SLOT_ID *pSlotList = NULL;
    CK_ULONG slotCount;

    /* Get slot list */
    crv = pFunctionList->C_GetSlotList(CK_FALSE /* all slots */,
                                       NULL, &slotCount);
    if (crv != CKR_OK) {
        PKM_Error( "C_GetSlotList failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return NULL;
    }
    PKM_LogIt("C_GetSlotList reported there are %lu slots\n", slotCount);
    pSlotList = (CK_SLOT_ID *)malloc(slotCount * sizeof(CK_SLOT_ID));
    if (!pSlotList) {
        PKM_Error( "failed to allocate slot list\n");
        return NULL;
    }
    crv = pFunctionList->C_GetSlotList(CK_FALSE /* all slots */,
                                       pSlotList, &slotCount);
    if (crv != CKR_OK) {
        PKM_Error( "C_GetSlotList failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        if (pSlotList) free(pSlotList);
        return NULL;
    }
    return pSlotList;
}

CK_RV PKM_InitPWforDB(CK_FUNCTION_LIST_PTR pFunctionList,
                      CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                      CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen)  {
    CK_RV crv = CKR_OK;
    CK_SESSION_HANDLE hSession;
    CK_UTF8CHAR testPin[] = {"0Mozilla"};
    CK_UTF8CHAR weakPin[] = {"mozilla"};

    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_RW_SESSION | CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Login(hSession, CKU_SO, NULL, 0);
    if (crv != CKR_OK) {
        PKM_Error( "C_Login failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    if (MODE == FIPSMODE) {
        crv = pFunctionList->C_InitPIN(hSession, weakPin, sizeof(weakPin));
        if (crv == CKR_OK) {
            PKM_Error( "C_InitPIN with a weak password succeeded\n");
            return crv;
        } else {
            PKM_LogIt("C_InitPIN with a weak password failed with "
                      "0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        }
    }
    crv = pFunctionList->C_InitPIN(hSession, testPin, sizeof(testPin));
    if (crv == CKR_OK) {
        PKM_LogIt("C_InitPIN succeeded\n");
    } else {
        PKM_Error( "C_InitPIN failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Logout(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }


    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_RW_SESSION | CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_Login(hSession, CKU_USER, testPin,
                                 sizeof(testPin));
    if (crv != CKR_OK) {
        PKM_Error( "C_Login failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    if (MODE == FIPSMODE) {
        crv = pFunctionList->C_SetPIN(
                                     hSession, testPin, sizeof(testPin),
                                     weakPin, sizeof(weakPin));
        if (crv == CKR_OK) {
            PKM_Error( "C_SetPIN with a weak password succeeded\n");
            return crv;
        } else {
            PKM_LogIt("C_SetPIN with a weak password failed with "
                      "0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        }
    }
    crv = pFunctionList->C_SetPIN(
                                 hSession, testPin, sizeof(testPin),
                                 pwd, pwdLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_CSetPin failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Logout(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    return crv;
}

CK_RV PKM_ShowInfo(CK_FUNCTION_LIST_PTR pFunctionList, CK_ULONG slotID) {
    CK_RV crv = CKR_OK;
    CK_INFO info;
    CK_SLOT_ID *pSlotList = NULL;
    unsigned i;

    CK_SLOT_INFO slotInfo;
    CK_TOKEN_INFO tokenInfo;
    CK_FLAGS bitflag;


    crv = pFunctionList->C_GetInfo(&info);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GetInfo succeeded\n");
    } else {
        PKM_Error( "C_GetInfo failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    PKM_LogIt("General information about the PKCS #11 library:\n");
    PKM_LogIt("    PKCS #11 version: %d.%d\n",
              (int)info.cryptokiVersion.major,
              (int)info.cryptokiVersion.minor);
    PKM_LogIt("    manufacturer ID: %.32s\n", info.manufacturerID);
    PKM_LogIt("    flags: 0x%08lX\n", info.flags);
    PKM_LogIt("    library description: %.32s\n", info.libraryDescription);
    PKM_LogIt("    library version: %d.%d\n",
              (int)info.libraryVersion.major, (int)info.libraryVersion.minor);
    PKM_LogIt("\n");

    /* Get slot list */
    pSlotList = PKM_GetSlotList(pFunctionList, slotID);
    if (pSlotList == NULL) {
        PKM_Error( "PKM_GetSlotList failed with \n");
        return crv;
    }
    crv = pFunctionList->C_GetSlotInfo(pSlotList[slotID], &slotInfo);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GetSlotInfo succeeded\n");
    } else {
        PKM_Error( "C_GetSlotInfo failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    PKM_LogIt("Information about slot %lu:\n", pSlotList[slotID]);
    PKM_LogIt("    slot description: %.64s\n", slotInfo.slotDescription);
    PKM_LogIt("    slot manufacturer ID: %.32s\n", slotInfo.manufacturerID);
    PKM_LogIt("    flags: 0x%08lX\n", slotInfo.flags);
    bitflag = 1;
    for (i = 0; i < sizeof(slotFlagName)/sizeof(slotFlagName[0]); i++) {
        if (slotInfo.flags & bitflag) {
            PKM_LogIt("           %s\n", slotFlagName[i]);
        }
        bitflag <<= 1;
    }
    PKM_LogIt("    slot's hardware version number: %d.%d\n",
              (int)slotInfo.hardwareVersion.major,
              (int)slotInfo.hardwareVersion.minor);
    PKM_LogIt("    slot's firmware version number: %d.%d\n",
              (int)slotInfo.firmwareVersion.major,
              (int)slotInfo.firmwareVersion.minor);
    PKM_LogIt("\n");

    crv = pFunctionList->C_GetTokenInfo(pSlotList[slotID], &tokenInfo);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GetTokenInfo succeeded\n");
    } else {
        PKM_Error( "C_GetTokenInfo failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    PKM_LogIt("Information about the token in slot %lu:\n",
              pSlotList[slotID]);
    PKM_LogIt("    label: %.32s\n", tokenInfo.label);
    PKM_LogIt("    device manufacturer ID: %.32s\n",
              tokenInfo.manufacturerID);
    PKM_LogIt("    device model: %.16s\n", tokenInfo.model);
    PKM_LogIt("    device serial number: %.16s\n", tokenInfo.serialNumber);
    PKM_LogIt("    flags: 0x%08lX\n", tokenInfo.flags);
    bitflag = 1;
    for (i = 0; i < sizeof(tokenFlagName)/sizeof(tokenFlagName[0]); i++) {
        if (tokenInfo.flags & bitflag) {
            PKM_LogIt("           %s\n", tokenFlagName[i]);
        }
        bitflag <<= 1;
    }
    PKM_LogIt("    maximum session count: %lu\n",
              tokenInfo.ulMaxSessionCount);
    PKM_LogIt("    session count: %lu\n", tokenInfo.ulSessionCount);
    PKM_LogIt("    maximum read/write session count: %lu\n",
              tokenInfo.ulMaxRwSessionCount);
    PKM_LogIt("    read/write session count: %lu\n",
              tokenInfo.ulRwSessionCount);
    PKM_LogIt("    maximum PIN length: %lu\n", tokenInfo.ulMaxPinLen);
    PKM_LogIt("    minimum PIN length: %lu\n", tokenInfo.ulMinPinLen);
    PKM_LogIt("    total public memory: %lu\n",
              tokenInfo.ulTotalPublicMemory);
    PKM_LogIt("    free public memory: %lu\n",
              tokenInfo.ulFreePublicMemory);
    PKM_LogIt("    total private memory: %lu\n",
              tokenInfo.ulTotalPrivateMemory);
    PKM_LogIt("    free private memory: %lu\n",
              tokenInfo.ulFreePrivateMemory);
    PKM_LogIt("    hardware version number: %d.%d\n",
              (int)tokenInfo.hardwareVersion.major,
              (int)tokenInfo.hardwareVersion.minor);
    PKM_LogIt("    firmware version number: %d.%d\n",
              (int)tokenInfo.firmwareVersion.major,
              (int)tokenInfo.firmwareVersion.minor);
    if (tokenInfo.flags & CKF_CLOCK_ON_TOKEN) {
        PKM_LogIt("    current time: %.16s\n", tokenInfo.utcTime);
    }
    PKM_LogIt("\n PKM_ShowInfo done \n\n");
    if (pSlotList) free(pSlotList);
    return crv;
}

/* PKM_HybridMode                                                         */
/* The NSS cryptographic module has two modes of operation: FIPS Approved */
/* mode and NONFIPS Approved mode. The two modes of operation are         */
/* independent of each other -- they have their own copies of data        */
/* structures and they are even allowed to be active at the same time.    */
/* The module is FIPS 140-2 compliant only when the NONFIPS mode          */
/* is inactive.                                                           */
/* PKM_HybridMode demostrates how an application can switch between the   */
/* two modes: FIPS Approved mode and NONFIPS mode.                        */
CK_RV PKM_HybridMode(CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {

    CK_C_GetFunctionList pC_GetFunctionList;  /* NONFIPSMode */
    CK_FUNCTION_LIST_PTR pC_FunctionList;
    CK_SLOT_ID *pC_SlotList = NULL;
    CK_ULONG slotID_C = 1;

    CK_C_GetFunctionList pFC_GetFunctionList; /* FIPSMode */
    CK_FUNCTION_LIST_PTR pFC_FunctionList;
    CK_SLOT_ID *pFC_SlotList = NULL;
    CK_ULONG slotID_FC = 0;


    CK_RV crv = CKR_OK;
    CK_C_INITIALIZE_ARGS_NSS initArgs;
    CK_SESSION_HANDLE hSession;


    MODE = NONFIPSMODE;
#ifdef _WIN32
    /* NON FIPS mode  == C_GetFunctionList */
    pC_GetFunctionList = (CK_C_GetFunctionList)
                         GetProcAddress(hModule, "C_GetFunctionList");
    if (pC_GetFunctionList == NULL) {
        PKM_Error( "cannot load %s\n", LIB_NAME);
        return crv;
    }
#else
        pC_GetFunctionList = (CK_C_GetFunctionList) PR_FindFunctionSymbol(lib,
        "C_GetFunctionList");
        assert(pC_GetFunctionList != NULL);
#endif
    PKM_LogIt("loading C_GetFunctionList for Non FIPS Mode; slotID %d \n",
              slotID_C);
    crv = (*pC_GetFunctionList)(&pC_FunctionList);
    assert(crv == CKR_OK);

    initArgs.CreateMutex = NULL;
    initArgs.DestroyMutex = NULL;
    initArgs.LockMutex = NULL;
    initArgs.UnlockMutex = NULL;
    initArgs.flags = CKF_OS_LOCKING_OK;
    initArgs.LibraryParameters = (CK_CHAR_PTR *)
    "configdir='.' certPrefix='' keyPrefix='' secmod='secmod.db' flags= ";
    initArgs.pReserved = NULL;

    /* invoke C_Initialize as pC_FunctionList->C_Initialize */
    crv = pC_FunctionList->C_Initialize(&initArgs);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Initialize succeeded\n");
    } else {
        PKM_Error( "C_Initialize failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    pC_SlotList = PKM_GetSlotList(pC_FunctionList, slotID_C);
    if (pC_SlotList == NULL) {
        PKM_Error( "PKM_GetSlotList failed with \n");
        return crv;
    }
    crv = pC_FunctionList->C_OpenSession(pC_SlotList[slotID_C],
                                         CKF_SERIAL_SESSION,
                                         NULL, NULL, &hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("NONFIPS C_OpenSession succeeded\n");
    } else {
        PKM_Error( "C_OpenSession failed for NONFIPS token "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pC_FunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("able to login in NONFIPS token\n");
    } else {
        PKM_Error( "Unable to login in to NONFIPS token "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pC_FunctionList->C_Logout(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_ShowInfo(pC_FunctionList, slotID_C);
    MODE = HYBRIDMODE;

    /* Now load the FIPS token */
    /* FIPS mode == FC_GetFunctionList */
    pFC_GetFunctionList = NULL; 
#ifdef _WIN32
    pFC_GetFunctionList = (CK_C_GetFunctionList)
                          GetProcAddress(hModule, "FC_GetFunctionList");
#else
     pFC_GetFunctionList = (CK_C_GetFunctionList) PR_FindFunctionSymbol(lib,
        "FC_GetFunctionList");
        assert(pFC_GetFunctionList != NULL);
#endif

    PKM_LogIt("loading FC_GetFunctionList for FIPS Mode; slotID %d \n",
              slotID_FC);
    PKM_LogIt("pFC_FunctionList->C_Foo == pFC_FunctionList->FC_Foo\n");
    if (pFC_GetFunctionList == NULL) {
        PKM_Error( "unable to load pFC_GetFunctionList\n");
        return crv;
    }

    crv = (*pFC_GetFunctionList)(&pFC_FunctionList);
    assert(crv == CKR_OK);

    /* invoke FC_Initialize as pFunctionList->C_Initialize */
    crv = pFC_FunctionList->C_Initialize(&initArgs);
    if (crv == CKR_OK) {
        PKM_LogIt("FC_Initialize succeeded\n");
    } else {
        PKM_Error( "FC_Initialize failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    PKM_ShowInfo(pFC_FunctionList, slotID_FC);

    pFC_SlotList = PKM_GetSlotList(pFC_FunctionList, slotID_FC);
    if (pFC_SlotList == NULL) {
        PKM_Error( "PKM_GetSlotList failed with \n");
        return crv;
    }

    crv = pC_FunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv != CKR_OK) {
        PKM_LogIt("NONFIPS token cannot log in when FIPS token is loaded\n");
    } else {
        PKM_Error("Able to login in to NONFIPS token\n");
        return crv;
    }
    crv = pC_FunctionList->C_CloseSession(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("NONFIPS pC_CloseSession succeeded\n");
    } else {
        PKM_Error( "pC_CloseSession failed for NONFIPS token "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt("The module is FIPS 140-2 compliant\n"
              "only when the NONFIPS Approved mode is inactive by \n"
              "calling C_Finalize on the NONFIPS token.\n");


    /* to go in FIPSMODE you must Finalize the NONFIPS mode pointer */
    crv = pC_FunctionList->C_Finalize(NULL);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Finalize of NONFIPS Token succeeded\n");
        MODE = FIPSMODE;
    } else {
        PKM_Error( "C_Finalize of NONFIPS Token failed with "
                   "0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt("In FIPS mode!");

    /* Could do something */

    crv = pFC_FunctionList->C_Finalize(NULL);
    if (crv == CKR_OK) {
        PKM_LogIt("Exiting FIPSMODE by caling FC_Finalize.\n");
        MODE = NOMODE;
    } else {
        PKM_Error( "FC_Finalize failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    if (pC_SlotList) free(pC_SlotList);
    if (pFC_SlotList) free(pFC_SlotList);

    PKM_LogIt("PKM_HybridMode test Completed\n\n");
    return crv;
}

CK_RV PKM_Mechanism(CK_FUNCTION_LIST_PTR pFunctionList,
                    CK_SLOT_ID * pSlotList, CK_ULONG slotID) {

    CK_RV crv = CKR_OK;
    CK_MECHANISM_TYPE *pMechanismList;
    CK_ULONG mechanismCount;
    CK_ULONG i;

    /* Get the mechanism list */
    crv = pFunctionList->C_GetMechanismList(pSlotList[slotID],
                                            NULL, &mechanismCount);
    if (crv != CKR_OK) {
        PKM_Error( "C_GetMechanismList failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    PKM_LogIt("C_GetMechanismList reported there are %lu mechanisms\n",
              mechanismCount);
    pMechanismList = (CK_MECHANISM_TYPE *)
                     malloc(mechanismCount * sizeof(CK_MECHANISM_TYPE));
    if (!pMechanismList) {
        PKM_Error( "failed to allocate mechanism list\n");
        return crv;
    }
    crv = pFunctionList->C_GetMechanismList(pSlotList[slotID],
                                            pMechanismList, &mechanismCount);
    if (crv != CKR_OK) {
        PKM_Error( "C_GetMechanismList failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    PKM_LogIt("C_GetMechanismList returned the mechanism types:\n");
    for (i = 0; i < mechanismCount; i++) {
        printf("    0x%08lX", pMechanismList[i]);
        if ((i != 0) && ((i % 4) == 0 )) printf("\n");
    }
    printf("\n");

    for ( i = 0; i < mechanismCount; i++ ) {
        CK_MECHANISM_INFO minfo;

        memset(&minfo, 0, sizeof(CK_MECHANISM_INFO));
        crv = pFunctionList->C_GetMechanismInfo(pSlotList[slotID],
                                                pMechanismList[i], &minfo);
        if ( CKR_OK != crv ) {
            PKM_Error( "C_GetMechanismInfo(%lu, %lu) returned 0x%08X, %-26s\n",
                       pSlotList[slotID], pMechanismList[i], crv, 
                       PKM_CK_RVtoStr(crv));
            return crv;
        }

        PKM_LogIt( "    [%lu]: CK_MECHANISM_TYPE = %lu\n", (i+1),
                   pMechanismList[i]);
        PKM_LogIt( "    ulMinKeySize = %lu\n", minfo.ulMinKeySize);
        PKM_LogIt( "    ulMaxKeySize = %lu\n", minfo.ulMaxKeySize);
        PKM_LogIt( "    flags = 0x%08x\n", minfo.flags);
        PKM_LogIt( "        -> HW = %s\n", minfo.flags & CKF_HW ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> ENCRYPT = %s\n", minfo.flags & CKF_ENCRYPT ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> DECRYPT = %s\n", minfo.flags & CKF_DECRYPT ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> DIGEST = %s\n", minfo.flags & CKF_DIGEST ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> SIGN = %s\n", minfo.flags & CKF_SIGN ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> SIGN_RECOVER = %s\n", minfo.flags &
                   CKF_SIGN_RECOVER ? "TRUE" : "FALSE");
        PKM_LogIt( "        -> VERIFY = %s\n", minfo.flags & CKF_VERIFY ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> VERIFY_RECOVER = %s\n",
                   minfo.flags & CKF_VERIFY_RECOVER ? "TRUE" : "FALSE");
        PKM_LogIt( "        -> GENERATE = %s\n", minfo.flags & CKF_GENERATE ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> GENERATE_KEY_PAIR = %s\n",
                   minfo.flags & CKF_GENERATE_KEY_PAIR ? "TRUE" : "FALSE");
        PKM_LogIt( "        -> WRAP = %s\n", minfo.flags & CKF_WRAP ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> UNWRAP = %s\n", minfo.flags & CKF_UNWRAP ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> DERIVE = %s\n", minfo.flags & CKF_DERIVE ?
                   "TRUE" : "FALSE");
        PKM_LogIt( "        -> EXTENSION = %s\n", minfo.flags & CKF_EXTENSION ?
                   "TRUE" : "FALSE");

        PKM_LogIt( "\n");
    }


    return crv;

}

CK_RV PKM_RNG(CK_FUNCTION_LIST_PTR pFunctionList, CK_SLOT_ID * pSlotList,
              CK_ULONG slotID) {
    CK_SESSION_HANDLE hSession;
    CK_RV crv = CKR_OK;
    CK_BYTE randomData[16];
    CK_BYTE seed[] = {0x01, 0x03, 0x35, 0x55, 0xFF};


    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_GenerateRandom(hSession,
                                          randomData, sizeof randomData);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateRandom without login succeeded\n");
    } else {
        PKM_Error( "C_GenerateRandom without login failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_SeedRandom(hSession, seed, sizeof(seed));
    if (crv == CKR_OK) {
        PKM_LogIt("C_SeedRandom without login succeeded\n");
    } else {
        PKM_Error( "C_SeedRandom without login failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_GenerateRandom(hSession,
                                          randomData, sizeof randomData);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateRandom without login succeeded\n");
    } else {
        PKM_Error( "C_GenerateRandom without login failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;

}

CK_RV PKM_SessionLogin(CK_FUNCTION_LIST_PTR pFunctionList,
                       CK_SLOT_ID *pSlotList, CK_ULONG slotID,
                       CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_SESSION_HANDLE hSession;
    CK_RV crv = CKR_OK;

    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_Login(hSession, CKU_USER, (unsigned char *) "netscape", 8);
    if (crv == CKR_OK) {
        PKM_Error( "C_Login with wrong password succeeded\n");
        return crv;
    } else {
        PKM_LogIt("C_Login with wrong password failed with 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
    }
    crv = pFunctionList->C_Login(hSession, CKU_USER, (unsigned char *) "red hat", 7);
    if (crv == CKR_OK) {
        PKM_Error( "C_Login with wrong password succeeded\n");
        return crv;
    } else {
        PKM_LogIt("C_Login with wrong password failed with 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
    }
    crv = pFunctionList->C_Login(hSession, CKU_USER, (unsigned char *) "sun", 3);
    if (crv == CKR_OK) {
        PKM_Error( "C_Login with wrong password succeeded\n");
        return crv;

    } else {
        PKM_LogIt("C_Login with wrong password failed with 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
    }
    crv = pFunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_Logout(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;

}

/*
* PKM_LegacyFunctions
*
* Legacyfunctions exist only for backwards compatibility.
* C_GetFunctionStatus and C_CancelFunction functions were
* meant for managing parallel execution of cryptographic functions.
*
* C_GetFunctionStatus is a legacy function which should simply return
* the value CKR_FUNCTION_NOT_PARALLEL.
*
* C_CancelFunction is a legacy function which should simply return the
* value CKR_FUNCTION_NOT_PARALLEL.
*
*/
CK_RV PKM_LegacyFunctions(CK_FUNCTION_LIST_PTR pFunctionList,
                          CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                          CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_SESSION_HANDLE hSession;
    CK_RV crv = CKR_OK;

    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_GetFunctionStatus(hSession);
    if (crv == CKR_FUNCTION_NOT_PARALLEL) {
        PKM_LogIt("C_GetFunctionStatus correctly"
                  "returned CKR_FUNCTION_NOT_PARALLEL \n");
    } else {
        PKM_Error( "C_GetFunctionStatus failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CancelFunction(hSession);
    if (crv == CKR_FUNCTION_NOT_PARALLEL) {
        PKM_LogIt("C_CancelFunction correctly "
                  "returned CKR_FUNCTION_NOT_PARALLEL \n");
    } else {
        PKM_Error( "C_CancelFunction failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_Logout(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;

}

/*
*  PKM_DualFuncDigest - demostrates the Dual-function
*  cryptograpic functions:
*
*   C_DigestEncryptUpdate - multi-part Digest and Encrypt
*   C_DecryptDigestUpdate - multi-part Decrypt and Digest
*
*
*/

CK_RV PKM_DualFuncDigest(CK_FUNCTION_LIST_PTR pFunctionList,
                         CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                         CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_SESSION_HANDLE hSession;
    CK_RV crv = CKR_OK;
    CK_OBJECT_HANDLE hKey;

    CK_BYTE eDigest[16];
    CK_BYTE dDigest[16];
    CK_ULONG ulDigestLen;
    CK_MECHANISM mechanism = {
        CKM_AES_KEY_GEN, NULL, 0
    };
    CK_MECHANISM aesEcbMech = {
        CKM_AES_ECB, NULL, 0
    };
    CK_OBJECT_CLASS class = CKO_SECRET_KEY;
    CK_KEY_TYPE keyType = CKK_AES;
    CK_UTF8CHAR label[] = "An AES secret key object";
    CK_ULONG valueLen = 16;
    CK_ATTRIBUTE template[] = {
        {CKA_CLASS, &class, sizeof(class)},
        {CKA_KEY_TYPE, &keyType, sizeof(keyType)},
        {CKA_TOKEN, &true, sizeof(true)},
        {CKA_LABEL, label, sizeof(label)-1},
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_DECRYPT, &true, sizeof(true)},
        {CKA_UNWRAP, &true, sizeof(true)},
        {CKA_VALUE_LEN, &valueLen, sizeof(valueLen)}
    };
    CK_BYTE KEY[] = {
        0x80, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    CK_BYTE IV[] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    CK_BYTE PLAINTEXT[] = {"Firefox  Rules!"};
    CK_BYTE CIPHERTEXT[] = {
        0xc6, 0x5c, 0xf8, 0x86,
        0x5f, 0xf8, 0x2a, 0xe5,
        0xb2, 0xb2, 0x3a, 0x5b,
        0x1e, 0x87, 0xfe, 0x7a
    };
    CK_BYTE ciphertext[16];
    CK_ULONG ciphertextLen, lastLen;
    CK_BYTE plaintext[16];
    CK_ULONG plaintextLen;
    CK_BYTE wrappedKey[16];
    CK_ULONG wrappedKeyLen;
    CK_OBJECT_HANDLE hTestKey;

    CK_MECHANISM cryptMech = {
        CKM_AES_CBC, IV, sizeof(IV)
    };
    CK_MECHANISM digestMech = {
        CKM_SHA512, NULL, 0
    };

    unsigned int i;

    memset(eDigest, 0, sizeof(eDigest));
    memset(dDigest, 0, sizeof(dDigest));
    memset(ciphertext, 0, sizeof(ciphertext));

    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_RW_SESSION | CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    if (MODE == FIPSMODE) {
        crv = pFunctionList->C_GenerateKey(hSession, &mechanism,
                                          template,
                                          sizeof(template)/sizeof(template[0]),
                                          &hKey);
        if (crv == CKR_OK) {
            PKM_Error("C_GenerateKey succeeded when not logged in.\n");
            return CKR_GENERAL_ERROR;
        } else {
            PKM_LogIt("C_GenerateKey failed with  0x%08X, %-26s\n"
                      "since not logged in\n", crv, PKM_CK_RVtoStr(crv));
        }
    }
    crv = pFunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_GenerateKey(hSession, &mechanism,
                                       template,
                                       sizeof(template)/sizeof(template[0]),
                                       &hKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKey succeeded\n");
    } else {
        PKM_Error( "C_GenerateKey failed with 0x%08X, %-26s\n", crv,
                    PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_EncryptInit(hSession, &aesEcbMech, hKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    wrappedKeyLen = sizeof(wrappedKey);
    crv = pFunctionList->C_Encrypt(hSession, KEY, sizeof(KEY),
                                   wrappedKey, &wrappedKeyLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Encrypt failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    if (wrappedKeyLen != sizeof(wrappedKey)) {
        PKM_Error( "wrappedKeyLen is %lu\n", wrappedKeyLen);
        return crv;
    }
    /* Import an encrypted key */
    crv = pFunctionList->C_UnwrapKey(hSession, &aesEcbMech, hKey,
                                     wrappedKey, wrappedKeyLen,
                                     template,
                                     sizeof(template)/sizeof(template[0]),
                                     &hTestKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_UnwraPKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }



    /*
     * First init the Digest and Ecrypt operations
     */
    crv = pFunctionList->C_EncryptInit(hSession, &cryptMech, hTestKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_DigestInit(hSession, &digestMech);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    ciphertextLen = sizeof(ciphertext);
    crv = pFunctionList->C_DigestEncryptUpdate(hSession, PLAINTEXT,
                                               sizeof(PLAINTEXT),
                                               ciphertext, &ciphertextLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestEncryptUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    ulDigestLen = sizeof(eDigest);
    crv = pFunctionList->C_DigestFinal(hSession, eDigest, &ulDigestLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }


    /* get the last piece of ciphertext (length should be 0 */
    lastLen = sizeof(ciphertext) - ciphertextLen;
    crv = pFunctionList->C_EncryptFinal(hSession,
                                      (CK_BYTE_PTR)&ciphertext[ciphertextLen],
                                      &lastLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    printf("ciphertext = ");
    for (i = 0; i < ciphertextLen; i++) {
        printf("%02x", (unsigned)ciphertext[i]);
    }
    printf("\n");
    printf("eDigest = ");
    for (i = 0; i < ulDigestLen; i++) {
        printf("%02x", (unsigned)eDigest[i]);
    }
    printf("\n");

    /* AES CBC Decrypt the text */
    crv = pFunctionList->C_DecryptInit(hSession, &cryptMech, hTestKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_DigestInit(hSession, &digestMech);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    plaintextLen = sizeof(plaintext);
    crv = pFunctionList->C_DecryptDigestUpdate(hSession, ciphertext,
                                               sizeof(ciphertext),
                                               plaintext, &plaintextLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptDigestUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    lastLen = sizeof(plaintext) - plaintextLen;

    crv = pFunctionList->C_DecryptFinal(hSession,
                                        (CK_BYTE_PTR)&plaintext[plaintextLen],
                                        &lastLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }


    ulDigestLen = sizeof(dDigest);
    crv = pFunctionList->C_DigestFinal(hSession, dDigest, &ulDigestLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }


    if (plaintextLen != sizeof(plaintext)) {
        PKM_Error( "plaintextLen is %lu\n", plaintextLen);
        return crv;
    }
    printf("plaintext = ");
    for (i = 0; i < plaintextLen; i++) {
        printf("%02x", (unsigned)plaintext[i]);
    }
    printf("\n");
    printf("dDigest = ");
    for (i = 0; i < ulDigestLen; i++) {
        printf("%02x", (unsigned)dDigest[i]);
    }
    printf("\n");

    if (memcmp(eDigest, dDigest, ulDigestLen) == 0) {
        PKM_LogIt("Encrypted Digest equals Decrypted Digest\n");
    } else {
        PKM_Error( "Digests don't match\n");
    }

    if ((plaintextLen == sizeof(PLAINTEXT)) &&
        (memcmp(plaintext, PLAINTEXT, sizeof(PLAINTEXT)))  == 0) {
        PKM_LogIt("AES CBCVarKey128 decrypt test case passed\n");
    } else {
        PKM_Error( "AES CBCVarKey128 derypt test case failed\n");
    }
    crv = pFunctionList->C_Logout(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    return crv;

}

/*
* PKM_SecKeyCrypt - Symmetric key encrypt/decyprt
*
*/

CK_RV PKM_SecKeyCrypt(CK_FUNCTION_LIST_PTR pFunctionList, 
                      CK_SESSION_HANDLE hSession,
                       CK_OBJECT_HANDLE hSymKey, CK_MECHANISM *cryptMech,
                       CK_BYTE *pData, CK_ULONG dataLen) {
    CK_RV crv = CKR_OK;

    CK_BYTE cipher1[MAX_CIPHER_SZ];
    CK_BYTE cipher2[MAX_CIPHER_SZ];
    CK_BYTE data1[MAX_DATA_SZ];
    CK_BYTE data2[MAX_DATA_SZ];
    CK_ULONG cipher1Len =0, cipher2Len =0, lastLen =0;
    CK_ULONG data1Len =0, data2Len =0;

    memset(cipher1, 0, sizeof(cipher1));
    memset(cipher2, 0, sizeof(cipher2));
    memset(data1, 0, sizeof(data1));
    memset(data2, 0, sizeof(data2));


    /* C_Encrypt */
    crv = pFunctionList->C_EncryptInit(hSession, cryptMech, hSymKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    cipher1Len = sizeof(cipher1);
    crv = pFunctionList->C_Encrypt(hSession, pData, dataLen,
                                   cipher1, &cipher1Len);
    if (crv != CKR_OK) {
        PKM_Error( "C_Encrypt failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* C_EncryptUpdate */
    crv = pFunctionList->C_EncryptInit(hSession, cryptMech, hSymKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    cipher2Len = sizeof(cipher2);
    crv = pFunctionList->C_EncryptUpdate (hSession, pData,
                                          dataLen,
                                          cipher2, &cipher2Len);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    lastLen = sizeof(cipher2) - cipher2Len;

    crv = pFunctionList->C_EncryptFinal(hSession,
                                    (CK_BYTE_PTR)&cipher2[cipher2Len],
                                    &lastLen);
    cipher2Len = cipher2Len + lastLen;

    if ( (cipher1Len == cipher2Len) &&
         (memcmp(cipher1, cipher2, sizeof(cipher1Len)) == 0) ) {
        PKM_LogIt("encrypt test case passed\n");
    } else {
        PKM_Error( "encrypt test case failed\n");
        return CKR_GENERAL_ERROR;
    }

    /* C_Decrypt */
    crv = pFunctionList->C_DecryptInit(hSession, cryptMech, hSymKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    data1Len = sizeof(data1);
    crv = pFunctionList->C_Decrypt(hSession, cipher1, cipher1Len,
                                   data1, &data1Len);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    /* now use C_DecryptUpdate the text */
    crv = pFunctionList->C_DecryptInit(hSession, cryptMech, hSymKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    data2Len = sizeof(data2);
    crv = pFunctionList->C_DecryptUpdate(hSession, cipher2,
                                         cipher2Len,
                                         data2, &data2Len);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    lastLen = sizeof(data2) - data2Len;
    crv = pFunctionList->C_DecryptFinal(hSession,
                                        (CK_BYTE_PTR)&data2[data2Len],
                                        &lastLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    data2Len = data2Len + lastLen;


    /* Comparison of Decrypt data */

    if ( (data1Len == data2Len) && (dataLen == data1Len) &&
         (memcmp(data1, pData, dataLen) == 0) &&
         (memcmp(data2, pData, dataLen) == 0) ) {
        PKM_LogIt("decrypt test case passed\n");
    } else {
        PKM_Error( "derypt test case failed\n");
    }

    return crv;

}

CK_RV PKM_SecretKey(CK_FUNCTION_LIST_PTR pFunctionList,
                    CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                    CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_SESSION_HANDLE hSession;
    CK_RV crv = CKR_OK;
    CK_OBJECT_HANDLE hKey;
    CK_MECHANISM mechanism = {
        CKM_AES_KEY_GEN, NULL, 0
    };
    CK_OBJECT_CLASS class = CKO_SECRET_KEY;
    CK_KEY_TYPE keyType = CKK_AES;
    CK_UTF8CHAR label[] = "An AES secret key object";
    CK_ULONG valueLen = 16;
    CK_ATTRIBUTE template[] = {
        {CKA_CLASS, &class, sizeof(class)},
        {CKA_KEY_TYPE, &keyType, sizeof(keyType)},
        {CKA_LABEL, label, sizeof(label)-1},
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_UNWRAP, &true, sizeof(true)},
        {CKA_VALUE_LEN, &valueLen, sizeof(valueLen)}
    };
    CK_BYTE KEY[] = {
        0x80, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    CK_BYTE IV[] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    CK_BYTE PLAINTEXT[] = {"Firefox  Rules!"};
    CK_BYTE CIPHERTEXT[] = {
        0xc6, 0x5c, 0xf8, 0x86,
        0x5f, 0xf8, 0x2a, 0xe5,
        0xb2, 0xb2, 0x3a, 0x5b,
        0x1e, 0x87, 0xfe, 0x7a
    };
    CK_BYTE ciphertext[64];
    CK_BYTE ciphertext2[64];
    CK_ULONG ciphertextLen, ciphertext2Len, lastLen;
    CK_BYTE plaintext[32];
    CK_BYTE plaintext2[32];
    CK_ULONG plaintextLen, plaintext2Len;
    CK_BYTE wrappedKey[16];
    CK_ULONG wrappedKeyLen;
    CK_MECHANISM aesEcbMech = {
        CKM_AES_ECB, NULL, 0
    };
    CK_OBJECT_HANDLE hTestKey;
    CK_MECHANISM aesCbcMech = {
        CKM_AES_CBC, IV, sizeof(IV)
    };

    memset(ciphertext, 0, sizeof(ciphertext));
    memset(ciphertext2, 0, sizeof(ciphertext2));

    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    if (MODE == FIPSMODE) {
        crv = pFunctionList->C_GenerateKey(hSession, &mechanism,
                                          template,
                                          sizeof(template)/sizeof(template[0]),
                                          &hKey);
        if (crv == CKR_OK) {
            PKM_Error("C_GenerateKey succeeded when not logged in.\n");
            return CKR_GENERAL_ERROR;
        } else {
            PKM_LogIt("C_GenerateKey failed with 0x%08X, %-26s\n"
                      "since not logged in\n", crv, PKM_CK_RVtoStr(crv));
        }
    }
    crv = pFunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_GenerateKey(hSession, &mechanism,
                                       template,
                                       sizeof(template)/sizeof(template[0]),
                                       &hKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKey succeeded\n");
    } else {
        PKM_Error( "C_GenerateKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_EncryptInit(hSession, &aesEcbMech, hKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    wrappedKeyLen = sizeof(wrappedKey);
    crv = pFunctionList->C_Encrypt(hSession, KEY, sizeof(KEY),
                                   wrappedKey, &wrappedKeyLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Encrypt failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    if (wrappedKeyLen != sizeof(wrappedKey)) {
        PKM_Error( "wrappedKeyLen is %lu\n", wrappedKeyLen);
        return crv;
    }
    /* Import an encrypted key */
    crv = pFunctionList->C_UnwrapKey(hSession, &aesEcbMech, hKey,
                                     wrappedKey, wrappedKeyLen,
                                     template,
                                     sizeof(template)/sizeof(template[0]),
                                     &hTestKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_UnwraPKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    /* AES Encrypt the text */
    crv = pFunctionList->C_EncryptInit(hSession, &aesCbcMech, hTestKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    ciphertextLen = sizeof(ciphertext);
    crv = pFunctionList->C_Encrypt(hSession, PLAINTEXT, sizeof(PLAINTEXT),
                                   ciphertext, &ciphertextLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Encrypt failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    if ( (ciphertextLen == sizeof(CIPHERTEXT)) &&
       (memcmp(ciphertext, CIPHERTEXT, ciphertextLen) == 0)) {
        PKM_LogIt("AES CBCVarKey128 encrypt test case 1 passed\n");
    } else {
        PKM_Error( "AES CBCVarKey128 encrypt test case 1 failed\n");
        return crv;
    }

    /* now use EncryptUpdate the text */
    crv = pFunctionList->C_EncryptInit(hSession, &aesCbcMech, hTestKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    ciphertext2Len = sizeof(ciphertext2);
    crv = pFunctionList->C_EncryptUpdate (hSession, PLAINTEXT,
                                          sizeof(PLAINTEXT),
                                          ciphertext2, &ciphertext2Len);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    lastLen = sizeof(ciphertext2) - ciphertext2Len;

    crv = pFunctionList->C_EncryptFinal(hSession,
                                    (CK_BYTE_PTR)&ciphertext2[ciphertext2Len],
                                    &lastLen);
    ciphertext2Len = ciphertext2Len + lastLen;

    if ( (ciphertextLen == ciphertext2Len) &&
         (memcmp(ciphertext, ciphertext2, sizeof(CIPHERTEXT)) == 0) &&
         (memcmp(ciphertext2, CIPHERTEXT, sizeof(CIPHERTEXT)) == 0)) {
        PKM_LogIt("AES CBCVarKey128 encrypt test case 2 passed\n");
    } else {
        PKM_Error( "AES CBCVarKey128 encrypt test case 2 failed\n");
        return CKR_GENERAL_ERROR;
    }

    /* AES CBC Decrypt the text */
    crv = pFunctionList->C_DecryptInit(hSession, &aesCbcMech, hTestKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    plaintextLen = sizeof(plaintext);
    crv = pFunctionList->C_Decrypt(hSession, ciphertext, ciphertextLen,
                                   plaintext, &plaintextLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    if ((plaintextLen == sizeof(PLAINTEXT))
        && (memcmp(plaintext, PLAINTEXT, plaintextLen) == 0)) {
        PKM_LogIt("AES CBCVarKey128 decrypt test case 1 passed\n");
    } else {
        PKM_Error( "AES CBCVarKey128 derypt test case 1 failed\n");
    }
    /* now use DecryptUpdate the text */
    crv = pFunctionList->C_DecryptInit(hSession, &aesCbcMech, hTestKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    plaintext2Len = sizeof(plaintext2);
    crv = pFunctionList->C_DecryptUpdate(hSession, ciphertext2,
                                         ciphertext2Len,
                                         plaintext2, &plaintext2Len);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    lastLen = sizeof(plaintext2) - plaintext2Len;
    crv = pFunctionList->C_DecryptFinal(hSession,
                                     (CK_BYTE_PTR)&plaintext2[plaintext2Len],
                                     &lastLen);
    plaintext2Len = plaintext2Len + lastLen;

    if ( (plaintextLen == plaintext2Len) &&
         (memcmp(plaintext, plaintext2, plaintext2Len) == 0) &&
         (memcmp(plaintext2, PLAINTEXT, sizeof(PLAINTEXT)) == 0)) {
        PKM_LogIt("AES CBCVarKey128 decrypt test case 2 passed\n");
    } else {
        PKM_Error( "AES CBCVarKey128 decrypt test case 2 failed\n");
        return CKR_GENERAL_ERROR;
    }

    crv = pFunctionList->C_Logout(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }


    return crv;

}

CK_RV PKM_PubKeySign(CK_FUNCTION_LIST_PTR pFunctionList, 
                    CK_SESSION_HANDLE hRwSession,
                    CK_OBJECT_HANDLE hPubKey, CK_OBJECT_HANDLE hPrivKey,
                    CK_MECHANISM *signMech, CK_BYTE *pData, 
                    CK_ULONG pDataLen) {
    CK_RV crv = CKR_OK;
    CK_BYTE sig[MAX_SIG_SZ];
    CK_ULONG sigLen = 0 ;

    memset(sig, 0, sizeof(sig));

    /* C_Sign  */
    crv = pFunctionList->C_SignInit(hRwSession, signMech, hPrivKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_SignInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    sigLen = sizeof(sig);
    crv = pFunctionList->C_Sign(hRwSession, pData, pDataLen,
                                sig, &sigLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Sign failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* C_Verify the signature */
    crv = pFunctionList->C_VerifyInit(hRwSession, signMech, hPubKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Verify(hRwSession, pData, pDataLen,
                                  sig, sigLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Verify succeeded\n");
    } else {
        PKM_Error( "C_Verify failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

   /* Check that the mechanism is Multi-part */
    if (signMech->mechanism == CKM_DSA || 
        signMech->mechanism == CKM_RSA_PKCS) {
        return crv;
    }

    memset(sig, 0, sizeof(sig));
    /* SignUpdate  */
    crv = pFunctionList->C_SignInit(hRwSession, signMech, hPrivKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_SignInit failed with 0x%08lX %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_SignUpdate(hRwSession, pData, pDataLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Sign failed with 0x%08lX %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    sigLen = sizeof(sig);
    crv = pFunctionList->C_SignFinal(hRwSession, sig, &sigLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Sign failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* C_VerifyUpdate the signature  */
    crv = pFunctionList->C_VerifyInit(hRwSession, signMech,
                                      hPubKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyUpdate(hRwSession, pData, pDataLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyFinal(hRwSession, sig, sigLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_VerifyFinal succeeded\n");
    } else {
        PKM_Error( "C_VerifyFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt("PKM_PubKeySign succeeded\n");
    return crv;

}



CK_RV PKM_PublicKey(CK_FUNCTION_LIST_PTR pFunctionList, 
                    CK_SLOT_ID * pSlotList,
                    CK_ULONG slotID, CK_UTF8CHAR_PTR pwd, 
                    CK_ULONG pwdLen){
    CK_SESSION_HANDLE hSession;
    CK_SESSION_HANDLE hRwSession;

    CK_RV crv = CKR_OK;

    CK_MECHANISM dsaParamGenMech = {
        CKM_DSA_PARAMETER_GEN, NULL, 0
    };
    CK_ULONG primeBits = 1024;
    CK_ATTRIBUTE dsaParamGenTemplate[] = {
        {CKA_PRIME_BITS, &primeBits, sizeof(primeBits)}
    };
    CK_OBJECT_HANDLE hDsaParams;
    CK_BYTE DSA_P[128];
    CK_BYTE DSA_Q[20];
    CK_BYTE DSA_G[128];
    CK_MECHANISM dsaKeyPairGenMech = {
        CKM_DSA_KEY_PAIR_GEN, NULL, 0
    };
    CK_ATTRIBUTE dsaPubKeyTemplate[] = {
        {CKA_PRIME, DSA_P, sizeof(DSA_P)},
        {CKA_SUBPRIME, DSA_Q, sizeof(DSA_Q)},
        {CKA_BASE, DSA_G, sizeof(DSA_G)},
        {CKA_TOKEN, &true, sizeof(true)},
        {CKA_VERIFY, &true, sizeof(true)}
    };
    CK_ATTRIBUTE dsaPrivKeyTemplate[] = {
        {CKA_TOKEN, &true, sizeof(true)},
        {CKA_PRIVATE, &true, sizeof(true)},
        {CKA_SENSITIVE, &true, sizeof(true)},
        {CKA_SIGN, &true, sizeof(true)},
        {CKA_EXTRACTABLE, &true, sizeof(true)}
    };
    CK_OBJECT_HANDLE hDsaPubKey, hDsaPrivKey;
    /* From SHA1ShortMsg.req, Len = 136 */
    CK_BYTE MSG[] = {
        0xba, 0x33, 0x95, 0xfb,
        0x5a, 0xfa, 0x8e, 0x6a,
        0x43, 0xdf, 0x41, 0x6b,
        0x32, 0x7b, 0x74, 0xfa,
        0x44
    };
    CK_BYTE MD[] = {
        0xf7, 0x5d, 0x92, 0xa4,
        0xbb, 0x4d, 0xec, 0xc3,
        0x7c, 0x5c, 0x72, 0xfa,
        0x04, 0x75, 0x71, 0x0a,
        0x06, 0x75, 0x8c, 0x1d
    };
    CK_BYTE PLAINTEXT[] = {"Firefox  Rules!"};

    CK_BYTE sha1Digest[20];
    CK_ULONG sha1DigestLen;
    CK_BYTE dsaSig[40];
    CK_ULONG dsaSigLen;
    CK_MECHANISM sha1Mech = {
        CKM_SHA_1, NULL, 0
    };
    CK_MECHANISM dsaMech = {
        CKM_DSA, NULL, 0
    };
    CK_MECHANISM dsaWithSha1Mech = {
        CKM_DSA_SHA1, NULL, 0
    };
    unsigned int i;


    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_RW_SESSION | CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Generate DSA domain parameters PQG */
    crv = pFunctionList->C_GenerateKey(hSession, &dsaParamGenMech,
                                       dsaParamGenTemplate,
                                       sizeof(dsaParamGenTemplate)
                                       /sizeof(dsaParamGenTemplate[0]),
                                       &hDsaParams);
    if (crv == CKR_OK) {
        PKM_LogIt("DSA domain parameter generation succeeded\n");
    } else {
        PKM_Error( "DSA domain parameter generation failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_GetAttributeValue(hSession, hDsaParams,
                                             dsaPubKeyTemplate, 3);
    if (crv == CKR_OK) {
        PKM_LogIt("Getting DSA domain parameters succeeded\n");
    } else {
        PKM_Error( "Getting DSA domain parameters failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_DestroyObject(hSession, hDsaParams);
    if (crv == CKR_OK) {
        PKM_LogIt("Destroying DSA domain parameters succeeded\n");
    } else {
        PKM_Error( "Destroying DSA domain parameters failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Generate a persistent DSA key pair */
    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_RW_SESSION | CKF_SERIAL_SESSION,
                                       NULL, NULL, &hRwSession);
    if (crv == CKR_OK) {
        PKM_LogIt("Opening a read/write session succeeded\n");
    } else {
        PKM_Error( "Opening a read/write session failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_GenerateKeyPair(hRwSession, &dsaKeyPairGenMech,
                                           dsaPubKeyTemplate,
                                           NUM_ELEM(dsaPubKeyTemplate),
                                           dsaPrivKeyTemplate,
                                           NUM_ELEM(dsaPrivKeyTemplate),
                                           &hDsaPubKey, &hDsaPrivKey);
    if (crv == CKR_OK) {
        PKM_LogIt("DSA key pair generation succeeded\n");
    } else {
        PKM_Error( "DSA key pair generation failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Compute SHA-1 digest */
    crv = pFunctionList->C_DigestInit(hRwSession, &sha1Mech);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    sha1DigestLen = sizeof(sha1Digest);
    crv = pFunctionList->C_Digest(hRwSession, MSG, sizeof(MSG),
                                  sha1Digest, &sha1DigestLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Digest failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    if (sha1DigestLen != sizeof(sha1Digest)) {
        PKM_Error( "sha1DigestLen is %lu\n", sha1DigestLen);
        return crv;
    }
    for (i = 0; i < sha1DigestLen; i++) {
        printf("%02x", (unsigned)sha1Digest[i]);
    }
    printf("\n");
    if (memcmp(sha1Digest, MD, sizeof(MD)) == 0) {
        PKM_LogIt("SHA-1 SHA1ShortMsg test case Len = 136 passed\n");
    } else {
        PKM_Error( "SHA-1 SHA1ShortMsg test case Len = 136 failed\n");
    }

    crv = PKM_PubKeySign(pFunctionList, hRwSession,
                    hDsaPubKey, hDsaPrivKey,
                    &dsaMech, sha1Digest, sizeof(sha1Digest));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_PubKeySign CKM_DSA succeeded \n");
    } else {
        PKM_Error( "PKM_PubKeySign failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = PKM_PubKeySign(pFunctionList, hRwSession,
                    hDsaPubKey, hDsaPrivKey,
                    &dsaWithSha1Mech, PLAINTEXT, sizeof(PLAINTEXT));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_PubKeySign CKM_DSA_SHA1 succeeded \n");
    } else {
        PKM_Error( "PKM_PubKeySign failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Sign with DSA */
    crv = pFunctionList->C_SignInit(hRwSession, &dsaMech, hDsaPrivKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_SignInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    dsaSigLen = sizeof(dsaSig);
    crv = pFunctionList->C_Sign(hRwSession, sha1Digest, sha1DigestLen,
                                dsaSig, &dsaSigLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Sign failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Verify the DSA signature */
    crv = pFunctionList->C_VerifyInit(hRwSession, &dsaMech, hDsaPubKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Verify(hRwSession, sha1Digest, sha1DigestLen,
                                  dsaSig, dsaSigLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Verify succeeded\n");
    } else {
        PKM_Error( "C_Verify failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Verify the signature in a different way */
    crv = pFunctionList->C_VerifyInit(hRwSession, &dsaWithSha1Mech,
                                      hDsaPubKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyUpdate(hRwSession, MSG, 1);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyUpdate(hRwSession, MSG+1, sizeof(MSG)-1);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyFinal(hRwSession, dsaSig, dsaSigLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_VerifyFinal succeeded\n");
    } else {
        PKM_Error( "C_VerifyFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Verify the signature in a different way */
    crv = pFunctionList->C_VerifyInit(hRwSession, &dsaWithSha1Mech,
                                      hDsaPubKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", 
            crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyUpdate(hRwSession, MSG, 1);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyUpdate failed with 0x%08X, %-26s\n", 
            crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyUpdate(hRwSession, MSG+1, sizeof(MSG)-1);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyUpdate failed with 0x%08X, %-26s\n", 
            crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyFinal(hRwSession, dsaSig, dsaSigLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_VerifyFinal of multi update succeeded.\n");
    } else {
        PKM_Error("C_VerifyFinal of multi update failed with 0x%08X, %-26s\n", 
            crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    /* Now modify the data */
    MSG[0] += 1;
    /* Compute SHA-1 digest */
    crv = pFunctionList->C_DigestInit(hRwSession, &sha1Mech);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    sha1DigestLen = sizeof(sha1Digest);
    crv = pFunctionList->C_Digest(hRwSession, MSG, sizeof(MSG),
                                  sha1Digest, &sha1DigestLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Digest failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyInit(hRwSession, &dsaMech, hDsaPubKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Verify(hRwSession, sha1Digest, sha1DigestLen,
                                  dsaSig, dsaSigLen);
    if (crv != CKR_SIGNATURE_INVALID) {
        PKM_Error( "C_Verify of modified data succeeded\n");
        return crv;
    } else {
        PKM_LogIt("C_Verify of modified data correctly failed "
            " with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
    }

    crv = pFunctionList->C_Logout(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CloseSession(hRwSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;

}

CK_RV PKM_Hmac(CK_FUNCTION_LIST_PTR pFunctionList, CK_SESSION_HANDLE hSession,
                CK_OBJECT_HANDLE sKey, CK_MECHANISM *hmacMech, 
                CK_BYTE *pData, CK_ULONG pDataLen) {

    CK_RV crv = CKR_OK;

    CK_BYTE hmac1[HMAC_MAX_LENGTH];
    CK_ULONG hmac1Len = 0;
    CK_BYTE hmac2[HMAC_MAX_LENGTH];
    CK_ULONG hmac2Len = 0;

    memset(hmac1, 0, sizeof(hmac1));
    memset(hmac2, 0, sizeof(hmac2));

    crv = pFunctionList->C_SignInit(hSession, hmacMech, sKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_SignInit succeeded\n");
    } else {
        PKM_Error( "C_SignInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    hmac1Len = sizeof(hmac1);
    crv = pFunctionList->C_Sign(hSession, (CK_BYTE_PTR)pData,
                                pDataLen,
                                (CK_BYTE_PTR)hmac1, &hmac1Len);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Sign succeeded\n");
    } else {
        PKM_Error( "C_Sign failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_SignInit(hSession, hmacMech, sKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_SignInit succeeded\n");
    } else {
        PKM_Error( "C_SignInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_SignUpdate(hSession, (CK_BYTE_PTR)pData,
                                      pDataLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_SignUpdate succeeded\n");
    } else {
        PKM_Error( "C_SignUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    hmac2Len = sizeof(hmac2);
    crv = pFunctionList->C_SignFinal(hSession, (CK_BYTE_PTR)hmac2, &hmac2Len);
    if (crv == CKR_OK) {
        PKM_LogIt("C_SignFinal succeeded\n");
    } else {
        PKM_Error( "C_SignFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    if ((hmac1Len == hmac2Len) && (memcmp(hmac1, hmac2, hmac1Len) == 0) ) {
        PKM_LogIt("hmacs are equal!\n");
    } else {
        PKM_Error("hmacs are not equal!\n");
    }
    crv = pFunctionList->C_VerifyInit(hSession, hmacMech, sKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Verify(hSession, (CK_BYTE_PTR)pData,
                                  pDataLen,
                                  (CK_BYTE_PTR) hmac2, hmac2Len);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Verify of hmac succeeded\n");
    } else {
        PKM_Error( "C_Verify failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyInit(hSession, hmacMech, sKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyUpdate(hSession, (CK_BYTE_PTR)pData,
                                  pDataLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_VerifyUpdate of hmac succeeded\n");
    } else {
        PKM_Error( "C_VerifyUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyFinal(hSession, (CK_BYTE_PTR) hmac1, 
                                       hmac1Len);
    if (crv == CKR_OK) {
        PKM_LogIt("C_VerifyFinal of hmac succeeded\n");
    } else {
        PKM_Error( "C_VerifyFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    return crv;
}

CK_RV PKM_FindAllObjects(CK_FUNCTION_LIST_PTR pFunctionList,
                         CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                         CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_RV crv = CKR_OK;

    CK_SESSION_HANDLE h = (CK_SESSION_HANDLE)0;
    CK_SESSION_INFO sinfo;
    CK_ATTRIBUTE_PTR pTemplate;
    CK_ULONG tnObjects = 0;
    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &h);
    if ( CKR_OK != crv ) {
        PKM_Error("C_OpenSession(%lu, CKF_SERIAL_SESSION, , )"
                  "returned 0x%08X, %-26s\n", pSlotList[slotID], crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "    Opened a session: handle = 0x%08x\n", h);

    (void)memset(&sinfo, 0, sizeof(CK_SESSION_INFO));
    crv = pFunctionList->C_GetSessionInfo(h, &sinfo);
    if ( CKR_OK != crv ) {
        PKM_LogIt( "C_GetSessionInfo(%lu, ) returned 0x%08X, %-26s\n", h, crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "    SESSION INFO:\n");
    PKM_LogIt( "        slotID = %lu\n", sinfo.slotID);
    PKM_LogIt( "        state = %lu\n", sinfo.state);
    PKM_LogIt( "        flags = 0x%08x\n", sinfo.flags);
#ifdef CKF_EXCLUSIVE_SESSION
    PKM_LogIt( "            -> EXCLUSIVE SESSION = %s\n", sinfo.flags &
               CKF_EXCLUSIVE_SESSION ? "TRUE" : "FALSE");
#endif /* CKF_EXCLUSIVE_SESSION */
    PKM_LogIt( "            -> RW SESSION = %s\n", sinfo.flags &
               CKF_RW_SESSION ? "TRUE" : "FALSE");
    PKM_LogIt( "            -> SERIAL SESSION = %s\n", sinfo.flags &
               CKF_SERIAL_SESSION ? "TRUE" : "FALSE");
#ifdef CKF_INSERTION_CALLBACK
    PKM_LogIt( "            -> INSERTION CALLBACK = %s\n", sinfo.flags &
               CKF_INSERTION_CALLBACK ? "TRUE" : "FALSE");
#endif /* CKF_INSERTION_CALLBACK */
    PKM_LogIt( "        ulDeviceError = %lu\n", sinfo.ulDeviceError);
    PKM_LogIt( "\n");

    crv = pFunctionList->C_FindObjectsInit(h, NULL, 0);
    if ( CKR_OK != crv ) {
        PKM_LogIt( "C_FindObjectsInit(%lu, NULL, 0) returned "
                   "0x%08X, %-26s\n",
                   h, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    pTemplate = (CK_ATTRIBUTE_PTR)calloc(number_of_all_known_attribute_types,
                                         sizeof(CK_ATTRIBUTE));
    if ( (CK_ATTRIBUTE_PTR)NULL == pTemplate ) {
        PKM_Error(  "[memory allocation of %lu bytes failed]\n",
                    number_of_all_known_attribute_types *
                    sizeof(CK_ATTRIBUTE));
        return crv;
    }

    PKM_LogIt( "    All objects:\n");

    while (1) {
        CK_OBJECT_HANDLE o = (CK_OBJECT_HANDLE)0;
        CK_ULONG nObjects = 0;
        CK_ULONG k;
        CK_ULONG nAttributes = 0;
        CK_ATTRIBUTE_PTR pT2;
        CK_ULONG l;

        crv = pFunctionList->C_FindObjects(h, &o, 1, &nObjects);
        if ( CKR_OK != crv ) {
            PKM_Error(  "C_FindObjects(%lu, , 1, ) returned 0x%08X, %-26s\n", 
                        h, crv, PKM_CK_RVtoStr(crv));
            return crv;
        }

        if ( 0 == nObjects ) {
            PKM_LogIt( "\n");
            break;
        }

        tnObjects++;

        PKM_LogIt( "        OBJECT HANDLE %lu:\n", o);

        for ( k = 0; k < (CK_ULONG)number_of_all_known_attribute_types; k++ ) {
            pTemplate[k].type = all_known_attribute_types[k];
            pTemplate[k].pValue = (CK_VOID_PTR) NULL;
            pTemplate[k].ulValueLen = 0;
        }

        crv = pFunctionList->C_GetAttributeValue(h, o, pTemplate,
                                       number_of_all_known_attribute_types);
        switch ( crv ) {
        case CKR_OK:
        case CKR_ATTRIBUTE_SENSITIVE:
        case CKR_ATTRIBUTE_TYPE_INVALID:
        case CKR_BUFFER_TOO_SMALL:
            break;
        default:
            PKM_Error(  "C_GetAtributeValue(%lu, %lu, {all attribute types},"
                        "%lu) returned 0x%08X, %-26s\n",
                        h, o, number_of_all_known_attribute_types, crv, 
                        PKM_CK_RVtoStr(crv));
            return crv;
        }

        for ( k = 0; k < (CK_ULONG) number_of_all_known_attribute_types; k++) {
            if ( -1 != (CK_LONG)pTemplate[k].ulValueLen ) {
                nAttributes++;
            }
        }

        if ( 1 ) {
            PKM_LogIt( "            %lu attributes:\n", nAttributes);
            for ( k = 0; k < (CK_ULONG) number_of_all_known_attribute_types;
                k++ ) {
                if ( -1 != (CK_LONG)pTemplate[k].ulValueLen ) {
                    PKM_LogIt( "                0x%08x (len = %lu)\n",
                               pTemplate[k].type,
                               pTemplate[k].ulValueLen);
                }
            }
            PKM_LogIt( "\n");
        }

        pT2 = (CK_ATTRIBUTE_PTR)calloc(nAttributes, sizeof(CK_ATTRIBUTE));
        if ( (CK_ATTRIBUTE_PTR)NULL == pT2 ) {
            PKM_Error(  "[memory allocation of %lu bytes failed]\n",
                        nAttributes * sizeof(CK_ATTRIBUTE));
            return crv;
        }

        for ( l = 0, k = 0; k < (CK_ULONG) number_of_all_known_attribute_types;
            k++ ) {
            if ( -1 != (CK_LONG)pTemplate[k].ulValueLen ) {
                pT2[l].type = pTemplate[k].type;
                pT2[l].ulValueLen = pTemplate[k].ulValueLen;
                pT2[l].pValue = (CK_VOID_PTR)malloc(pT2[l].ulValueLen);
                if ( (CK_VOID_PTR)NULL == pT2[l].pValue ) {
                    PKM_Error(  "[memory allocation of %lu bytes failed]\n",
                                pT2[l].ulValueLen);
                    return crv;
                }
                l++;
            }
        }

        assert( l == nAttributes );

        crv = pFunctionList->C_GetAttributeValue(h, o, pT2, nAttributes);
        switch ( crv ) {
        case CKR_OK:
        case CKR_ATTRIBUTE_SENSITIVE:
        case CKR_ATTRIBUTE_TYPE_INVALID:
        case CKR_BUFFER_TOO_SMALL:
            break;
        default:
            PKM_Error(  "C_GetAtributeValue(%lu, %lu, {existant attribute"
                        " types}, %lu) returned 0x%08X, %-26s\n",
                        h, o, nAttributes, crv, PKM_CK_RVtoStr(crv));
            return crv;
        }

        for ( l = 0; l < nAttributes; l++ ) {
            PKM_LogIt( "            type = 0x%08x, len = %ld", pT2[l].type,
                       (CK_LONG)pT2[l].ulValueLen);
            if ( -1 == (CK_LONG)pT2[l].ulValueLen ) {
                ;
            } else {
                CK_ULONG m;

                if ( pT2[l].ulValueLen <= 8 ) {
                    PKM_LogIt( ", value = ");
                } else {
                    PKM_LogIt( ", value = \n                ");
                }

                for ( m = 0; (m < pT2[l].ulValueLen) && (m < 20); m++ ) {
                    PKM_LogIt( "%02x", (CK_ULONG)(0xff &
                              ((CK_CHAR_PTR)pT2[l].pValue)[m]));
                }

                PKM_LogIt( " ");

                for ( m = 0; (m < pT2[l].ulValueLen) && (m < 20); m++ ) {
                    CK_CHAR c = ((CK_CHAR_PTR)pT2[l].pValue)[m];
                    if ( (c < 0x20) || (c >= 0x7f) ) {
                        c = '.';
                    }
                    PKM_LogIt( "%c", c);
                }
            }

            PKM_LogIt( "\n");
        }

        PKM_LogIt( "\n");

        for ( l = 0; l < nAttributes; l++ ) {
            free(pT2[l].pValue);
        }
        free(pT2);
    } /* while(1) */

    crv = pFunctionList->C_FindObjectsFinal(h);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_FindObjectsFinal(%lu) returned 0x%08X, %-26s\n", h, crv, 
                    PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "    (%lu objects total)\n", tnObjects);

    crv = pFunctionList->C_CloseSession(h);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_CloseSession(%lu) returned 0x%08X, %-26s\n", h, crv, 
                    PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;
}
/* session to create, find, and delete a couple session objects */
CK_RV PKM_MultiObjectManagement (CK_FUNCTION_LIST_PTR pFunctionList,
                                 CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                                 CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {

    CK_RV crv = CKR_OK;

    CK_SESSION_HANDLE h = (CK_SESSION_HANDLE)0;
    CK_SESSION_HANDLE h2 = (CK_SESSION_HANDLE)0;
    CK_ATTRIBUTE one[7], two[7], three[7], delta[1], mask[1];
    CK_OBJECT_CLASS cko_data = CKO_DATA;
    char *key = "TEST PROGRAM";
    CK_ULONG key_len = 0; 
    CK_OBJECT_HANDLE hOneIn = (CK_OBJECT_HANDLE)0;
    CK_OBJECT_HANDLE hTwoIn = (CK_OBJECT_HANDLE)0;
    CK_OBJECT_HANDLE hThreeIn = (CK_OBJECT_HANDLE)0;
    CK_OBJECT_HANDLE hDeltaIn = (CK_OBJECT_HANDLE)0;
    CK_OBJECT_HANDLE found[10];
    CK_ULONG nFound;
    CK_ULONG   hDeltaLen, hThreeLen = 0;

    CK_TOKEN_INFO tinfo;
    key_len = sizeof(key);
    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_SERIAL_SESSION, NULL, NULL, &h);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_OpenSession(%lu, CKF_SERIAL_SESSION, , )"
                    "returned 0x%08X, %-26s\n", pSlotList[slotID], crv, 
                    PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Login(h, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }


    (void)memset(&tinfo, 0, sizeof(CK_TOKEN_INFO));
    crv = pFunctionList->C_GetTokenInfo(pSlotList[slotID], &tinfo);
    if ( CKR_OK != crv ) {
        PKM_Error("C_GetTokenInfo(%lu, ) returned 0x%08X, %-26s\n",
                  pSlotList[slotID], crv, PKM_CK_RVtoStr(crv));
        return crv;
    }


    PKM_LogIt( "    Opened a session: handle = 0x%08x\n", h);

    one[0].type = CKA_CLASS;
    one[0].pValue = &cko_data;
    one[0].ulValueLen = sizeof(CK_OBJECT_CLASS);
    one[1].type = CKA_TOKEN;
    one[1].pValue = &false;
    one[1].ulValueLen = sizeof(CK_BBOOL);
    one[2].type = CKA_PRIVATE;
    one[2].pValue = &false;
    one[2].ulValueLen = sizeof(CK_BBOOL);
    one[3].type = CKA_MODIFIABLE;
    one[3].pValue = &true;
    one[3].ulValueLen = sizeof(CK_BBOOL);
    one[4].type = CKA_LABEL;
    one[4].pValue = "Test data object one";
    one[4].ulValueLen = strlen(one[4].pValue);
    one[5].type = CKA_APPLICATION;
    one[5].pValue = key;
    one[5].ulValueLen = key_len;
    one[6].type = CKA_VALUE;
    one[6].pValue = "Object one";
    one[6].ulValueLen = strlen(one[6].pValue);

    two[0].type = CKA_CLASS;
    two[0].pValue = &cko_data;
    two[0].ulValueLen = sizeof(CK_OBJECT_CLASS);
    two[1].type = CKA_TOKEN;
    two[1].pValue = &false;
    two[1].ulValueLen = sizeof(CK_BBOOL);
    two[2].type = CKA_PRIVATE;
    two[2].pValue = &false;
    two[2].ulValueLen = sizeof(CK_BBOOL);
    two[3].type = CKA_MODIFIABLE;
    two[3].pValue = &true;
    two[3].ulValueLen = sizeof(CK_BBOOL);
    two[4].type = CKA_LABEL;
    two[4].pValue = "Test data object two";
    two[4].ulValueLen = strlen(two[4].pValue);
    two[5].type = CKA_APPLICATION;
    two[5].pValue = key;
    two[5].ulValueLen = key_len;
    two[6].type = CKA_VALUE;
    two[6].pValue = "Object two";
    two[6].ulValueLen = strlen(two[6].pValue);

    three[0].type = CKA_CLASS;
    three[0].pValue = &cko_data;
    three[0].ulValueLen = sizeof(CK_OBJECT_CLASS);
    three[1].type = CKA_TOKEN;
    three[1].pValue = &false;
    three[1].ulValueLen = sizeof(CK_BBOOL);
    three[2].type = CKA_PRIVATE;
    three[2].pValue = &false;
    three[2].ulValueLen = sizeof(CK_BBOOL);
    three[3].type = CKA_MODIFIABLE;
    three[3].pValue = &true;
    three[3].ulValueLen = sizeof(CK_BBOOL);
    three[4].type = CKA_LABEL;
    three[4].pValue = "Test data object three";
    three[4].ulValueLen = strlen(three[4].pValue);
    three[5].type = CKA_APPLICATION;
    three[5].pValue = key;
    three[5].ulValueLen = key_len;
    three[6].type = CKA_VALUE;
    three[6].pValue = "Object three";
    three[6].ulValueLen = strlen(three[6].pValue);

    crv = pFunctionList->C_CreateObject(h, one, 7, &hOneIn);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_CreateObject(%lu, one, 7, ) returned 0x%08X, %-26s\n",
                    h, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "    Created object one: handle = %lu\n", hOneIn);

    crv = pFunctionList->C_CreateObject(h, two, 7, &hTwoIn);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_CreateObject(%lu, two, 7, ) returned 0x%08X, %-26s\n",
                    h, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "    Created object two: handle = %lu\n", hTwoIn);

    crv = pFunctionList->C_CreateObject(h, three, 7, &hThreeIn);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_CreateObject(%lu, three, 7, ) returned 0x%08x\n",
                    h, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_GetObjectSize(h, hThreeIn, &hThreeLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GetObjectSize succeeded\n");
    } else {
        PKM_Error("C_GetObjectSize failed "
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "    Created object three: handle = %lu\n", hThreeIn);

    delta[0].type = CKA_VALUE;
    delta[0].pValue = "Copied object";
    delta[0].ulValueLen = strlen(delta[0].pValue);

    crv = pFunctionList->C_CopyObject(h, hThreeIn, delta, 1, &hDeltaIn);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_CopyObject(%lu, %lu, delta, 1, ) returned "
                    "0x%08X, %-26s\n",
                    h, hThreeIn, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_GetObjectSize(h, hDeltaIn, &hDeltaLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GetObjectSize succeeded\n");
    } else {
        PKM_Error("C_GetObjectSize failed "
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    if (hThreeLen == hDeltaLen) {
        PKM_LogIt("Copied object size same as orginal\n");
    } else {
        PKM_Error("Copied object different from original\n");
        return CKR_DEVICE_ERROR;
    }

    PKM_LogIt( "    Copied object three: new handle = %lu\n", hDeltaIn);

    mask[0].type = CKA_APPLICATION;
    mask[0].pValue = key;
    mask[0].ulValueLen = key_len;

    crv = pFunctionList->C_FindObjectsInit(h, mask, 1);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_FindObjectsInit(%lu, mask, 1) returned 0x%08X, %-26s\n",
                    h, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    (void)memset(&found, 0, sizeof(found));
    nFound = 0;
    crv = pFunctionList->C_FindObjects(h, found, 10, &nFound);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_FindObjects(%lu,, 10, ) returned 0x%08X, %-26s\n",
                    h, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    if ( 4 != nFound ) {
        PKM_Error(  "Found %lu objects, not 4.\n", nFound);
        return crv;
    }

    PKM_LogIt( "    Found 4 objects: %lu, %lu, %lu, %lu\n",
               found[0], found[1], found[2], found[3]);

    crv = pFunctionList->C_FindObjectsFinal(h);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_FindObjectsFinal(%lu) returned 0x%08X, %-26s\n", 
                    h, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_DestroyObject(h, hThreeIn);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_DestroyObject(%lu, %lu) returned 0x%08X, %-26s\n", h,
                    hThreeIn, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "    Destroyed object three (handle = %lu)\n", hThreeIn);

    delta[0].type = CKA_APPLICATION;
    delta[0].pValue = "Changed application";
    delta[0].ulValueLen = strlen(delta[0].pValue);

    crv = pFunctionList->C_SetAttributeValue(h, hTwoIn, delta, 1);
    if ( CKR_OK != crv ) {
        PKM_Error("C_SetAttributeValue(%lu, %lu, delta, 1) returned "
                  "0x%08X, %-26s\n",
                  h, hTwoIn, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "    Changed object two (handle = %lu).\n", hTwoIn);

    /* Can another session find these session objects? */

    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &h2);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_OpenSession(%lu, CKF_SERIAL_SESSION, , )"
                    " returned 0x%08X, %-26s\n", pSlotList[slotID], crv, 
                    PKM_CK_RVtoStr(crv));
        return crv;
    }
    PKM_LogIt( "    Opened a second session: handle = 0x%08x\n", h2);

    /* mask is still the same */

    crv = pFunctionList->C_FindObjectsInit(h2, mask, 1);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_FindObjectsInit(%lu, mask, 1) returned 0x%08X, %-26s\n",
                    h2, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    (void)memset(&found, 0, sizeof(found));
    nFound = 0;
    crv = pFunctionList->C_FindObjects(h2, found, 10, &nFound);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_FindObjects(%lu,, 10, ) returned 0x%08X, %-26s\n",
                    h2, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    if ( 2 != nFound ) {
        PKM_Error(  "Found %lu objects, not 2.\n", nFound);
        return crv;
    }

    PKM_LogIt( "    Found 2 objects: %lu, %lu\n",
               found[0], found[1]);

    crv = pFunctionList->C_FindObjectsFinal(h2);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_FindObjectsFinal(%lu) returned 0x%08X, %-26s\n", h2, crv, 
                    PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Logout(h);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_CloseAllSessions(pSlotList[slotID]);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_CloseAllSessions(%lu) returned 0x%08X, %-26s\n",
                    pSlotList[slotID], crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    PKM_LogIt( "\n");
    return crv;
}

CK_RV PKM_OperationalState(CK_FUNCTION_LIST_PTR pFunctionList,
                           CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                           CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_SESSION_HANDLE hSession;
    CK_RV crv = CKR_OK;
    CK_MECHANISM sMechGen = {
        CKM_AES_KEY_GEN, NULL, 0
    };
    CK_OBJECT_CLASS class = CKO_SECRET_KEY;
    CK_KEY_TYPE skeytype = CKK_AES;
    CK_UTF8CHAR label[] = "An AES secret key object";
    CK_ULONG valueLen = 16;
    CK_OBJECT_HANDLE sKey;
    CK_BYTE_PTR     pstate = NULL;
    CK_ULONG statelen, digestlen, plainlen, plainlen_1, plainlen_2, slen;

    CK_UTF8CHAR *plaintext = (CK_UTF8CHAR *)"Firefox rules.";
    CK_UTF8CHAR *plaintext_1 = (CK_UTF8CHAR *)"Thunderbird rules.";
    CK_UTF8CHAR *plaintext_2 = (CK_UTF8CHAR *)"Firefox and Thunderbird.";

    char    digest[MAX_DIGEST_LEN], digest_1[MAX_DIGEST_LEN];
    CK_ATTRIBUTE signtemplate[] = {
         {CKA_CLASS, &class, sizeof(class)},
         {CKA_KEY_TYPE, &skeytype, sizeof(skeytype)},
         {CKA_LABEL, label, sizeof(label)-1},
         {CKA_ENCRYPT, &true, sizeof(true)},
         {CKA_SIGN, &true, sizeof (true)},
         {CKA_UNWRAP, &true, sizeof(true)},
         {CKA_VALUE_LEN, &valueLen, sizeof(valueLen)}
     };
    char    sign[MAX_SIG_SZ];
    CK_MECHANISM signmech;
    CK_MECHANISM digestmech;

    signmech.mechanism = CKM_SHA_1_HMAC;
    signmech.pParameter = NULL;
    signmech.ulParameterLen = 0;
    digestmech.mechanism = CKM_SHA256;
    digestmech.pParameter = NULL;
    digestmech.ulParameterLen = 0;


    plainlen = strlen((char *)plaintext);
    plainlen_1 = strlen((char *)plaintext_1);
    plainlen_2 = strlen((char *)plaintext_2);
    digestlen = DIGESTLEN;


    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_GenerateKey(hSession, &sMechGen,
                                       signtemplate, sizeof(signtemplate)
                                       /sizeof(CK_ATTRIBUTE), &sKey);

    if (crv != CKR_OK) {
        PKM_Error("Could not create object for signing returned "
                  "0x%08X, %-26s\n",
                  crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_SignInit(hSession, &signmech, sKey);
    if (crv != CKR_OK) {
        PKM_Error("C_SignInit failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    slen = sizeof(sign);
    crv = pFunctionList->C_Sign(hSession, (CK_BYTE_PTR)plaintext, plainlen,
                                (CK_BYTE_PTR)sign, &slen);
    if (crv != CKR_OK) {
        PKM_Error("C_Sign failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_DestroyObject(hSession, sKey);
    if (crv != CKR_OK) {
        PKM_Error("C_DestroyObject failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    digestlen = DIGESTLEN;
    crv = pFunctionList->C_DigestInit(hSession, &digestmech);
    if (crv != CKR_OK) {
        PKM_Error("C_DigestInit failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_DigestUpdate(hSession, (CK_BYTE_PTR)plaintext,
                                        plainlen);
    if (crv != CKR_OK) {
        PKM_Error("C_DigestUpdate failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_GetOperationState(hSession, NULL, &statelen);
    if (crv != CKR_OK) {
        PKM_Error("C_GetOperationState failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    pstate = (CK_BYTE_PTR) malloc(statelen * sizeof (CK_BYTE_PTR));
    crv = pFunctionList->C_GetOperationState(hSession, pstate, &statelen);
    if (crv != CKR_OK) {
        PKM_Error("C_GetOperationState failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_DigestUpdate(hSession, (CK_BYTE_PTR)plaintext_1,
                                        plainlen_1);
    if (crv != CKR_OK) {
        PKM_Error("C_DigestUpdate failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_DigestUpdate(hSession, (CK_BYTE_PTR)plaintext_2,
                                        plainlen_2);
    if (crv != CKR_OK) {
        PKM_Error("C_DigestUpdate failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    /*
     *      This will override/negate the above 2 digest_update
     *      operations
     */
    crv = pFunctionList->C_SetOperationState(hSession, pstate, statelen,
                                             0, 0);
    if (crv != CKR_OK) {
        PKM_Error("C_SetOperationState failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_DigestFinal(hSession, (CK_BYTE_PTR)digest,
                                       &digestlen);
    if (crv != CKR_OK) {
        PKM_Error("C_DigestFinal failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }
    digestlen = DIGESTLEN;
    crv = pFunctionList->C_DigestInit(hSession, &digestmech);
    if (crv != CKR_OK) {
        PKM_Error("C_DigestInit failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Digest(hSession, (CK_BYTE_PTR)plaintext, plainlen,
                                  (CK_BYTE_PTR)digest_1, &digestlen);
    if (crv != CKR_OK) {
        PKM_Error("C_Digest failed returned 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }
    if (memcmp(digest, digest_1, digestlen) == 0) {
        PKM_LogIt("Digest and digest_1 are equal!\n");
    } else {
        PKM_Error("Digest and digest_1 are not equal!\n");
    }
    crv = pFunctionList->C_Logout(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_CloseSession(hSession);
    if ( CKR_OK != crv ) {
        PKM_Error(  "C_CloseSession(%lu) returned 0x%08X, %-26s\n", 
                    hSession, crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;
}


/*
* Recover Functions
*/
CK_RV PKM_RecoverFunctions(CK_FUNCTION_LIST_PTR pFunctionList,
                           CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                           CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_SESSION_HANDLE hRwSession;
    CK_RV crv = CKR_OK;
    CK_KEY_TYPE rsatype = CKK_RSA;
    CK_MECHANISM rsaKeyPairGenMech = {
        CKM_RSA_PKCS_KEY_PAIR_GEN, NULL, 0
    };
    CK_BYTE subject[] = {"RSA Private Key"};
    CK_ULONG modulusBits = 768;
    CK_BYTE publicExponent[] = { 3};
    CK_MECHANISM signRSAMech = {
        CKM_RSA_PKCS, NULL, 0
    };
    CK_BYTE id[] = {"RSA123"};
    CK_ATTRIBUTE publicKeyTemplate[] = {
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_VERIFY, &true, sizeof(true)},
        {CKA_WRAP, &true, sizeof(true)},
        {CKA_MODULUS_BITS, &modulusBits, sizeof(modulusBits)},
        {CKA_PUBLIC_EXPONENT, publicExponent, sizeof (publicExponent)}
    };
    CK_ATTRIBUTE privateKeyTemplate[] = {
        {CKA_TOKEN, &true, sizeof(true)},
        {CKA_PRIVATE, &true, sizeof(true)},
        {CKA_SUBJECT, subject, sizeof(subject)},
        {CKA_ID, id, sizeof(id)},
        {CKA_SENSITIVE, &true, sizeof(true)},
        {CKA_DECRYPT, &true, sizeof(true)},
        {CKA_SIGN, &true, sizeof(true)},
        {CKA_UNWRAP, &true, sizeof(true)}
    };
    CK_OBJECT_HANDLE publicKey, privateKey;

    CK_BYTE PLAINTEXT[] = {"Firefox  Rules!"};

    CK_BYTE rsaSig[128];
    CK_ULONG rsaSigLen = 128;
    CK_BYTE recover[128];
    CK_ULONG recoverLen = 128;

    CK_MECHANISM signMech = {
        CKM_RSA_PKCS, NULL, 0
    };



    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_RW_SESSION | CKF_SERIAL_SESSION,
                                       NULL, NULL, &hRwSession);
    if (crv == CKR_OK) {
        PKM_LogIt("Opening a read/write session succeeded\n");
    } else {
        PKM_Error( "Opening a read/write session failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Login(hRwSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error("C_Login with correct password failed "
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_GenerateKeyPair(hRwSession, &rsaKeyPairGenMech,
                                           publicKeyTemplate,
                                           (sizeof (publicKeyTemplate)
                                            / sizeof (CK_ATTRIBUTE)),
                                           privateKeyTemplate,
                                           (sizeof (privateKeyTemplate)
                                            / sizeof (CK_ATTRIBUTE)),
                                           &publicKey, &privateKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKeyPair created an RSA key pair. \n");
    } else {
        PKM_Error("C_GenerateKeyPair failed to create an RSA key pair.\n"
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }


    /* initializes a signature operation,
     *  where the data can be recovered from the signature
     */
    crv = pFunctionList->C_SignRecoverInit(hRwSession, &signMech,
                                           privateKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_SignRecoverInit succeeded. \n");
    } else {
        PKM_Error("C_SignRecoverInit failed.\n"
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* signs single-part data,
     * where the data can be recovered from the signature
     */
    crv = pFunctionList->C_SignRecover(hRwSession, (CK_BYTE_PTR)PLAINTEXT,
                                       sizeof(PLAINTEXT),
                                       (CK_BYTE_PTR)rsaSig, &rsaSigLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_SignRecover succeeded. \n");
    } else {
        PKM_Error("C_SignRecoverInit failed to create an RSA key pair.\n"
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }



    /*
     * initializes a verification operation
     *where the data is recovered from the signature
     */
    crv = pFunctionList->C_VerifyRecoverInit(hRwSession, &signMech,
                                             publicKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_VerifyRecoverInit succeeded. \n");
    } else {
        PKM_Error("C_VerifyRecoverInit failed.\n"
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /*
    * verifies a signature on single-part data,
    * where the data is recovered from the signature
    */
    crv = pFunctionList->C_VerifyRecover(hRwSession, (CK_BYTE_PTR)rsaSig,
                                         rsaSigLen,
                                         (CK_BYTE_PTR)recover, &recoverLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_VerifyRecover succeeded. \n");
    } else {
        PKM_Error("C_VerifyRecover failed.\n"
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    if ((recoverLen == sizeof(PLAINTEXT))
        && (memcmp(recover, PLAINTEXT, sizeof(PLAINTEXT)) == 0)) {
        PKM_LogIt("VerifyRecover test case passed\n");
    } else {
        PKM_Error( "VerifyRecover test case failed\n");
    }

    crv = pFunctionList->C_Logout(hRwSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error("C_Logout failed with 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CloseSession(hRwSession);
    if (crv != CKR_OK) {
        PKM_Error("C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;
}
/*
* wrapUnwrap
* Create and RSA public keypair
* Create a DES3 secretkey
* wrap the secretkey with the public key.
* unwrap the secretkey with the private key.
* check that the unwrapped key == orginal secret key
*/
CK_RV PKM_wrapUnwrap(CK_FUNCTION_LIST_PTR pFunctionList,
                     CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                     CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_SESSION_HANDLE hRwSession;
    CK_RV crv = CKR_OK;
    CK_KEY_TYPE rsatype = CKK_RSA;
    CK_MECHANISM rsaKeyPairGenMech = {
        CKM_RSA_PKCS_KEY_PAIR_GEN, NULL, 0
    };
    CK_BYTE subject[] = {"RSA Private Key"};
    CK_ULONG modulusBits = 768;
    CK_BYTE publicExponent[] = { 3};
    CK_BYTE id[] = {"RSA123"};
    CK_ATTRIBUTE publicKeyTemplate[] = {
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_VERIFY, &true, sizeof(true)},
        {CKA_WRAP, &true, sizeof(true)},
        {CKA_MODULUS_BITS, &modulusBits, sizeof(modulusBits)},
        {CKA_PUBLIC_EXPONENT, publicExponent, sizeof (publicExponent)}
    };
    CK_ATTRIBUTE privateKeyTemplate[] = {
        {CKA_TOKEN, &true, sizeof(true)},
        {CKA_PRIVATE, &true, sizeof(true)},
        {CKA_SUBJECT, subject, sizeof(subject)},
        {CKA_ID, id, sizeof(id)},
        {CKA_SENSITIVE, &true, sizeof(true)},
        {CKA_DECRYPT, &true, sizeof(true)},
        {CKA_SIGN, &true, sizeof(true)},
        {CKA_UNWRAP, &true, sizeof(true)}
    };
    CK_OBJECT_HANDLE hPublicKey, hPrivateKey;

    CK_OBJECT_CLASS class = CKO_SECRET_KEY;
    CK_KEY_TYPE keyType = CKK_DES3;
    CK_UTF8CHAR label[] = "An Triple DES secret key object";
    CK_ULONG valueLen = 56;
    CK_MECHANISM sKeyGenMechanism = {
        CKM_DES3_KEY_GEN, NULL, 0
    };
    CK_ATTRIBUTE sKeyTemplate[] = {
        {CKA_CLASS, &class, sizeof(class)},
        {CKA_KEY_TYPE, &keyType, sizeof(keyType)},
        {CKA_LABEL, label, sizeof(label)-1},
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_UNWRAP, &true, sizeof(true)},
        {CKA_VALUE_LEN, &valueLen, sizeof(valueLen)}
    };
    CK_OBJECT_HANDLE hSecretKey;

    CK_OBJECT_HANDLE hSecretKeyUnwrapped;

    CK_MECHANISM wrapMechanism = {
        CKM_RSA_PKCS, NULL, 0
    };
    CK_BYTE wrappedKey[128];
    CK_ULONG ulWrappedKeyLen;


    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_RW_SESSION | CKF_SERIAL_SESSION,
                                       NULL, NULL, &hRwSession);
    if (crv == CKR_OK) {
        PKM_LogIt("Opening a read/write session succeeded\n");
    } else {
        PKM_Error( "Opening a read/write session failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Login(hRwSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error("C_Login with correct password failed "
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_GenerateKeyPair(hRwSession, &rsaKeyPairGenMech,
                                           publicKeyTemplate,
                                           (sizeof (publicKeyTemplate)
                                            / sizeof (CK_ATTRIBUTE)),
                                           privateKeyTemplate,
                                           (sizeof (privateKeyTemplate)
                                            / sizeof (CK_ATTRIBUTE)),
                                           &hPublicKey, &hPrivateKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKeyPair created an RSA key pair. \n");
    } else {
        PKM_Error("C_GenerateKeyPair failed to create an RSA key pair.\n"
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_GenerateKey(hRwSession, &sKeyGenMechanism,
                                       sKeyTemplate, sizeof(sKeyTemplate)
                                       /sizeof(sKeyTemplate[0]), &hSecretKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKey succeeded\n");
    } else {
        PKM_Error( "C_GenerateKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    ulWrappedKeyLen = sizeof(wrappedKey);
    crv = pFunctionList->C_WrapKey(
                                  hRwSession, &wrapMechanism,
                                  hPublicKey, hSecretKey,
                                  wrappedKey, &ulWrappedKeyLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_WrapKey succeeded\n");
    } else {
        PKM_Error( "C_WrapKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_UnwrapKey(
                                    hRwSession, &wrapMechanism, hPrivateKey,
                                    wrappedKey, ulWrappedKeyLen, sKeyTemplate,
                                    sizeof(sKeyTemplate)
                                    /sizeof(sKeyTemplate[0]),
                                    &hSecretKeyUnwrapped);
    if (crv == CKR_OK) {
        PKM_LogIt("C_UnwrapKey succeeded\n");
    } else {
        PKM_Error( "C_UnwrapKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }



/*
* Todo: check that the hSecretKeyUnwrapped == hSecretKey
*/
    crv = pFunctionList->C_Logout(hRwSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error("C_Logout failed with 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CloseSession(hRwSession);
    if (crv != CKR_OK) {
        PKM_Error("C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                  PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;
}

/*
 * Tests if the object's attributes match the expected_attrs
 */
CK_RV
PKM_AttributeCheck(CK_FUNCTION_LIST_PTR pFunctionList,
                   CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE obj,
                   CK_ATTRIBUTE_PTR expected_attrs,
                   CK_ULONG expected_attrs_count)
{
    CK_RV crv;
    CK_ATTRIBUTE_PTR tmp_attrs;
    unsigned int i;

    /* First duplicate the themplate */
    tmp_attrs = malloc(expected_attrs_count * sizeof (CK_ATTRIBUTE));

    if (tmp_attrs == NULL) {
        PKM_Error("Internal test memory failure\n");
        return (CKR_HOST_MEMORY);
    }

    for (i = 0; i < expected_attrs_count; i++) {
        tmp_attrs[i].type = expected_attrs[i].type;
        tmp_attrs[i].ulValueLen = expected_attrs[i].ulValueLen;

        /* Don't give away the expected one. just zeros */
        tmp_attrs[i].pValue = calloc(expected_attrs[i].ulValueLen, 1);

        if (tmp_attrs[i].pValue == NULL) {
            unsigned int j;
            for (j = 0; j < i; j++)
                free(tmp_attrs[j].pValue);

            free(tmp_attrs);
            printf("Internal test memory failure\n");
            return (CKR_HOST_MEMORY);
        }
    }

    /* then get the attributes from the object */
    crv = pFunctionList->C_GetAttributeValue(hSession, obj, tmp_attrs,
                                             expected_attrs_count);
    if (crv != CKR_OK) {
        PKM_Error( "C_GetAttributeValue failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        crv = CKR_FUNCTION_FAILED;
        goto out;
    }

    /* Finally compare with the expected ones */
    for (i = 0; i < expected_attrs_count; i++) {

        if (memcmp(tmp_attrs[i].pValue, expected_attrs[i].pValue,
                   expected_attrs[i].ulValueLen) != 0) {
        PKM_LogIt("comparing attribute type 0x%x with expected  0x%x\n",
                      tmp_attrs[i].type, expected_attrs[i].type);
        PKM_LogIt("comparing attribute type value 0x%x with expected 0x%x\n",
                      tmp_attrs[i].pValue, expected_attrs[i].pValue);
        /* don't report error at this time */
        }
    }

    out:
    for (i = 0; i < expected_attrs_count; i++)
        free(tmp_attrs[i].pValue);
    free(tmp_attrs);
    return (crv);
}

/*
 * Check the validity of a mech
 */
CK_RV
PKM_MechCheck(CK_FUNCTION_LIST_PTR pFunctionList, CK_SESSION_HANDLE hSession,
              CK_MECHANISM_TYPE mechType, CK_FLAGS flags,
              CK_BBOOL check_sizes, CK_ULONG minkeysize, CK_ULONG maxkeysize)
{
    CK_SESSION_INFO         sess_info;
    CK_MECHANISM_INFO       mech_info;
    CK_RV                   crv;

    if ((crv = pFunctionList->C_GetSessionInfo(hSession, &sess_info))
        != CKR_OK) {
        PKM_Error( "C_GetSessionInfo failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return (CKR_FUNCTION_FAILED);
    }

    crv = pFunctionList->C_GetMechanismInfo(0, mechType,
                                            &mech_info);


    crv = pFunctionList->C_GetMechanismInfo(sess_info.slotID, mechType,
                                            &mech_info);

    if (crv != CKR_OK) {
        PKM_Error( "C_GetMechanismInfo failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return (CKR_FUNCTION_FAILED);
    }

    if ((mech_info.flags & flags) == 0) {
        PKM_Error("0x%x flag missing from mech\n", flags);
        return (CKR_MECHANISM_INVALID);
    }
    if (!check_sizes)
        return (CKR_OK);

    if (mech_info.ulMinKeySize != minkeysize) {
        PKM_Error("Bad MinKeySize %d expected %d\n", mech_info.ulMinKeySize,
                  minkeysize);
        return (CKR_MECHANISM_INVALID);
    }
    if (mech_info.ulMaxKeySize != maxkeysize) {
        PKM_Error("Bad MaxKeySize %d expected %d\n", mech_info.ulMaxKeySize,
                  maxkeysize);
        return (CKR_MECHANISM_INVALID);
    }
    return (CKR_OK);
}





/*
 * Can be called with a non-null premaster_key_len for the
 * *_DH mechanisms. In that case, no checking for the matching of
 * the expected results is done.
 * The rnd argument tells which correct/bogus randomInfo to use.
 */
CK_RV
PKM_TLSMasterKeyDerive( CK_FUNCTION_LIST_PTR pFunctionList,
                        CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                        CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen,
                        CK_MECHANISM_TYPE mechType,
                        enum_random_t rnd)  {
    CK_SESSION_HANDLE hSession;
    CK_RV crv;
    CK_MECHANISM            mk_mech;
    CK_VERSION              expected_version, version;
    CK_OBJECT_CLASS         class = CKO_SECRET_KEY;
    CK_KEY_TYPE             type = CKK_GENERIC_SECRET;
    CK_BBOOL                derive_bool = true;
    CK_ATTRIBUTE            attrs[] = {
                {CKA_CLASS, &class, sizeof (class)},
                {CKA_KEY_TYPE, &type, sizeof (type)},
                {CKA_DERIVE, &derive_bool, sizeof (derive_bool)},
                {CKA_VALUE, NULL, 0}};
    CK_ULONG                attrs_count = 4;
    CK_OBJECT_HANDLE        pmk_obj = CK_INVALID_HANDLE;
    CK_OBJECT_HANDLE        mk_obj = CK_INVALID_HANDLE;
    CK_SSL3_MASTER_KEY_DERIVE_PARAMS mkd_params;
    CK_MECHANISM            skmd_mech;

    CK_BBOOL isDH = false;

    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Before all, check if the mechanism is supported correctly */
    if (MODE == FIPSMODE) {
    crv = PKM_MechCheck(pFunctionList, hSession, mechType, CKF_DERIVE, false,
                        0, 0);
    if (crv != CKR_OK) {
        PKM_Error( "PKM_MechCheck failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return (crv);
    }
    }

    mk_mech.mechanism = mechType;
    mk_mech.pParameter = &mkd_params;
    mk_mech.ulParameterLen = sizeof (mkd_params);

    switch (mechType) {
    case CKM_TLS_MASTER_KEY_DERIVE_DH:
        isDH = true;
        /* FALLTHRU */
    case CKM_TLS_MASTER_KEY_DERIVE:
        attrs[3].pValue = NULL;
        attrs[3].ulValueLen = 0;
        expected_version.major = 3;
        expected_version.minor = 1;

        mkd_params.RandomInfo.pClientRandom = (unsigned char * ) TLSClientRandom;
        mkd_params.RandomInfo.ulClientRandomLen =
        sizeof (TLSClientRandom);
        mkd_params.RandomInfo.pServerRandom = (unsigned char * ) TLSServerRandom;
        mkd_params.RandomInfo.ulServerRandomLen =
        sizeof (TLSServerRandom);
        break;
    }
    mkd_params.pVersion = (!isDH) ? &version : NULL;

    /* First create the pre-master secret key */

    skmd_mech.mechanism = CKM_SSL3_PRE_MASTER_KEY_GEN;
    skmd_mech.pParameter = &mkd_params;
    skmd_mech.ulParameterLen = sizeof (mkd_params);


    crv = pFunctionList->C_GenerateKey(hSession, &skmd_mech,
                                       attrs,
                                       attrs_count,
                                       &pmk_obj);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKey succeeded\n");
    } else {
        PKM_Error( "C_GenerateKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;

    }
    /* Test the bad cases */
    switch (rnd) {
    case CORRECT:
        goto correct;

    case BOGUS_CLIENT_RANDOM:
        mkd_params.RandomInfo.pClientRandom = NULL;
        break;

    case BOGUS_CLIENT_RANDOM_LEN:
        mkd_params.RandomInfo.ulClientRandomLen = 0;
        break;

    case BOGUS_SERVER_RANDOM:
        mkd_params.RandomInfo.pServerRandom = NULL;
        break;

    case BOGUS_SERVER_RANDOM_LEN:
        mkd_params.RandomInfo.ulServerRandomLen = 0;
        break;
    }
    crv = pFunctionList->C_DeriveKey(hSession, &mk_mech, pmk_obj, NULL, 0,
                                     &mk_obj);
    if (crv != CKR_MECHANISM_PARAM_INVALID) {
        PKM_LogIt( "C_DeriveKey correctly failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
    } else {
        PKM_Error( "C_DeriveKey did not fail  with  bad data \n" );
    }
    goto out;


    correct:
    /* Now derive the master secret key */
    crv = pFunctionList->C_DeriveKey(hSession, &mk_mech, pmk_obj, NULL, 0,
                                     &mk_obj);
    if (crv == CKR_OK) {
        PKM_LogIt("C_DeriveKey succeeded\n");
    } else {
        PKM_Error( "C_DeriveKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;

    }

    out:
    if (pmk_obj != CK_INVALID_HANDLE)
        (void) pFunctionList->C_DestroyObject(hSession, pmk_obj);
    if (mk_obj != CK_INVALID_HANDLE)
        (void) pFunctionList->C_DestroyObject(hSession, mk_obj);
    crv = pFunctionList->C_Logout(hSession);

    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    return (crv);
}


CK_RV
PKM_TLSKeyAndMacDerive( CK_FUNCTION_LIST_PTR pFunctionList,
                        CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                        CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen,
                        CK_MECHANISM_TYPE mechType, enum_random_t rnd)
{
    CK_SESSION_HANDLE hSession;
    CK_RV crv;
    CK_MECHANISM            kmd_mech;
    CK_MECHANISM            skmd_mech;
    CK_OBJECT_CLASS         class = CKO_SECRET_KEY;
    CK_KEY_TYPE             type = CKK_GENERIC_SECRET;
    CK_BBOOL                derive_bool = true;
    CK_BBOOL                sign_bool = true, verify_bool = true;
    CK_BBOOL                encrypt_bool = true, decrypt_bool = true;
    CK_ULONG                value_len;

    /*
     * We arrange this template so that:
     * . Attributes 0-6 are good for a MAC key comparison template.
     * . Attributes 2-5 are good for the master key creation template.
     * . Attributes 3-8 are good for a cipher key comparison template.
     */
    CK_ATTRIBUTE            attrs[] = {
        /* 0 */ {CKA_SIGN, &sign_bool, sizeof (sign_bool)},
        /* 1 */ {CKA_VERIFY, &verify_bool, sizeof (verify_bool)},
        /* 2 */ {CKA_KEY_TYPE, &type, sizeof (type)},
        /* 3 */ {CKA_CLASS, &class, sizeof (class)},
        /* 4 */ {CKA_DERIVE, &derive_bool, sizeof (derive_bool)},
        /* 5 */ {CKA_VALUE, NULL, 0},
        /* 6 */ {CKA_VALUE_LEN, &value_len, sizeof (value_len)},
        /* 7 */ {CKA_ENCRYPT, &encrypt_bool, sizeof (encrypt_bool)},
        /* 8 */ {CKA_DECRYPT, &decrypt_bool, sizeof (decrypt_bool)}};

    CK_ULONG                attrs_count = 4;
    CK_OBJECT_HANDLE        mk_obj = CK_INVALID_HANDLE;
    CK_SSL3_KEY_MAT_PARAMS km_params;
    CK_SSL3_KEY_MAT_OUT kmo;
    CK_BYTE         IVClient[8];
    CK_BYTE         IVServer[8];

    crv = pFunctionList->C_OpenSession(pSlotList[slotID], CKF_SERIAL_SESSION,
                                       NULL, NULL, &hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_OpenSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Login(hSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error( "C_Login with correct password failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }


    /* Before all, check if the mechanism is supported correctly */
    if (MODE == FIPSMODE) {
        crv = PKM_MechCheck(pFunctionList, hSession, mechType, CKF_DERIVE,
                            CK_TRUE, 48, 48);

        if (crv != CKR_OK) {
            PKM_Error( "PKM_MechCheck failed with 0x%08X, %-26s\n", crv, 
                       PKM_CK_RVtoStr(crv));
            return (crv);
        }
    }
    kmd_mech.mechanism = mechType;
    kmd_mech.pParameter = &km_params;
    kmd_mech.ulParameterLen = sizeof (km_params);

    km_params.ulMacSizeInBits = 128;        /* an MD5 based MAC */
    km_params.ulKeySizeInBits = 192;        /* 3DES key size */
    km_params.ulIVSizeInBits = 64;          /* 3DES block size */
    km_params.pReturnedKeyMaterial = &kmo;
    km_params.bIsExport = false;
    kmo.hClientMacSecret = CK_INVALID_HANDLE;
    kmo.hServerMacSecret = CK_INVALID_HANDLE;
    kmo.hClientKey = CK_INVALID_HANDLE;
    kmo.hServerKey = CK_INVALID_HANDLE;
    kmo.pIVClient = IVClient;
    kmo.pIVServer = IVServer;

    skmd_mech.mechanism = CKM_SSL3_PRE_MASTER_KEY_GEN;
    skmd_mech.pParameter = &km_params;
    skmd_mech.ulParameterLen = sizeof (km_params);


    crv = pFunctionList->C_GenerateKey(hSession, &skmd_mech,
                                       &attrs[2],
                                       4,
                                       &mk_obj);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKey succeeded\n");
    } else {
        PKM_Error( "C_GenerateKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

        attrs[5].pValue = NULL;
        attrs[5].ulValueLen = 0;

    km_params.RandomInfo.pClientRandom = (unsigned char *) TLSClientRandom;
    km_params.RandomInfo.ulClientRandomLen =
    sizeof (TLSClientRandom);
    km_params.RandomInfo.pServerRandom = (unsigned char *) TLSServerRandom;
    km_params.RandomInfo.ulServerRandomLen =
    sizeof (TLSServerRandom);

    /* Test the bad cases */
    switch (rnd) {
    case CORRECT:
        goto correct;

    case BOGUS_CLIENT_RANDOM:
        km_params.RandomInfo.pClientRandom = NULL;
        break;

    case BOGUS_CLIENT_RANDOM_LEN:
        km_params.RandomInfo.ulClientRandomLen = 0;
        break;

    case BOGUS_SERVER_RANDOM:
        km_params.RandomInfo.pServerRandom = NULL;
        break;

    case BOGUS_SERVER_RANDOM_LEN:
        km_params.RandomInfo.ulServerRandomLen = 0;
        break;
    }
    crv = pFunctionList->C_DeriveKey(hSession, &kmd_mech, mk_obj, NULL, 0,
                                     NULL);
    if (crv != CKR_MECHANISM_PARAM_INVALID) {
        PKM_Error( "key materials derivation returned unexpected "
                   "error 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        (void) pFunctionList->C_DestroyObject(hSession, mk_obj);
        return (CKR_FUNCTION_FAILED);

    }
    return (CKR_OK);

    correct:
    /*
     * Then use the master key and the client 'n server random data to
     * derive the key materials
     */
    crv = pFunctionList->C_DeriveKey(hSession, &kmd_mech, mk_obj, NULL, 0,
                                     NULL);
    if (crv != CKR_OK) {
        PKM_Error( "Cannot derive the key materials, crv 0x%08X, %-26s\n", 
                   crv, PKM_CK_RVtoStr(crv));
        (void) pFunctionList->C_DestroyObject(hSession, mk_obj);
        return (crv);
    }

    if (mk_obj != CK_INVALID_HANDLE)
        (void) pFunctionList->C_DestroyObject(hSession, mk_obj);
    if (kmo.hClientMacSecret != CK_INVALID_HANDLE)
        (void) pFunctionList->C_DestroyObject(hSession, kmo.hClientMacSecret);
    if (kmo.hServerMacSecret != CK_INVALID_HANDLE)
        (void) pFunctionList->C_DestroyObject(hSession, kmo.hServerMacSecret);
    if (kmo.hClientKey != CK_INVALID_HANDLE);
        (void) pFunctionList->C_DestroyObject(hSession, kmo.hClientKey);
    if (kmo.hServerKey != CK_INVALID_HANDLE)
        (void) pFunctionList->C_DestroyObject(hSession, kmo.hServerKey);
    crv = pFunctionList->C_Logout(hSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_CloseSession(hSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }


    return (crv);
}

/*
*  PKM_KeyTests
*
*
*/

CK_RV PKM_KeyTests(CK_FUNCTION_LIST_PTR pFunctionList,
                        CK_SLOT_ID * pSlotList, CK_ULONG slotID,
                        CK_UTF8CHAR_PTR pwd, CK_ULONG pwdLen) {
    CK_SESSION_HANDLE hRwSession;

    CK_RV crv = CKR_OK;
    CK_BYTE PLAINTEXT[] = {"Firefox  Rules!"};
    CK_BYTE PLAINTEXT_PAD[] = {"Firefox and thunderbird rule the world!"};

/*** DSA Key ***/
    CK_MECHANISM dsaParamGenMech = {
        CKM_DSA_PARAMETER_GEN, NULL, 0
    };
    CK_ULONG primeBits = 1024;
    CK_ATTRIBUTE dsaParamGenTemplate[] = {
        {CKA_PRIME_BITS, &primeBits, sizeof(primeBits)}
    };
    CK_OBJECT_HANDLE hDsaParams;
    CK_BYTE DSA_P[128];
    CK_BYTE DSA_Q[20];
    CK_BYTE DSA_G[128];
    CK_MECHANISM dsaKeyPairGenMech = {
        CKM_DSA_KEY_PAIR_GEN, NULL, 0
    };
    CK_ATTRIBUTE dsaPubKeyTemplate[] = {
        {CKA_PRIME, DSA_P, sizeof(DSA_P)},
        {CKA_SUBPRIME, DSA_Q, sizeof(DSA_Q)},
        {CKA_BASE, DSA_G, sizeof(DSA_G)},
        {CKA_TOKEN, &true, sizeof(true)},
        {CKA_VERIFY, &true, sizeof(true)}
    };
    CK_ATTRIBUTE dsaPrivKeyTemplate[] = {
        {CKA_TOKEN, &true, sizeof(true)},
        {CKA_PRIVATE, &true, sizeof(true)},
        {CKA_SENSITIVE, &true, sizeof(true)},
        {CKA_SIGN, &true, sizeof(true)},
        {CKA_EXTRACTABLE, &true, sizeof(true)}
    };
    CK_OBJECT_HANDLE hDSApubKey, hDSAprivKey;

/**** RSA Key ***/
    CK_KEY_TYPE rsatype = CKK_RSA;
    CK_MECHANISM rsaKeyPairGenMech = {
        CKM_RSA_PKCS_KEY_PAIR_GEN, NULL, 0
    };
    CK_BYTE subject[] = {"RSA Private Key"};
    CK_ULONG modulusBits = 768;
    CK_BYTE publicExponent[] = { 3};
    CK_BYTE id[] = {"RSA123"};
    CK_ATTRIBUTE publicKeyTemplate[] = {
        {CKA_PRIVATE, &true, sizeof(true)},
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_DECRYPT, &true, sizeof(true)},
        {CKA_VERIFY, &true, sizeof(true)},
        {CKA_SIGN, &true, sizeof(true)},
        {CKA_WRAP, &true, sizeof(true)},
        {CKA_MODULUS_BITS, &modulusBits, sizeof(modulusBits)},
        {CKA_PUBLIC_EXPONENT, publicExponent, sizeof (publicExponent)}
    };
    CK_ATTRIBUTE privateKeyTemplate[] = {
        {CKA_TOKEN, &true, sizeof(true)},
        {CKA_PRIVATE, &true, sizeof(true)},
        {CKA_SUBJECT, subject, sizeof(subject)},
        {CKA_ID, id, sizeof(id)},
        {CKA_SENSITIVE, &true, sizeof(true)},
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_DECRYPT, &true, sizeof(true)},
        {CKA_VERIFY, &true, sizeof(true)},
        {CKA_SIGN, &true, sizeof(true)},
        {CKA_UNWRAP, &true, sizeof(true)}
    };
    CK_OBJECT_HANDLE hRSApubKey, hRSAprivKey;

/*** AES Key ***/
    CK_MECHANISM sAESKeyMech = {
        CKM_AES_KEY_GEN, NULL, 0
    };
    CK_BYTE wrappedKey[56];
    CK_ULONG wrappedKeyLen = 0;
    CK_OBJECT_CLASS class = CKO_SECRET_KEY;
    CK_KEY_TYPE keyAESType = CKK_AES;
    CK_UTF8CHAR AESlabel[] = "An AES secret key object";
    CK_ULONG AESvalueLen = 32;
    CK_ATTRIBUTE sAESKeyTemplate[] = {
        {CKA_CLASS, &class, sizeof(class)},
        {CKA_KEY_TYPE, &keyAESType, sizeof(keyAESType)},
        {CKA_LABEL, AESlabel, sizeof(AESlabel)-1},
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_DECRYPT, &true, sizeof(true)},
        {CKA_SIGN, &true, sizeof (true)},
        {CKA_VERIFY, &true, sizeof(true)},
        {CKA_UNWRAP, &true, sizeof(true)},
        {CKA_VALUE_LEN, &AESvalueLen, sizeof(AESvalueLen)}
    };
    
    CK_BYTE IV[] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    CK_OBJECT_HANDLE hAESSecKey;

/*** DES3 Key ***/
    CK_KEY_TYPE keyDES3Type = CKK_DES3;
    CK_UTF8CHAR DES3label[] = "An Triple DES secret key object";
    CK_ULONG DES3valueLen = 56;
    CK_MECHANISM sDES3KeyGenMechanism = {
        CKM_DES3_KEY_GEN, NULL, 0
    };
    CK_MECHANISM des3EcbMech = {
        CKM_DES3_ECB, NULL, 0
    };

    CK_ATTRIBUTE sDES3KeyTemplate[] = {
        {CKA_CLASS, &class, sizeof(class)},
        {CKA_KEY_TYPE, &keyDES3Type, sizeof(keyDES3Type)},
        {CKA_LABEL, DES3label, sizeof(DES3label)-1},
        {CKA_ENCRYPT, &true, sizeof(true)},
        {CKA_DECRYPT, &true, sizeof(true)},
        {CKA_UNWRAP, &true, sizeof(true)},
        {CKA_SIGN, &true, sizeof (true)},
        {CKA_VERIFY, &true, sizeof(true)},
        {CKA_VALUE_LEN, &DES3valueLen, sizeof(DES3valueLen)}
    };
    CK_OBJECT_HANDLE hDES3SecKey;

    CK_MECHANISM DES3_CBC_PAD_Mech = {
        CKM_DES3_CBC_PAD, IV, sizeof(IV)
    };
    CK_MECHANISM DES3_CBC_Mech = {
        CKM_DES3_CBC, IV, sizeof(IV)
    };

    CK_MECHANISM AES_CBC_PAD_Mech = {
        CKM_AES_CBC_PAD, IV, sizeof(IV)
    };
    CK_MECHANISM AES_CBC_Mech = {
        CKM_AES_CBC, IV, sizeof(IV)
    };

    CK_MECHANISM dsaWithSha1Mech = {
        CKM_DSA_SHA1, NULL, 0
    };

    struct mech_str {
        CK_ULONG    mechanism;
        const char *mechanismStr;
    };

    typedef struct mech_str mech_str;

    mech_str digestMechs[] = {
        CKM_SHA_1,  "CKM_SHA_1 ",
        CKM_SHA256, "CKM_SHA256",
        CKM_SHA384, "CKM_SHA384",
        CKM_SHA512, "CKM_SHA512"
    };
    mech_str hmacMechs[] = {
        CKM_SHA_1_HMAC, "CKM_SHA_1_HMAC", 
        CKM_SHA256_HMAC, "CKM_SHA256_HMAC",
        CKM_SHA384_HMAC, "CKM_SHA384_HMAC",
        CKM_SHA512_HMAC, "CKM_SHA512_HMAC"
    };
    mech_str sigRSAMechs[] = {
        CKM_SHA1_RSA_PKCS, "CKM_SHA1_RSA_PKCS", 
        CKM_SHA256_RSA_PKCS, "CKM_SHA256_RSA_PKCS",
        CKM_SHA384_RSA_PKCS, "CKM_SHA384_RSA_PKCS",
        CKM_SHA512_RSA_PKCS, "CKM_SHA512_RSA_PKCS"
    };

    CK_ULONG digestMechsSZ = NUM_ELEM(digestMechs);
    CK_ULONG sigRSAMechsSZ = NUM_ELEM(sigRSAMechs);
    CK_ULONG hmacMechsSZ = NUM_ELEM(hmacMechs);
    CK_MECHANISM mech;

    unsigned int i;

    memset(wrappedKey, 0, sizeof(wrappedKey));

    crv = pFunctionList->C_OpenSession(pSlotList[slotID],
                                       CKF_RW_SESSION | CKF_SERIAL_SESSION,
                                       NULL, NULL, &hRwSession);
    if (crv == CKR_OK) {
        PKM_LogIt("Opening a read/write session succeeded\n");
    } else {
        PKM_Error( "Opening a read/write session failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_Login(hRwSession, CKU_USER, pwd, pwdLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Login with correct password succeeded\n");
    } else {
        PKM_Error("C_Login with correct password failed "
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }


    /* generate an AES Secret Key */
    crv = pFunctionList->C_GenerateKey(hRwSession, &sAESKeyMech,
                                       sAESKeyTemplate,
                                       sizeof(sAESKeyTemplate)/
                                       sizeof(sAESKeyTemplate[0]),
                                       &hAESSecKey);

    /* generate an 3DES Secret Key */   
    crv = pFunctionList->C_GenerateKey(hRwSession, &sDES3KeyGenMechanism,
                                       sDES3KeyTemplate,
                                       sizeof(sDES3KeyTemplate)/
                                       sizeof(sDES3KeyTemplate[0]),
                                       &hDES3SecKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKey DES3 succeeded\n");
    } else {
        PKM_Error( "C_GenerateKey failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Generate DSA domain parameters PQG */
    crv = pFunctionList->C_GenerateKey(hRwSession, &dsaParamGenMech,
                                       dsaParamGenTemplate,
                                       sizeof(dsaParamGenTemplate)
                                       /sizeof(dsaParamGenTemplate[0]),
                                       &hDsaParams);
    if (crv == CKR_OK) {
        PKM_LogIt("DSA domain parameter generation succeeded\n");
    } else {
        PKM_Error( "DSA domain parameter generation failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_GetAttributeValue(hRwSession, hDsaParams,
                                             dsaPubKeyTemplate, 3);
    if (crv == CKR_OK) {
        PKM_LogIt("Getting DSA domain parameters succeeded\n");
    } else {
        PKM_Error( "Getting DSA domain parameters failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_DestroyObject(hRwSession, hDsaParams);
    if (crv == CKR_OK) {
        PKM_LogIt("Destroying DSA domain parameters succeeded\n");
    } else {
        PKM_Error( "Destroying DSA domain parameters failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Generate a persistent DSA key pair */
    crv = pFunctionList->C_GenerateKeyPair(hRwSession, &dsaKeyPairGenMech,
                                           dsaPubKeyTemplate,
                                           NUM_ELEM(dsaPubKeyTemplate),
                                           dsaPrivKeyTemplate,
                                           NUM_ELEM(dsaPrivKeyTemplate),
                                           &hDSApubKey, &hDSAprivKey);
    if (crv == CKR_OK) {
        PKM_LogIt("DSA key pair generation succeeded\n");
    } else {
        PKM_Error( "DSA key pair generation failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    /*** GEN RSA Key ***/
    crv = pFunctionList->C_GenerateKeyPair(hRwSession, &rsaKeyPairGenMech,
                                           publicKeyTemplate,
                                           (sizeof (publicKeyTemplate)
                                            / sizeof (CK_ATTRIBUTE)),
                                           privateKeyTemplate,
                                           (sizeof (privateKeyTemplate)
                                            / sizeof (CK_ATTRIBUTE)),
                                           &hRSApubKey, &hRSAprivKey);
    if (crv == CKR_OK) {
        PKM_LogIt("C_GenerateKeyPair created an RSA key pair. \n");
    } else {
        PKM_Error("C_GenerateKeyPair failed to create an RSA key pair.\n"
                  "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = PKM_SecKeyCrypt(pFunctionList, hRwSession,
                    hAESSecKey, &AES_CBC_PAD_Mech,
                    PLAINTEXT_PAD, sizeof(PLAINTEXT_PAD));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_SecKeyCrypt succeeded \n");
    } else {
        PKM_Error( "PKM_SecKeyCrypt failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    
    crv = PKM_SecKeyCrypt(pFunctionList, hRwSession,
                    hAESSecKey, &AES_CBC_Mech,
                    PLAINTEXT, sizeof(PLAINTEXT));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_SecKeyCrypt AES succeeded \n");
    } else {
        PKM_Error( "PKM_SecKeyCrypt failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = PKM_SecKeyCrypt(pFunctionList, hRwSession,
                    hDES3SecKey, &DES3_CBC_Mech,
                    PLAINTEXT, sizeof(PLAINTEXT));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_SecKeyCrypt DES3 succeeded \n");
    } else {
        PKM_Error( "PKM_SecKeyCrypt DES3 failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = PKM_SecKeyCrypt(pFunctionList, hRwSession,
                    hDES3SecKey, &DES3_CBC_PAD_Mech,
                    PLAINTEXT_PAD, sizeof(PLAINTEXT_PAD));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_SecKeyCrypt DES3 succeeded \n");
    } else {
        PKM_Error( "PKM_SecKeyCrypt DES3 failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }

    mech.pParameter = NULL;
    mech.ulParameterLen = 0;

    for (i=0; i < sigRSAMechsSZ; i++) {
        mech.mechanism = sigRSAMechs[i].mechanism;
        crv = PKM_PubKeySign(pFunctionList, hRwSession,
                       hRSApubKey, hRSAprivKey,
                       &mech,
                       PLAINTEXT, sizeof(PLAINTEXT));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_PubKeySign succeeded for %-10s\n", 
                sigRSAMechs[i].mechanismStr );
        } else {
            PKM_Error( "PKM_PubKeySign failed for %-10s  "
                "with 0x%08X, %-26s\n", sigRSAMechs[i].mechanismStr, crv, 
                PKM_CK_RVtoStr(crv));
            return crv;
        }
        crv = PKM_DualFuncSign(pFunctionList, hRwSession,
                       hRSApubKey, hRSAprivKey,
                       &mech,
                       hAESSecKey, &AES_CBC_Mech,
                       PLAINTEXT, sizeof(PLAINTEXT));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_DualFuncSign with AES secret key succeeded "
                      "for %-10s\n", 
                sigRSAMechs[i].mechanismStr );
        } else {
            PKM_Error( "PKM_DualFuncSign with AES secret key failed "
                       "for %-10s  "
                "with 0x%08X, %-26s\n", sigRSAMechs[i].mechanismStr, crv, 
                PKM_CK_RVtoStr(crv));
            return crv;
        }
        crv = PKM_DualFuncSign(pFunctionList, hRwSession,
                       hRSApubKey, hRSAprivKey,
                       &mech,
                       hDES3SecKey, &DES3_CBC_Mech,
                       PLAINTEXT, sizeof(PLAINTEXT));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_DualFuncSign with DES3 secret key succeeded "
                      "for %-10s\n", 
                sigRSAMechs[i].mechanismStr );
        } else {
            PKM_Error( "PKM_DualFuncSign with DES3 secret key failed "
                       "for %-10s  "
                "with 0x%08X, %-26s\n", sigRSAMechs[i].mechanismStr, crv, 
                PKM_CK_RVtoStr(crv));
            return crv;
        }
        crv = PKM_DualFuncSign(pFunctionList, hRwSession,
                       hRSApubKey, hRSAprivKey,
                       &mech,
                       hAESSecKey, &AES_CBC_PAD_Mech,
                       PLAINTEXT_PAD, sizeof(PLAINTEXT_PAD));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_DualFuncSign with AES secret key CBC_PAD "
                      "succeeded for %-10s\n", 
                sigRSAMechs[i].mechanismStr );
        } else {
            PKM_Error( "PKM_DualFuncSign with AES secret key CBC_PAD "
                       "failed for %-10s  "
                "with 0x%08X, %-26s\n", sigRSAMechs[i].mechanismStr, crv, 
                PKM_CK_RVtoStr(crv));
            return crv;
        }
        crv = PKM_DualFuncSign(pFunctionList, hRwSession,
                       hRSApubKey, hRSAprivKey,
                       &mech,
                       hDES3SecKey, &DES3_CBC_PAD_Mech,
                       PLAINTEXT_PAD, sizeof(PLAINTEXT_PAD));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_DualFuncSign with DES3 secret key CBC_PAD "
                      "succeeded for %-10s\n", 
                sigRSAMechs[i].mechanismStr );
        } else {
            PKM_Error( "PKM_DualFuncSign with DES3 secret key CBC_PAD "
                       "failed for %-10s  "
                "with 0x%08X, %-26s\n", sigRSAMechs[i].mechanismStr, crv, 
                PKM_CK_RVtoStr(crv));
            return crv;
        }

    } /* end of RSA for loop */

    crv = PKM_PubKeySign(pFunctionList, hRwSession,
                    hDSApubKey, hDSAprivKey,
                    &dsaWithSha1Mech, PLAINTEXT, sizeof(PLAINTEXT));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_PubKeySign succeeded \n");
    } else {
        PKM_Error( "PKM_PubKeySign failed "
                   "with 0x%08X, %-26s\n", crv, PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = PKM_DualFuncSign(pFunctionList, hRwSession,
                       hDSApubKey, hDSAprivKey,
                       &dsaWithSha1Mech,
                       hAESSecKey, &AES_CBC_Mech,
                       PLAINTEXT, sizeof(PLAINTEXT));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_DualFuncSign with AES secret key succeeded "
                "for DSAWithSHA1\n");
    } else {
        PKM_Error( "PKM_DualFuncSign with AES secret key failed "
                "for DSAWithSHA1 with 0x%08X, %-26s\n", 
                crv, PKM_CK_RVtoStr(crv));
           return crv;
    }
    crv = PKM_DualFuncSign(pFunctionList, hRwSession,
                       hDSApubKey, hDSAprivKey,
                       &dsaWithSha1Mech,
                       hDES3SecKey, &DES3_CBC_Mech,
                       PLAINTEXT, sizeof(PLAINTEXT));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_DualFuncSign with DES3 secret key succeeded "
                "for DSAWithSHA1\n");
    } else {
        PKM_Error( "PKM_DualFuncSign with DES3 secret key failed "
                "for DSAWithSHA1 with 0x%08X, %-26s\n", 
                crv, PKM_CK_RVtoStr(crv));
           return crv;
    }
    crv = PKM_DualFuncSign(pFunctionList, hRwSession,
                       hDSApubKey, hDSAprivKey,
                       &dsaWithSha1Mech,
                       hAESSecKey, &AES_CBC_PAD_Mech,
                       PLAINTEXT_PAD, sizeof(PLAINTEXT_PAD));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_DualFuncSign with AES secret key CBC_PAD succeeded "
                "for DSAWithSHA1\n");
    } else {
        PKM_Error( "PKM_DualFuncSign with AES secret key CBC_PAD failed "
                "for DSAWithSHA1 with 0x%08X, %-26s\n", 
                crv, PKM_CK_RVtoStr(crv));
           return crv;
    }
    crv = PKM_DualFuncSign(pFunctionList, hRwSession,
                       hDSApubKey, hDSAprivKey,
                       &dsaWithSha1Mech,
                       hDES3SecKey, &DES3_CBC_PAD_Mech,
                       PLAINTEXT_PAD, sizeof(PLAINTEXT_PAD));
    if (crv == CKR_OK) {
        PKM_LogIt("PKM_DualFuncSign with DES3 secret key CBC_PAD succeeded "
                "for DSAWithSHA1\n");
    } else {
        PKM_Error( "PKM_DualFuncSign with DES3 secret key CBC_PAD failed "
                "for DSAWithSHA1 with 0x%08X, %-26s\n", 
                crv, PKM_CK_RVtoStr(crv));
           return crv;
    }


    for (i=0; i < digestMechsSZ; i++) {
        mech.mechanism = digestMechs[i].mechanism;
        crv = PKM_Digest(pFunctionList, hRwSession,
                     &mech, hAESSecKey,
                     PLAINTEXT, sizeof(PLAINTEXT));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_Digest with AES secret key succeeded for %-10s\n", 
                digestMechs[i].mechanismStr);
        } else {
            PKM_Error( "PKM_Digest with AES secret key failed for "
                       "%-10s with 0x%08X,  %-26s\n", 
                       digestMechs[i].mechanismStr, crv, 
                       PKM_CK_RVtoStr(crv));
            return crv;
        }

        crv = PKM_Digest(pFunctionList, hRwSession,
                     &mech, 0,
                     PLAINTEXT, sizeof(PLAINTEXT));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_Digest with no secret key succeeded for %-10s\n", 
                digestMechs[i].mechanismStr );
        } else {
            PKM_Error( "PKM_Digest with no secret key failed for %-10s  "
                "with 0x%08X, %-26s\n", digestMechs[i].mechanismStr, crv, 
                PKM_CK_RVtoStr(crv));
            return crv;
        }
    } /* end of digest loop */

    for (i=0; i < hmacMechsSZ; i++) {
        mech.mechanism = hmacMechs[i].mechanism;
        crv = PKM_Hmac(pFunctionList, hRwSession,
                      hAESSecKey, &mech,
                     PLAINTEXT, sizeof(PLAINTEXT));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_Hmac with AES secret key succeeded for %-10s\n", 
                hmacMechs[i].mechanismStr);
        } else {
            PKM_Error( "PKM_Hmac with AES secret key failed for %-10s "
                       "with 0x%08X, %-26s\n", 
                       hmacMechs[i].mechanismStr, crv, PKM_CK_RVtoStr(crv));
            return crv;
        }
        if ((MODE == FIPSMODE) && (mech.mechanism == CKM_SHA512_HMAC)) break;
        crv = PKM_Hmac(pFunctionList, hRwSession,
                      hDES3SecKey, &mech,
                     PLAINTEXT, sizeof(PLAINTEXT));
        if (crv == CKR_OK) {
            PKM_LogIt("PKM_Hmac with DES3 secret key succeeded for %-10s\n", 
                hmacMechs[i].mechanismStr);
        } else {
            PKM_Error( "PKM_Hmac with DES3 secret key failed for %-10s "
                       "with 0x%08X,  %-26s\n", 
                       hmacMechs[i].mechanismStr, crv, PKM_CK_RVtoStr(crv));
            return crv;
        }

    } /* end of hmac loop */

    crv = pFunctionList->C_Logout(hRwSession);
    if (crv == CKR_OK) {
        PKM_LogIt("C_Logout succeeded\n");
    } else {
        PKM_Error( "C_Logout failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_CloseSession(hRwSession);
    if (crv != CKR_OK) {
        PKM_Error( "C_CloseSession failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    return crv;

}


CK_RV PKM_DualFuncSign(CK_FUNCTION_LIST_PTR pFunctionList,
                       CK_SESSION_HANDLE hRwSession,
                       CK_OBJECT_HANDLE publicKey, CK_OBJECT_HANDLE privateKey,
                       CK_MECHANISM *sigMech,
                       CK_OBJECT_HANDLE secretKey, CK_MECHANISM *cryptMech,
                       CK_BYTE *pData, CK_ULONG pDataLen) {

    CK_RV crv = CKR_OK;
    CK_BYTE encryptedData[MAX_CIPHER_SZ];
    CK_ULONG ulEncryptedDataLen = 0;
    CK_ULONG ulLastUpdateSize = 0 ;
    CK_BYTE sig[MAX_SIG_SZ];
    CK_ULONG ulSigLen = 0;
    CK_BYTE data[MAX_DATA_SZ];
    CK_ULONG ulDataLen = 0;

    memset(encryptedData, 0, sizeof(encryptedData));
    memset(sig, 0, sizeof(sig));
    memset(data, 0, sizeof(data));

    /* Check that the mechanism is Multi-part */
    if (sigMech->mechanism == CKM_DSA || sigMech->mechanism == CKM_RSA_PKCS) {
        PKM_Error( "PKM_DualFuncSign must be called with a Multi-part "
                   "operation mechanism\n");        
        return CKR_DEVICE_ERROR;
    }

    /* Sign and Encrypt */
    if (privateKey == 0 && publicKey == 0) {
        crv = pFunctionList->C_SignInit(hRwSession, sigMech, secretKey);
        if (crv != CKR_OK) {
            PKM_Error( "C_SignInit failed with 0x%08X, %-26s\n", crv, 
                       PKM_CK_RVtoStr(crv));
            return crv;
        }
    } else {
        crv = pFunctionList->C_SignInit(hRwSession, sigMech, privateKey);
        if (crv != CKR_OK) {
            PKM_Error( "C_SignInit failed with 0x%08X, %-26s\n", crv, 
                       PKM_CK_RVtoStr(crv));
            return crv;
        }
     }
    crv = pFunctionList->C_EncryptInit(hRwSession, cryptMech, secretKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }


    ulEncryptedDataLen = sizeof(encryptedData);
    crv = pFunctionList->C_SignEncryptUpdate(hRwSession, pData,
                                             pDataLen,
                                             encryptedData,
                                             &ulEncryptedDataLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_Sign failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    ulLastUpdateSize = sizeof(encryptedData) - ulEncryptedDataLen;
    crv = pFunctionList->C_EncryptFinal(hRwSession,
         (CK_BYTE_PTR)&encryptedData[ulEncryptedDataLen], &ulLastUpdateSize);
    if (crv != CKR_OK) {
        PKM_Error( "C_EncryptFinal failed with 0x%08X, %-26s\n", crv,
                    PKM_CK_RVtoStr(crv));
        return crv;
    }
    ulEncryptedDataLen = ulEncryptedDataLen + ulLastUpdateSize; 
    ulSigLen = sizeof(sig);
    crv = pFunctionList->C_SignFinal(hRwSession, sig, &ulSigLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_SignFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Decrypt and Verify */

    crv = pFunctionList->C_DecryptInit(hRwSession, cryptMech, secretKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    crv = pFunctionList->C_VerifyInit(hRwSession, sigMech,
                                      publicKey);
    if (crv != CKR_OK) {
        PKM_Error( "C_VerifyInit failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    ulDataLen = sizeof(data);
    crv = pFunctionList->C_DecryptVerifyUpdate(hRwSession,
                                               encryptedData,
                                               ulEncryptedDataLen,
                                               data, &ulDataLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptVerifyUpdate failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    ulLastUpdateSize = sizeof(data) - ulDataLen;
    /* Get last little piece of plaintext.  Should have length 0 */
    crv = pFunctionList->C_DecryptFinal(hRwSession, &data[ulDataLen],
                                        &ulLastUpdateSize);
    if (crv != CKR_OK) {
        PKM_Error( "C_DecryptFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }
    
    if (ulLastUpdateSize != 0) {
        crv = pFunctionList->C_VerifyUpdate(hRwSession, &data[ulDataLen],
                                        ulLastUpdateSize);
        if (crv != CKR_OK) {
            PKM_Error( "C_DecryptFinal failed with 0x%08X, %-26s\n", crv, 
                       PKM_CK_RVtoStr(crv));
            return crv;
        }
    }
    ulDataLen = ulDataLen + ulLastUpdateSize; 

    /* input for the verify operation is the decrypted data */
    crv = pFunctionList->C_VerifyFinal(hRwSession, sig, ulSigLen);
    if (crv == CKR_OK) {
        PKM_LogIt("C_VerifyFinal succeeded\n");
    } else {
        PKM_Error( "C_VerifyFinal failed with 0x%08X, %-26s\n", crv, 
                   PKM_CK_RVtoStr(crv));
        return crv;
    }

    /* Comparison of Decrypted data with inputed data */
    if ( (ulDataLen == pDataLen) && 
         (memcmp(data, pData, pDataLen) == 0) ) {
        PKM_LogIt("PKM_DualFuncSign decrypt test case passed\n");
    } else {
        PKM_Error( "PKM_DualFuncSign derypt test case failed\n");
    }

    return crv;

}

CK_RV PKM_Digest(CK_FUNCTION_LIST_PTR pFunctionList, 
                 CK_SESSION_HANDLE hSession,
                 CK_MECHANISM *digestMech, CK_OBJECT_HANDLE hSecretKey,
                 CK_BYTE *pData, CK_ULONG pDataLen) {
    CK_RV crv = CKR_OK;
    CK_BYTE digest1[MAX_DIGEST_LEN];
    CK_ULONG digest1Len = 0 ;
    CK_BYTE digest2[MAX_DIGEST_LEN];
    CK_ULONG digest2Len = 0;

    /* Tested with CKM_SHA_1, CKM_SHA256, CKM_SHA384, CKM_SHA512 */

    memset(digest1, 0, sizeof(digest1));
    memset(digest2, 0, sizeof(digest2));

    crv = pFunctionList->C_DigestInit(hSession, digestMech);
    if (crv != CKR_OK) {
        PKM_Error( "C_SignInit failed with 0x%08X, %-26s\n", crv, 
            PKM_CK_RVtoStr(crv));
        return crv;
    }
    digest1Len = sizeof(digest1);
    crv = pFunctionList->C_Digest(hSession, pData, pDataLen, 
        digest1, &digest1Len);
    if (crv != CKR_OK) {
        PKM_Error( "C_Sign failed with 0x%08X, %-26s\n", crv, 
            PKM_CK_RVtoStr(crv));
        return crv;
    }


    crv = pFunctionList->C_DigestInit(hSession, digestMech);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestInit failed with 0x%08X, %-26s\n", crv, 
            PKM_CK_RVtoStr(crv));
        return crv;
    }

    crv = pFunctionList->C_DigestUpdate(hSession, pData, pDataLen);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestUpdate failed with 0x%08X, %-26s\n", crv, 
            PKM_CK_RVtoStr(crv));
        return crv;
    }
 	
    /* C_DigestKey continues a multiple-part message-digesting operation by*/
    /* digesting the value of a secret key. (only used with C_DigestUpdate)*/
    if (hSecretKey != 0) {
        crv = pFunctionList->C_DigestKey(hSession, hSecretKey);
        if (crv != CKR_OK) {
            PKM_Error( "C_DigestKey failed with 0x%08X, %-26s\n", crv, 
                PKM_CK_RVtoStr(crv));
            return crv;
        }
    }

    digest2Len = sizeof(digest2);
    crv = pFunctionList->C_DigestFinal(hSession, digest2, &digest2Len);
    if (crv != CKR_OK) {
        PKM_Error( "C_DigestFinal failed with 0x%08X, %-26s\n", crv, 
            PKM_CK_RVtoStr(crv));
        return crv;
    }

    if (hSecretKey == 0){
        /* did not digest a secret key so digests should equal */ 
        if  ( (digest1Len == digest2Len) 
            && (memcmp(digest1, digest2, digest1Len) == 0) ) {
                PKM_LogIt("Single and Multiple-part message digest "
                    "operations succesful\n");
            } else {
                PKM_Error("Single and Multiple-part message digest "
                    "operations failed\n");
            }
    } else {
        if  (digest1Len == digest2Len) { 
            PKM_LogIt("PKM_Digest Single and Multiple-part message digest "
                "operations succesful\n");
        } else {
            PKM_Error("PKM_Digest Single and Multiple-part message digest "
                "operations failed\n");
        }

    }

    return crv;

}

