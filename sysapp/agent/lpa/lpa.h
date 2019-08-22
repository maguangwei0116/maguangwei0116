#ifndef __LPA_H__
#define __LPA_H__

#include <stdint.h>

// See SGP.22_v2.2 for more information
typedef struct profile_info {
    char iccid[21]; // 20-digit ICCID, padded with F
    uint8_t class;  // 0 test, 1 provisioning, 2 operational
    uint8_t state;  // 0 disabled, 1 enabled
} profile_info_t;

int lpa_init(void *fun, void *arg);
int lpa_get_eid(uint8_t *eid);
int lpa_get_profile_info(profile_info_t *pi, uint8_t *num);
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

int lpa_load_cert(const uint8_t *data, uint16_t data_len);

int lpa_load_profile(const uint8_t *data, uint16_t data_len);
#endif  // __LPA_H__
