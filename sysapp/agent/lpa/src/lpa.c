
#include <stdio.h>

#include "rt_os.h"
#include "lpa.h"
#include "lpa_config.h"
#include "lpdd.h"
#include "luid.h"
#include "convert.h"
#include "lpa_https.h"
#include "lpa_error_codes.h"
#include "ProfileInfoListResponse.h"
#include "ProfileInstallationResult.h"
#include "MoreEIDOperateResponse.h"

#define BUFFER_SIZE                10*1024

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
    int ret = RT_SUCCESS;
    uint8_t buf[500] = {0};
    asn_dec_rval_t dc;
    uint16_t size = sizeof(buf);
    int i;
    int num =0;
    EIDInfo_t **p = NULL;
    uint8_t channel = 0xFF;
    MoreEIDOperateResponse_t *rsp = NULL;

    linux_mutex_lock(g_lpa_mutex);

    if (open_channel(&channel) != RT_SUCCESS) {
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        goto end;
    }

    get_eid_list(buf, &size, channel);
    MSG_DUMP_ARRAY("get eid list:\n", buf, size);
    dc = ber_decode(NULL, &asn_DEF_MoreEIDOperateResponse, (void **)&rsp, buf, size);
    if (dc.code != RC_OK) {
        MSG_ERR("Broken ProfileInfoListResponse decoding at byte %ld\n", (long)dc.consumed);
        ret = RT_ERR_ASN1_DECODE_FAIL;
        goto end;
    }
    if (rsp->present != MoreEIDOperateResponse__moreEIDOperateResult_ok) {
        ret = RT_ERR_UNKNOWN_ERROR;
        goto end;
    }
    num = rsp->choice.eidListinfo.list.count;
    p = (EIDInfo_t **)(rsp->choice.eidListinfo.list.array);
    MSG_INFO("count: %d, size: %d, present: %d\n", num,(p[0])->eidValue.size, rsp->present);
    if (p != NULL) {
        for (i = 0; i < num; i++) {
            //memcpy(eid,(p[i])->eidValue.buf, (p[i])->eidValue.size);
            bytes2hexstring((p[i])->eidValue.buf,(p[i])->eidValue.size,eid_list[i]);
            MSG_INFO("eid%d: %s\n", i,eid_list[i]);
        }
    }

end:

    ASN_STRUCT_FREE(asn_DEF_MoreEIDOperateResponse, rsp);

    if (ret != RT_ERR_APDU_OPEN_CHANNEL_FAIL) {
        close_channel(channel);
    }

    linux_mutex_unlock(g_lpa_mutex);

    return ret;
}

int lpa_get_profile_info(profile_info_t *pi, uint8_t *num, uint8_t max_num)
{
    int ret = RT_SUCCESS;
    uint8_t *buf = NULL;
    uint16_t size = 1024 * 10;
    asn_dec_rval_t dc;
    ProfileInfoListResponse_t *rsp = NULL;
    ProfileInfo_t **p = NULL;
    int i;
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

    dc = ber_decode(NULL, &asn_DEF_ProfileInfoListResponse, (void **)&rsp, buf, size);
    if (dc.code != RC_OK) {
        MSG_ERR("Broken ProfileInfoListmResponse decoding at byte %ld\n", (long)dc.consumed);
        ret = RT_ERR_ASN1_DECODE_FAIL;
        goto end;
    }
    MSG_INFO("present: %d, count: %d\n", rsp->present, rsp->choice.profileInfoListOk.list.count);

    if (rsp->present != ProfileInfoListResponse_PR_profileInfoListOk) {
        ret = RT_ERR_UNKNOWN_ERROR;
        goto end;
    }

    p = (ProfileInfo_t **)(rsp->choice.profileInfoListOk.list.array);
    *num = rsp->choice.profileInfoListOk.list.count;

    if (*num > max_num) {
        MSG_WARN("too many profile detected (%d > %d)\n", *num, max_num);
        *num = max_num;
    }

    if (pi != NULL) {
        for (i = 0; i < *num; i++) {
            memcpy(buf, (p[i])->iccid->buf, (p[i])->iccid->size);
            swap_nibble(buf, (p[i])->iccid->size);
            bytes2hexstring(buf, (p[i])->iccid->size, pi[i].iccid);
            pi[i].class = (uint8_t)*((p[i])->profileClass);
            pi[i].state = (uint8_t)*((p[i])->profileState);
        }
    }

end:

    if (buf != NULL) {
        free(buf);
    }

    ASN_STRUCT_FREE(asn_DEF_ProfileInfoListResponse, rsp);

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

static int process_bpp_rsp(const uint8_t *pir, uint16_t pir_len,
                        char *iccid, uint8_t *bppcid, uint8_t *error)
{
    int ret = RT_SUCCESS;
    asn_dec_rval_t dc;
    ProfileInstallationResult_t *res = NULL;
    Iccid_t *p_iccid = NULL;
    ProfileInstallationResultData__finalResult_PR present;

    dc = ber_decode(NULL, &asn_DEF_ProfileInstallationResult, (void **)&res, pir, pir_len);
    if (dc.code != RC_OK) {
        MSG_ERR("Broken ProfileInstallationResult decoding at byte %ld\n", (long)dc.consumed);
        ret = RT_ERR_ASN1_DECODE_FAIL;
        goto end;
    }

    p_iccid = res->profileInstallationResultData.notificationMetadata.iccid;
    MSG_DUMP_ARRAY("ICCID: ", p_iccid->buf, p_iccid->size);
    swap_nibble(p_iccid->buf, p_iccid->size);
    ret = bytes2hexstring(p_iccid->buf, p_iccid->size, iccid);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    present = res->profileInstallationResultData.finalResult.present;
    MSG_INFO("present: %d\n", present);

    if (present == ProfileInstallationResultData__finalResult_PR_successResult) {
        *bppcid = 0;
        *error = 0;
    } else if (present == ProfileInstallationResultData__finalResult_PR_errorResult) {
        ErrorResult_t *er = NULL;
        er = &(res->profileInstallationResultData.finalResult.choice.errorResult);
        *bppcid = er->bppCommandId;
        *error  = er->errorReason;
    }

end:
    ASN_STRUCT_FREE(asn_DEF_ProfileInstallationResult, res);

    return ret;
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
    mid[matching_id_len + 1] = '\0';
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

