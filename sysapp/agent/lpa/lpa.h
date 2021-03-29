#ifndef __LPA_H__
#define __LPA_H__

#include <stdint.h>

#define TAG_LPA_GET_EUICC_INFO1_REQ             0xBF20
#define TAG_LPA_PREPARE_DOWNLOAD_REQ            0xBF21
#define TAG_LPA_GET_EUICC_INFO2_REQ             0xBF22
#define TAG_LPA_LIST_NOTIFICATION_REQ           0xBF28
#define TAG_LPA_SET_NICK_NAME_REQ               0xBF29
#define TAG_LPA_RETRIEVE_NOTIFICATION_REQ       0xBF2B
#define TAG_LPA_PROFILE_INFO_LIST_REQ           0xBF2D
#define TAG_LPA_GET_EUICC_CHALLENGE_REQ         0xBF2E
#define TAG_LPA_REMOVE_NOTIFICATION_REQ         0xBF30
#define TAG_LPA_ENABLE_PROFILE_REQ              0xBF31
#define TAG_LPA_DISABLE_PROFILE_REQ             0xBF32
#define TAG_LPA_DELETE_PROFILE_REQ              0xBF33
#define TAG_LPA_EUICC_MEMORY_RESET_REQ          0xBF34
#define TAG_LPA_AUTH_SERVER_REQ                 0xBF38
#define TAG_LPA_GET_EUICC_CONFIG_ADDR_REQ       0xBF3C
#define TAG_LPA_GET_EUICC_DATA_REQ              0xBF3E
#define TAG_LPA_SET_DEFAULT_DP_ADDR_REQ         0xBF3F
#define TAG_LPA_CACEL_SESSION_REQ               0xBF41
#define TAG_LPA_GET_RAT_REQ                     0xBF43

#define TAG_LPA_SET_ROOT_KEY_REQ                0xFF20

typedef enum LPA_CHANNEL_TYPE {
    LPA_CHANNEL_BY_IPC = 0,     // vUICC mode
    LPA_CHANNEL_BY_QMI          // eUICC mode
} lpa_channel_type_e;

// See SGP.22_v2.2 for more information
typedef struct PROFILE_INFO {
    char iccid[21];             // 20-digit ICCID, padded with F
    uint8_t class;              // 0 test, 1 provisioning, 2 operational
    uint8_t state;              // 0 disabled, 1 enabled
} profile_info_t;

extern uint8_t g_buf[10 * 1024];
extern uint16_t g_buf_size;

int init_lpa(void *arg);

int lpa_get_eid(uint8_t *eid);

int lpa_get_profile_info(profile_info_t *pi, uint8_t *num, uint8_t max_num);

int lpa_get_eid_list(uint8_t (*eid_list)[33]);

int lpa_switch_eid(const uint8_t *eid);

/**
 @return:
    ok(0), iccidOrAidNotFound (1), profileNotInDisabledState(2),
    disallowedByPolicy(3), undefinedError(127),
    others reference lpa_error_codes.h
*/
int lpa_delete_profile(const char *iccid);

/**
 @return:
    ok(0), iccidOrAidNotFound(1), profileNotInDisabledState(2),
    disallowedByPolicy(3), wrongProfileReenabling(4), catBusy(5),
    undefinedError(127),
    others reference lpa_error_codes.h
*/
int lpa_enable_profile(const char *iccid);

/**
 @return:
    ok(0), iccidOrAidNotFound(1), profileNotInEnabledState(2),
    disallowedByPolicy(3), catBusy(5), undefinedError(127),
    others reference lpa_error_codes.h
*/
int lpa_disable_profile(const char *iccid);

/**
 @return:
    ok(0), otherwise,

    byte 1 for BPP Command ID,
        initialiseSecureChannel(0), configureISDP(1), storeMetadata(2),
        storeMetadata2(3), replaceSessionKeys(4), loadProfileElements(5)

    byte 2 for Error Reason,
        incorrectInputValues(1),
        invalidSignature(2),
        invalidTransactionId(3),
        unsupportedCrtValues(4),
        unsupportedRemoteOperationType(5),
        unsupportedProfileClass(6),
        scp03tStructureError(7),
        scp03tSecurityError(8),
        installFailedDueToIccidAlreadyExistsOnEuicc(9),
        installFailedDueToInsufficientMemoryForProfile(10),
        installFailedDueToInterruption(11),
        installFailedDueToPEProcessingError (12),
        installFailedDueToDataMismatch(13),
        testProfileInstallFailedDueToInvalidNaaKey(14),
        pprNotAllowed(15),
        installFailedDueToUnknownError(127)

    others reference lpa_error_codes.h
*/
int lpa_download_profile(const char *ac, const char *cc, char iccid[21],uint8_t *server_url);

int lpa_load_customized_data(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len);

#endif  /* __LPA_H__ */

