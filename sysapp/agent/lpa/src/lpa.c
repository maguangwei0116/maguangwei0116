
#include <stdio.h>

#include "rt_os.h"
#include "lpa.h"
#include "lpa_config.h"
#include "lpdd.h"
#include "luid.h"
#include "convert.h"
#include "lpa_https.h"
#include "lpa_error_codes.h"

#define BUFFER_SIZE                10*1024

uint8_t g_buf[10 * 1024];
uint16_t g_buf_size;

static rt_pthread_mutex_t *g_lpa_mutex;

extern void init_apdu_channel(lpa_channel_type_e channel_mode);

int init_lpa(void *arg)
{
    init_apdu_channel(*(lpa_channel_type_e *)arg);

    g_lpa_mutex = linux_mutex_init();

    return g_lpa_mutex ? RT_SUCCESS : RT_ERROR;
}

int lpa_get_eid(uint8_t *eid)
{
    //BF3E125A1089049032123451234512345678901235
    // BF3E 12 5A 10 EID
    uint8_t channel = 0xFF;
    uint8_t buf[64] = {0};  // 21 bytes is not enough, 2 more bytes is in need !!! It may stack buffer overflow !!!
    uint16_t size = sizeof(buf);
    int ret = RT_SUCCESS;
    int cnt = 3;  // max 3 times
    int i = 0;

    linux_mutex_lock(g_lpa_mutex);

    for (i = 0; i < cnt; i++) {
        if (open_channel(&channel) != RT_SUCCESS) {
            ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
            goto end;
        }
        //hexstring2bytes((uint8_t *)eid, buf, &size);
        ret = get_eid(buf, &size, channel);
        memcpy(eid, &buf[5], 16);
        close_channel(channel);

        /* retry more times when APDU SW error */
        if (ret != RT_ERR_APDU_STORE_DATA_FAIL) {
            break;
        }

        rt_os_sleep(1);
    }

end:

    linux_mutex_unlock(g_lpa_mutex);

    return ret;
}

int lpa_switch_eid(const uint8_t *eid)
{
    uint8_t rsp[33] = {0};
    uint8_t channel = 0xFF;
    uint16_t rsp_size = sizeof(rsp);
    int ret = RT_SUCCESS;

    linux_mutex_lock(g_lpa_mutex);

    if (open_channel(&channel) != RT_SUCCESS) {
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        goto end;
    }

    MSG_INFO("eid:%s\n", eid);
    hexstring2bytes((uint8_t *)eid, rsp, &rsp_size);
    switch_eid(rsp, rsp_size, rsp, &rsp_size, channel);
    close_channel(channel);

    ret = RT_SUCCESS;

end:

    linux_mutex_unlock(g_lpa_mutex);

    return RT_SUCCESS;
}

int lpa_get_eid_list(uint8_t (*eid_list)[33])
{
    // Reserved

    return RT_SUCCESS;
}

int lpa_get_profile_info(profile_info_t *pi, uint8_t *num, uint8_t max_num)
{
    int ret = RT_SUCCESS;
    uint8_t *buf = NULL;
    uint16_t size = 1024 * 10;
    uint32_t i;
    uint32_t offset;
    uint32_t total_info_len;
    uint32_t info_len;
    uint16_t info_off;
    uint16_t element_off;
    uint8_t channel = 0xFF;

    linux_mutex_lock(g_lpa_mutex);

    if (open_channel(&channel) != RT_SUCCESS) {
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        goto end;
    }
    if (!num) {
        ret = RT_ERR_NULL_POINTER;
        goto end;
    }
    buf = (uint8_t *) malloc (size);
    if (!buf) {
        ret = RT_ERR_NULL_POINTER;
        goto end;
    }
    memset(buf, 0, size);
    size = 0;
    get_profiles_info(SEARCH_NONE, NULL, 0, (uint8_t *)buf, (uint16_t *)&size, channel);

    MSG_DUMP_ARRAY("profile info:\n", buf, size);

    /*
    -- Definition of ProfileInfoListResponse 
    ProfileInfoListResponse ::= [45] CHOICE { -- Tag 'BF2D' 
        profileInfoListOk SEQUENCE OF ProfileInfo, 
        profileInfoListError ProfileInfoListError 
    } 
    ProfileInfo ::= [PRIVATE 3] SEQUENCE { -- Tag 'E3' 
        iccid Iccid OPTIONAL, 
        isdpAid [APPLICATION 15] OctetTo16 OPTIONAL, -- AID of the ISD-P containing the Profile, tag '4F' 
        profileState [112] ProfileState OPTIONAL, -- Tag '9F70' 
        profileNickname [16] UTF8String (SIZE(0..64)) OPTIONAL, -- Tag '90' 
        serviceProviderName [17] UTF8String (SIZE(0..32)) OPTIONAL, -- Tag '91' 
        profileName [18] UTF8String (SIZE(0..64)) OPTIONAL, -- Tag '92' 
        iconType [19] IconType OPTIONAL, -- Tag '93' 
        icon [20] OCTET STRING (SIZE(0..1024)) OPTIONAL, -- Tag '94', see condition in ES10c:GetProfilesInfo 
        profileClass [21] ProfileClass OPTIONAL, -- Tag '95' 
        notificationConfigurationInfo [22] SEQUENCE OF NotificationConfigurationInformation OPTIONAL, -- Tag 'B6' 
        profileOwner [23] OperatorId OPTIONAL, -- Tag 'B7' 
        dpProprietaryData [24] DpProprietaryData OPTIONAL, -- Tag 'B8' 
        profilePolicyRules [25] PprIds OPTIONAL -- Tag '99'
    } 
    
    IconType ::= INTEGER { jpg(0), png(1) } 
    ProfileState ::= INTEGER { disabled(0), enabled(1) } 
    ProfileClass ::= INTEGER { test(0), provisioning(1), operational(2) } 
    ProfileInfoListError ::= INTEGER { incorrectInputValues(1), undefinedError(127) }
    */

    //  ProfileInfoListResponse
    offset = bertlv_get_tl_length(buf, NULL);

    if (bertlv_get_tag(buf + offset, NULL) != 0xA0) {
        ret = RT_ERR_UNKNOWN_ERROR;
        goto end;
    }

    // profileInfoListOk SEQUENCE OF ProfileInfo
    offset += bertlv_get_tl_length(buf + offset, &total_info_len);

    if (pi != NULL) {
        for (i = 0, *num = 0; i < total_info_len; *num++) {
            if (*num >= max_num) {
                MSG_WARN("too many profile detected (%d > %d)\n", *num, max_num);
                break;;
            }
            // ProfileInfo E3 TL
            info_off = bertlv_get_tl_length(buf + offset + i, &info_len);
            // find ICCID
            element_off = bertlv_find_tag(buf + offset + i + info_off, info_len, 0x5A, 1);
            if (element_off != BERTLV_INVALID_OFFSET) {
                // get ICCID value offset
                element_off += bertlv_get_tl_length(buf + offset + i + info_off + element_off, NULL);
                swap_nibble(buf + offset + i + info_off + element_off, 10);
                bytes2hexstring(buf + offset + i + info_off + element_off, 10, pi[i].iccid);
            }

            // find ProfileClass
            element_off = bertlv_find_tag(buf + offset + i + info_off, info_len, 0x95, 1);
            if (element_off != BERTLV_INVALID_OFFSET) {
                pi[i].class = (uint8_t)bertlv_get_integer(buf + offset + i + info_off + element_off);
            }

            // find ProfileClass
            element_off = bertlv_find_tag(buf + offset + i + info_off, info_len, 0x9F70, 1);
            if (element_off != BERTLV_INVALID_OFFSET) {
                pi[i].state = (uint8_t)bertlv_get_integer(buf + offset + i + info_off + element_off);
            }

            // get next E3 TLV
            i += bertlv_get_tlv_length(buf + offset + i);
        }
    }

end:

    if (buf != NULL) {
        free(buf);
    }

    if (ret != RT_ERR_APDU_OPEN_CHANNEL_FAIL) {
        close_channel(channel);
    }

    linux_mutex_unlock(g_lpa_mutex);

    return ret;
}

int lpa_delete_profile(const char *iccid)
{
    int ret;
    uint8_t rsp[32] = {0};
    uint16_t rsp_size = sizeof(rsp);
    uint8_t channel = 0xFF;

    linux_mutex_lock(g_lpa_mutex);

    if (open_channel(&channel) != RT_SUCCESS) {
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        goto end;
    }
    hexstring2bytes(iccid, rsp, &rsp_size);
    swap_nibble(rsp, 10);
    MSG_DUMP_ARRAY("ICCID: ", rsp, 10);
    ret = delete_profile(PID_ICCID, rsp, rsp, &rsp_size, channel);
    if (ret == RT_SUCCESS) {
        MSG_DUMP_ARRAY("lpa_delete_profile: ", rsp, rsp_size);
        // BF33038001 Result
        ret = rsp[5];
    }

    close_channel(channel);

end:

    linux_mutex_unlock(g_lpa_mutex);

    return ret;
}

int lpa_enable_profile(const char *iccid)
{
    int ret;
    uint8_t rsp[32] = {0};
    uint16_t rsp_size = sizeof(rsp);
    uint8_t channel = 0xFF;

    linux_mutex_lock(g_lpa_mutex);

    if (open_channel(&channel) != RT_SUCCESS) {
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        goto end;
    }
    hexstring2bytes(iccid, rsp, &rsp_size);
    swap_nibble(rsp, 10);
    MSG_DUMP_ARRAY("ICCID: ", rsp, 10);
    ret = enable_profile(PID_ICCID, rsp, true, rsp, &rsp_size, channel);
    if (ret == RT_SUCCESS) {
        MSG_DUMP_ARRAY("lpa_enable_profile: ", rsp, rsp_size);
        // BF31038001 Result
        ret = rsp[5];
    } else if (ret == RT_ERR_AT_WRONG_RSP) {
        // With refresh request, it might be failed to get response, this also indicates success
        ret = 0;
    }

    close_channel(channel);

end:

    linux_mutex_unlock(g_lpa_mutex);

    return ret;
}

int lpa_disable_profile(const char *iccid)
{
    int ret;
    uint8_t rsp[32] = {0};
    uint16_t rsp_size = sizeof(rsp);
    uint8_t channel = 0xFF;

    linux_mutex_lock(g_lpa_mutex);

    if (open_channel(&channel) != RT_SUCCESS) {
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        goto end;
    }
    hexstring2bytes(iccid, rsp, &rsp_size);
    swap_nibble(rsp, 10);
    MSG_DUMP_ARRAY("ICCID: ", rsp, 10);
    ret = disable_profile(PID_ICCID, rsp, true, rsp, &rsp_size, channel);
    if (ret == RT_SUCCESS) {
        MSG_DUMP_ARRAY("lpa_disable_profile: ", rsp, rsp_size);
        // BF32038001 Result
        ret = rsp[5];
    } else if (ret == RT_ERR_AT_WRONG_RSP) {
        // With refresh request, it might be failed to get response, this also indicates success
        ret = 0;
    }

    close_channel(channel);

end:

    linux_mutex_unlock(g_lpa_mutex);

    return ret;
}

static int check_ac(const char *ac, uint16_t *smdp_addr_start, uint16_t *smdp_addr_len,
                uint16_t *matching_id_start, uint16_t *matching_id_len, bool *need_cc)
{
    const char *p1 = NULL, *p2 = NULL;

    // AC
    // 1$QUARK-QA.REDTEA.IO$TK3J73RBQ3K91NUP$$1
    // 1$esim.wo.cn$SATKB8Z-I4YBPHJS$1.3.6.1.4.1.47814.2.4$1
    if (ac == NULL) {
        return RT_ERR_NULL_POINTER;
    }

    if ((smdp_addr_start == NULL) || (smdp_addr_len == NULL) ||
        (matching_id_start == NULL) || (matching_id_len == NULL) ||
        (need_cc == NULL)) {
        return RT_ERR_NULL_POINTER;
    }

    *need_cc = false;
    p1 = ac;
    if ((*p1++ != '1') || (*p1++ != '$')) { // AC_Format and Delimiter 1, M
        return RT_ERR_WRONG_AC_FORMAT;
    }
    // p1++;

    MSG_DBG("ac: %p, p1: %p, p2: %p\n", ac, p1, p2);
    *smdp_addr_start = p1 - ac;             // SM-DP+ Address, M
    p2 = strstr(p1, "$");                   // Delimiter 2, M
    if (p2 == NULL) {
        return RT_ERR_WRONG_AC_FORMAT;
    }
    *smdp_addr_len = p2 - p1;
    p2++;

    MSG_DBG("ac: %p, p1: %p, p2: %p\n", ac, p1, p2);
    *matching_id_start = p2 - ac;           // AC_token, M
    p1 = strstr(p2, "$");                   // Delimiter 3, M
    if (p1 == NULL) {
        *matching_id_len = strlen(p2);
        return RT_SUCCESS;
    }
    *matching_id_len = p1 - p2;
    p1++;                                   // SM-DP OID, O

    MSG_DBG("ac: %p, p1: %p, p2: %p\n", ac, p1, p2);
    p2 = strstr(p1, "$");                   // Delimiter 4, C
    if (p2 == NULL) {
        return RT_SUCCESS;
    }
    p2++;                                   // Confirmation Code Required Flag, O

    MSG_DBG("ac: %p, p1: %p, p2: %p\n", ac, p1, p2);
    if (*p2 != '1') {
        return RT_SUCCESS;
    }
    *need_cc = true;

    return RT_SUCCESS;
}

static int process_bpp_rsp(const uint8_t* pir, uint16_t pir_len,
                                char* iccid, uint8_t* bppcid, uint8_t* error)
{
    // char* bpp_cmd = NULL;
    // char* error_reason = NULL;
    uint16_t tag;
    uint16_t result_data_offset = 0;
    uint16_t iccid_offset = 0;
    uint16_t final_result_offset = 0;
    uint16_t bpp_id_off;
    uint16_t error_reson_off;
    uint16_t euicc_response_off;
    uint32_t element_len;
    uint32_t notify_metadata_len;
    int ret = 0;

    /*
    ProfileInstallationResult ::= [55] SEQUENCE { -- Tag 'BF37'
        profileInstallationResultData [39] ProfileInstallationResultData,
        euiccSignPIR EuiccSignPIR
    }

    ProfileInstallationResultData ::= [39] SEQUENCE { -- Tag 'BF27'
        transactionId[0] TransactionId, -- The TransactionID generated by the SM-DP+
        notificationMetadata[47] NotificationMetadata,
        smdpOid OBJECT IDENTIFIER, -- SM-DP+ OID (same value as in CERT.DPpb.ECDSA)
        finalResult [2] CHOICE {
            successResult SuccessResult,
            errorResult ErrorResult
        }
    }

    EuiccSignPIR [APPLICATION 55] OCTET STRING -- Tag '5F37', eUICC¡¯s signature

    SuccessResult ::= SEQUENCE {
        aid [APPLICATION 15] OCTET STRING (SIZE (5..16)), -- AID of ISD-P
        simaResponse OCTET STRING -- contains (multiple) 'EUICCResponse' as defined in [5]
    }

    ErrorResult ::= SEQUENCE {
        bppCommandId BppCommandId,
        errorReason ErrorReason,
        simaResponse OCTET STRING OPTIONAL -- contains (multiple) 'EUICCResponse' as defined in [5]
    }

    BppCommandId ::= INTEGER {
        initialiseSecureChannel(0),
        configureISDP(1),
        storeMetadata(2),
        storeMetadata2(3),
        replaceSessionKeys(4),
        loadProfileElements(5)
    }

    ErrorReason ::= INTEGER {
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
    }
    */

    // profileInstallationResultData [39] ProfileInstallationResultData
    result_data_offset = bertlv_get_tl_length(pir, NULL);
    if (bertlv_get_tag(pir + result_data_offset) != 0xBF27) {
        MSG_ERR("Could not found profileInstallationResultData! \n");
        return RT_ERROR;
    }
    element_len = bertlv_get_tlv_length(pir + result_data_offset);
    MSG_INFO_ARRAY("ProfileInstallationResultData: ", pir + result_data_offset, element_len);

    // get ProfileInstallationResultData value
    result_data_offset += bertlv_get_tl_length(pir + result_data_offset, &element_len);
    
    // find notificationMetadata
    iccid_offset = bertlv_find_tag(pir + result_data_offset, element_len, 0xBF2F, 1);
    if (final_result_offset == BERTLV_INVALID_OFFSET) {
        MSG_ERR("Could not found notificationMetadata! \n");
        return RT_ERROR;
    }
    iccid_offset += bertlv_get_tl_length(pir + result_data_offset + iccid_offset, &notify_metadata_len);

    // find notificationMetadata
    iccid_offset += bertlv_find_tag(pir + result_data_offset + iccid_offset, notify_metadata_len, 0x5A, 1);
    if (final_result_offset == BERTLV_INVALID_OFFSET) {
        MSG_ERR("Could not found iccid! \n");
        return RT_ERROR;
    }
    iccid_offset += bertlv_get_tl_length(pir + result_data_offset + iccid_offset, &notify_metadata_len);
    MSG_DUMP_ARRAY("ICCID: ", pir + result_data_offset + iccid_offset, notify_metadata_len);
    swap_nibble(pir + result_data_offset + iccid_offset, notify_metadata_len);
    ret = bytes2hexstring(pir + result_data_offset + iccid_offset, notify_metadata_len, iccid);
    if (ret != RT_SUCCESS) {
        MSG_ERR("iccid convert error! \n");
        return ret;
    }

    // find finalResult
    final_result_offset = bertlv_find_tag(pir + result_data_offset, element_len, 0xA2, 1);
    if (final_result_offset == BERTLV_INVALID_OFFSET) {
        MSG_ERR("Could not found finalResult! \n");
        return RT_ERROR;
    }
    final_result_offset += result_data_offset;

    final_result_offset += bertlv_get_tl_length(pir + final_result_offset, NULL);

    tag = bertlv_get_tag(pir + final_result_offset, NULL);
    if (tag == 0xA0) {
        // successResult SuccessResult
        MSG_INFO("Profile intatll success. \n");
        *bppcid = 0;
        *error = 0;
        return RT_SUCCESS;
    } else if (tag == 0xA1) {
        // errorResult ErrorResult
        final_result_offset += bertlv_get_tl_length(pir + final_result_offset, &element_len);

        // bppCommandId BppCommandId
        bpp_id_off = bertlv_find_tag(pir + final_result_offset, element_len, 0x80, 1);
        if (final_result_offset == BERTLV_INVALID_OFFSET) {
            MSG_ERR("Profile intatll failed. bppCommandId not found! \n");
            return RT_ERROR;
        }
        *bppcid = (uint8_t)bertlv_get_integer(pir + final_result_offset + bpp_id_off, NULL);

        // errorReason ErrorReason
        error_reson_off = bertlv_find_tag(pir + final_result_offset, element_len, 0x81, 1);
        if (final_result_offset == BERTLV_INVALID_OFFSET) {
            MSG_ERR("Profile intatll failed. bppCommandId not found! \n");
            return RT_ERROR;
        }
        *error = (uint8_t)bertlv_get_integer(pir + final_result_offset + error_reson_off, NULL);

        MSG_ERR("Profile intatll failed. bppCommandId: %d, errorReason: %d. \n", *bppcid, *error);

        // simaResponse
        euicc_response_off = bertlv_find_tag(pir + final_result_offset, element_len, 0x82, 1);
        if (final_result_offset == BERTLV_INVALID_OFFSET) {
            MSG_ERR("Profile intatll failed. simaResponse not found! \n");
            return RT_ERROR;
        }
        element_len = bertlv_get_tlv_length(pir + final_result_offset + euicc_response_off);
        MSG_DUMP_ARRAY("simaResponse: \n", pir + final_result_offset + euicc_response_off, element_len);
        return RT_SUCCESS;
    } else {
        MSG_ERR("Profile intatll failed. Resutl not found! \n");
        return RT_ERROR;
    }
}

int lpa_download_profile(const char *ac, const char *cc, char iccid[21], uint8_t *server_url)
{
    int ret = RT_ERR_UNKNOWN_ERROR;
    bool need_cc;
    char *smdp_addr = NULL;
    char *mid = NULL;
    uint16_t smdp_addr_start, smdp_addr_len;
    uint16_t matching_id_start, matching_id_len;
    char *buf1 = NULL;
    int buf1_len;
    uint8_t *buf2 = NULL;
    uint16_t buf2_len = BUFFER_SIZE;
    uint8_t bppcid, error;
    uint8_t channel = 0xFF;

    linux_mutex_lock(g_lpa_mutex);

    if (open_channel(&channel) != RT_SUCCESS) {
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        goto end;
    }
    buf1 = malloc(BUFFER_SIZE);
    if( buf1 == NULL) {
        goto end;
    }
    buf2 = malloc(BUFFER_SIZE);
    if(buf2 == NULL) {
        goto end;
    }

    memset(buf1,0x00,BUFFER_SIZE);
    memset(buf2,0x00,BUFFER_SIZE);
    lpa_https_set_url(server_url);
    MSG_DBG("AC:%s\n", ac);
    RT_CHECK(check_ac(ac, &smdp_addr_start, &smdp_addr_len, &matching_id_start, &matching_id_len, &need_cc));

    MSG_DBG("smdp_addr_len: %d\n", smdp_addr_len);
    smdp_addr = calloc(1, smdp_addr_len + 1);
    RT_CHECK_GO(smdp_addr, RT_ERR_OUT_OF_MEMORY, end);
    memcpy(smdp_addr, ac+smdp_addr_start, smdp_addr_len);

    MSG_DBG("matching_id_len: %d\n", matching_id_len);
    mid = calloc(1, matching_id_len + 1);
    RT_CHECK_GO(mid, RT_ERR_OUT_OF_MEMORY, end);
    memcpy(mid, ac+matching_id_start, matching_id_len);
    mid[matching_id_len] = '\0';
    MSG_DBG("AC_token:     %s\n", mid);
    MSG_DBG("SMDP_ADDRESS: %s\n", smdp_addr);
    MSG_DBG("Need CC:      %s\n", need_cc ? "Yes" : "No");
    if (need_cc) {
        RT_CHECK_GO(cc, RT_ERR_NEED_CONFIRMATION_CODE, end);
    } else {
        cc = NULL;
    }

    buf1_len = BUFFER_SIZE;
    ret = initiate_authentication(smdp_addr, (char *)buf1, &buf1_len, channel);
    if (ret != RT_SUCCESS) {
        MSG_ERR("initiate_authentication error response:\n%s\n", buf1);
    } else {
        MSG_DBG("initiate_authentication response:\n%s\n", buf1);
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    buf2_len = BUFFER_SIZE;
    ret = authenticate_server(mid, (char *)buf1, buf2, &buf2_len, channel);
    MSG_DUMP_ARRAY("authenticate_server:\n", buf2, buf2_len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    buf1_len = BUFFER_SIZE;
    ret = authenticate_client(smdp_addr, buf2, buf2_len, buf1, &buf1_len);
    if (ret != RT_SUCCESS) {
        MSG_ERR("authenticate_client error response:\n%s\n", buf1);
    } else {
        MSG_DBG("authenticate_client response:\n%s\n", buf1);
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    buf2_len = BUFFER_SIZE;
    ret = prepare_download(buf1, cc, buf2, &buf2_len, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DUMP_ARRAY("prepare_download:\n", buf2, buf2_len);

    buf1_len = BUFFER_SIZE;
    ret = get_bound_profile_package(smdp_addr, buf2, buf2_len, buf1, &buf1_len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DBG("get_bound_profile_package response:\n%s\n", buf1);

    buf2_len = BUFFER_SIZE;
    ret = load_bound_profile_package(smdp_addr, buf1, buf2, &buf2_len, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DUMP_ARRAY("load_bound_profile_package:\n", buf2, buf2_len);

    ret = parse_install_profile_result(buf2, buf2_len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    ret = process_bpp_rsp(buf2, buf2_len, iccid, &bppcid, &error);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    MSG_DBG("iccid: %s, bppcid: %d, error: %d\n", iccid, bppcid, error);
    if ((bppcid != 0) || (error != 0) ){
        ret = (((uint16_t)bppcid) << 8) + error;
    }

end:
    close_session();

    if (ret != RT_ERR_APDU_OPEN_CHANNEL_FAIL) {
        close_channel(channel);
    }

    if (smdp_addr != NULL)  { free(smdp_addr);}
    if (mid != NULL)        { free(mid);}
    if (buf1 != NULL)       { free(buf1);}
    if (buf2 != NULL)       { free(buf2);}

    linux_mutex_unlock(g_lpa_mutex);

    return ret;
}

int lpa_load_customized_data(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint8_t channel = 0xFF;
    int ret = RT_SUCCESS;

    linux_mutex_lock(g_lpa_mutex);

    if (open_channel(&channel) != RT_SUCCESS) {
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        goto end;
    }
    ret = load_customized_data(data, data_len, rsp, rsp_len, channel);
    close_channel(channel);

end:

    linux_mutex_unlock(g_lpa_mutex);

    return ret;
}

