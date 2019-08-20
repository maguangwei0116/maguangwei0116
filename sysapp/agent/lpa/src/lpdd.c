#include "lpdd.h"
#include "lpa_config.h"
#include "apdu.h"
#include "converter.h"
#include "base64.h"
#include "https.h"

#include "ListNotificationRequest.h"
#include "RetrieveNotificationsListRequest.h"
#include "NotificationSentRequest.h"
#include "GetEuiccChallengeRequest.h"
#include "GetEuiccInfo1Request.h"
#include "GetEuiccInfo2Request.h"
#include "CtxParams1.h"
#include "AuthenticateServerRequest.h"
#include "AuthenticateServerResponse.h"
#include "GetRatRequest.h"
#include "PrepareDownloadRequest.h"
#include "BoundProfilePackage.h"
#include "ProfileInfoListRequest.h"
#include "CancelSessionRequest.h"
#include "LoadCRLRequest.h"

#include "cJSON.h"

#include <openssl/sha.h>

static uint8_t g_buf[1024*10];
static uint16_t g_buf_size;

static char g_transaction_id[33] = {0};

int encode_cb(const void *buffer, size_t size, void *app_key)
{
    memcpy(g_buf + g_buf_size, buffer, size);
    g_buf_size += size;

    return 0;
}

void clean_cb_data(void)
{
    g_buf_size = 0;
}

uint8_t *get_cb_data(void)
{
    return g_buf;
}

uint16_t get_cb_size(void)
{
    return g_buf_size;
}

int list_notification(notification_t ne, uint8_t *out, uint16_t *out_size)
{
    asn_enc_rval_t ec;
    uint8_t bit_string[1] = {0};
    ListNotificationRequest_t req = {0};
    NotificationEvent_t req_ne = {0};

    if (ne == NE_INSTALL) {
        bit_string[0] = 0x80;
        req_ne.bits_unused = 7;
    } else if (ne == NE_ENABLE) {
        bit_string[0] = 0x40;
        req_ne.bits_unused = 6;
    } else if (ne == NE_DISABLE) {
        bit_string[0] = 0x20;
        req_ne.bits_unused = 5;
    } else if (ne == NE_DELETE) {
        bit_string[0] = 0x10;
        req_ne.bits_unused = 4;
    }

    if (ne == NE_ALL) {
        // Do nothing
    } else {
        req_ne.buf = bit_string;
        req_ne.size = 1;
        req.profileManagementOperation = &req_ne;
    }

    *out_size = 0;
    clean_cb_data();
    ec = der_encode(&asn_DEF_ListNotificationRequest, &req, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("ListNotificationRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size));
    //*out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int load_crl(const uint8_t *crl, uint16_t crl_size, uint8_t *out, uint16_t *out_size)
{
    // asn_enc_rval_t ec;
    // LoadCRLRequest_t req = {0};

    // *out_size = 0;
    // clean_cb_data();
    // ec = der_encode(&asn_DEF_LoadCRLRequest, &req, encode_cb, NULL);
    // MSG_ERR("ec.encoded: %d\n", (int)ec.encoded);
    // if(ec.encoded == -1) {
    //     MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
    //     return RT_ERR_ASN1_ENCODE_FAIL;
    // }
    // MSG_INFO_ARRAY("LoadCRLRequest: ", get_cb_data(), get_cb_size());
    // RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size));
    // *out_size -= 2;  // Remove sw 9000

    // Impl for crl prepared by caller
    RT_CHECK(cmd_store_data(crl, crl_size, out, out_size));
    *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int retrieve_notification_list(notification_t ne, long *seq, uint8_t *out, uint16_t *out_size)
{
    int ret = RT_SUCCESS;
    asn_enc_rval_t ec;
    uint8_t bit_string[1] = {0};
    RetrieveNotificationsListRequest_t req = {0};

    if (ne == NE_ALL) {
        // Do nothing
    } else {
        req.searchCriteria = calloc(1, sizeof(struct ProfileInfoListRequest__searchCriteria));
        RT_CHECK_GO(req.searchCriteria, RT_ERR_OUT_OF_MEMORY, end);
        if (ne == NE_SEQ_NUM) {
            req.searchCriteria->present = RetrieveNotificationsListRequest__searchCriteria_PR_seqNumber;
            req.searchCriteria->choice.seqNumber = *seq;
        } else {
            req.searchCriteria->present = RetrieveNotificationsListRequest__searchCriteria_PR_profileManagementOperation;
            if (ne == NE_INSTALL) {
                bit_string[0] = 0x80;
                req.searchCriteria->choice.profileManagementOperation.bits_unused = 7;
            } else if (ne == NE_ENABLE) {
                bit_string[0] = 0x40;
                req.searchCriteria->choice.profileManagementOperation.bits_unused = 6;
            } else if (ne == NE_DISABLE) {
                bit_string[0] = 0x20;
                req.searchCriteria->choice.profileManagementOperation.bits_unused = 5;
            } else if (ne == NE_DELETE) {
                bit_string[0] = 0x10;
                req.searchCriteria->choice.profileManagementOperation.bits_unused = 4;
            }
            req.searchCriteria->choice.profileManagementOperation.buf = bit_string;
            req.searchCriteria->choice.profileManagementOperation.size = 1;
        }
    }

    *out_size = 0;
    clean_cb_data();
    ec = der_encode(&asn_DEF_RetrieveNotificationsListRequest, &req, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        ret = RT_ERR_ASN1_ENCODE_FAIL;
        goto end;
    }
    MSG_INFO_ARRAY("RetrieveNotificationsListRequest: ", get_cb_data(), get_cb_size());
    ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*out_size -= 2;  // Remove sw 9000

end:
    if (req.searchCriteria != NULL) {
        free(req.searchCriteria);
    }
    return ret;
}

int remove_notification_from_list(long seq, uint8_t *out, uint16_t *out_size)
{
    asn_enc_rval_t ec;
    NotificationSentRequest_t req = {0};

    req.seqNumber = seq;

    *out_size = 0;
    clean_cb_data();
    ec = der_encode(&asn_DEF_NotificationSentRequest, &req, encode_cb, NULL);
    MSG_ERR("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("ListNotificationRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size));
    *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int get_euicc_info(uint8_t *info1, uint16_t *size1, uint8_t *info2, uint16_t *size2)
{
    int ret = RT_SUCCESS;
    asn_enc_rval_t ec;
    GetEuiccInfo1Request_t *req1 = NULL;
    GetEuiccInfo2Request_t *req2 = NULL;

    if ((info1 == NULL) || (size1 == NULL) ) {
        ret = RT_ERR_NULL_POINTER;
        goto end;
    }

    clean_cb_data();
    ec = der_encode(&asn_DEF_GetEuiccInfo1Request, req1, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        ASN_STRUCT_FREE(asn_DEF_GetEuiccInfo1Request, req1);
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    ASN_STRUCT_FREE(asn_DEF_GetEuiccInfo1Request, req1);
    MSG_INFO_ARRAY("GetEuiccInfo1Request: ", get_cb_data(), get_cb_size());

    ret = cmd_store_data(get_cb_data(), get_cb_size(), info1, size1);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size1 -= 2;  // Remove sw 9000

    if ((info2 != NULL) && (size2 != NULL) ) {
        clean_cb_data();
        ec = der_encode(&asn_DEF_GetEuiccInfo2Request, req2, encode_cb, NULL);
        if(ec.encoded == -1) {
            MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
            ASN_STRUCT_FREE(asn_DEF_GetEuiccInfo1Request, req2);
            return RT_ERR_ASN1_ENCODE_FAIL;
        }
        ASN_STRUCT_FREE(asn_DEF_GetEuiccInfo1Request, req2);
        MSG_INFO_ARRAY("GetEuiccInfo1Request: ", get_cb_data(), get_cb_size());

        ret = cmd_store_data(get_cb_data(), get_cb_size(), info2, size2);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
        //*size2 -= 2;  // Remove sw 9000
    }
end:
    ASN_STRUCT_FREE(asn_DEF_GetEuiccInfo1Request, req1);
    ASN_STRUCT_FREE(asn_DEF_GetEuiccInfo2Request, req2);
    return ret;
}

int get_euicc_challenge(uint8_t challenge[16])
{
    int ret = RT_SUCCESS;
    uint16_t rlen;
    uint8_t rsp[23];  // 8F2E128010-CHALLENAGE-9000

    asn_enc_rval_t ec;
    GetEuiccChallengeRequest_t *req = NULL;

    clean_cb_data();
    ec = der_encode(&asn_DEF_GetEuiccChallengeRequest, req, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        ASN_STRUCT_FREE(asn_DEF_GetEuiccChallengeRequest, req);
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    ASN_STRUCT_FREE(asn_DEF_GetEuiccChallengeRequest, req);

    MSG_INFO_ARRAY("GetEuiccChallengeRequest: ", get_cb_data(), get_cb_size());
    // RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), rsp, &rlen));
    ret = cmd_store_data(get_cb_data(), get_cb_size(), rsp, &rlen);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    memcpy(challenge, rsp + 5, 16);

end:
    ASN_STRUCT_FREE(asn_DEF_GetEuiccChallengeRequest, req);
    return ret;
}

int get_rat(uint8_t *rat, uint16_t *size)
{
    int ret = RT_SUCCESS;
    asn_enc_rval_t ec;
    GetRatRequest_t *req = NULL;

    clean_cb_data();
    ec = der_encode(&asn_DEF_GetRatRequest, req, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        ASN_STRUCT_FREE(asn_DEF_GetRatRequest, req);
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    ASN_STRUCT_FREE(asn_DEF_GetRatRequest, req);

    MSG_INFO_ARRAY("GetEuiccInfo1Request: ", get_cb_data(), get_cb_size());
    // RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), rat, size));
    ret = cmd_store_data(get_cb_data(), get_cb_size(), rat, size);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size -= 2;  // Remove sw 9000

end:
    ASN_STRUCT_FREE(asn_DEF_GetRatRequest, req);
    return ret;
}

int cancel_session(const uint8_t *tid, uint8_t tid_size, uint8_t reason, uint8_t *csr, uint16_t *size)
{
    int ret = RT_SUCCESS;
    asn_enc_rval_t ec;
    CancelSessionRequest_t *req = NULL;

    req = calloc(1, sizeof(CancelSessionRequest_t));
    RT_CHECK_GO(req, RT_ERR_OUT_OF_MEMORY, end);

    ret = OCTET_STRING_fromBuf(&req->transactionId, (const char *)tid, tid_size);
    RT_CHECK_GO(ret == 0, RT_ERR_OUT_OF_MEMORY, end);

    req->reason = reason;

    clean_cb_data();
    ec = der_encode(&asn_DEF_CancelSessionRequest, req, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        ret = RT_ERR_ASN1_ENCODE_FAIL;
        goto end;
    }
    MSG_DUMP_ARRAY("CancelSessionRequest: ", get_cb_data(), get_cb_size());

    ret = cmd_store_data(get_cb_data(), get_cb_size(), csr, size);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size -= 2;  // Remove sw 9000

end:
    ASN_STRUCT_FREE(asn_DEF_CancelSessionRequest, req);
    return ret;
}

static int get_status_codes(const char *in, char *sc, char *rc)
{
    int ret = RT_SUCCESS;
    const char *p1 = NULL;
    const char *p2 = NULL;

    p1 = strstr(in, "\"status\"");  // Find "status"
    RT_CHECK_GO(p1, RT_ERR_HTTPS_SMDP_ERROR, end);

    p1 += 8;  // Skip "status"

    // Now p1 points to right after "status"
    p2 = strstr(p1, "Executed-Success");  // Check if success
    if (p2 != NULL) {  // Get it!
        *sc = '\0';
        *rc = '\0';
        ret = RT_SUCCESS;
        goto end;
    }

    // Failed
    p2 = strstr(p1, "\"subjectCode\""); // Find "subjectCode"
    RT_CHECK_GO(p2, RT_ERR_HTTPS_SMDP_ERROR, end);

    p2 += 13;  // Skip "subjectCode"
    p1 = strstr(p2, "\"");  // Find the first " after :
    RT_CHECK_GO(p1, RT_ERR_HTTPS_SMDP_ERROR, end);

    p1 += 1;  // Skip "
    p2 = strstr(p1, "\"");  // Find the secoond " after :
    RT_CHECK_GO(p2, RT_ERR_HTTPS_SMDP_ERROR, end);
    MSG_INFO("p1: %s\np2: %s\n", p1, p2);

    // Now we get subjectCode, and p1 points to the start and p2 points to the end of the subjectCode
    memcpy(sc, p1, p2 - p1);
    sc[p2-p1] = '\0';
    MSG_INFO("sc: %s\n", sc);

    p2 = strstr(p2, "\"reasonCode\""); // Find "reasonCode"
    RT_CHECK_GO(p2, RT_ERR_HTTPS_SMDP_ERROR, end);

    p2 += 12;  // Skip "reasonCode"
    p1 = strstr(p2, "\"");  // Find the first " after :
    RT_CHECK_GO(p1, RT_ERR_HTTPS_SMDP_ERROR, end);

    p1 += 1;  // Skipp "
    p2 = strstr(p1, "\"");  // Find the secoond " after :
    RT_CHECK_GO(p2, RT_ERR_HTTPS_SMDP_ERROR, end);
    MSG_INFO("p1: %s\np2: %s\n", p1, p2);

    // Now we get subjectCode, and p1 points to the start and p2 points to the end of the reasonCode
    memcpy(rc, p1, p2 - p1);
    rc[p2-p1] = '\0';
    MSG_INFO("rc: %s\n", rc);

    ret = 1;

end:
    return ret;
}

int initiate_authentication(const char *smdp_addr, char *auth_data, int *size)
{
    int ret = RT_SUCCESS;
    char *data = NULL;
    cJSON *content = NULL;
    uint8_t tmp[1024] = {0};
    uint16_t tmp_size = 0;

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);

    RT_CHECK_GO((ret = get_euicc_challenge(tmp)) == RT_SUCCESS, ret, end);
    MSG_INFO_ARRAY("euiccChallenge: ", tmp, 16);
    RT_CHECK_GO((ret = rt_base64_encode(tmp, 16, (char *)g_buf)) == RT_SUCCESS, ret, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "euiccChallenge", (char *)g_buf), RT_ERR_CJSON_ERROR, end);

    RT_CHECK_GO((ret = get_euicc_info(tmp, &tmp_size, NULL, NULL)) == RT_SUCCESS, ret, end);
    MSG_INFO_ARRAY("euiccInfo1: ", tmp, tmp_size);
    RT_CHECK_GO((ret = rt_base64_encode(tmp, tmp_size, (char *)g_buf)) == RT_SUCCESS, ret, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "euiccInfo1", (char *)g_buf), RT_ERR_CJSON_ERROR, end);

    RT_CHECK_GO(cJSON_AddStringToObject(content, "smdpAddress", smdp_addr), RT_ERR_CJSON_ERROR, end);

    data = (char *)g_buf;
    ret = cJSON_PrintPreallocated(content, data, sizeof(g_buf), 1);
    RT_CHECK_GO(ret == 1, RT_ERR_CJSON_ERROR, end);
    MSG_INFO("content:\n%s\n", data);

    ret = lpa_https_post(smdp_addr, API_INITIATE_AUTHENTICATION, data, auth_data, size);
    // RT_CHECK_GO(ret == 200, RT_ERR_HTTPS_POST_FAIL, end);
    if (ret == 200) {
        ret = RT_SUCCESS;
    } else {
        MSG_ERR("lpa_https_post: %d\n", ret);
        // ret = RT_ERR_HTTPS_POST_FAIL;
        ret = ret < 0 ? ret : RT_ERR_HTTPS_POST_FAIL;
        goto end;
    }

    ret = get_status_codes(auth_data, (char *)tmp, (char *)g_buf);
    if (ret == 1) {
        MSG_ERR("subjectCode: %s, reasonCode: %s\n", tmp, g_buf);
        ret = RT_ERR_INITIATE_AUTHENTICATION;
        goto end;
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

end:
    cJSON_Delete(content);
    return ret;
}

int gen_ctx_params1(const char *matching_id)
{
    int ret = RT_SUCCESS;
    asn_enc_rval_t ec;
    CtxParams1_t ctx_params1 = {0};

    UTF8String_t mid;
    Octet4_t gsm;
    Octet4_t utran;
    Octet4_t cdma1x;
    Octet4_t cdmahrpd;
    Octet4_t cdmaehrpd;
    Octet4_t eutran;
    Octet4_t contract_less;
    Octet4_t rsp_ctl;

    uint8_t arr_tac[]           = {0x00, 0x00, 0x00, 0x00};
    uint8_t arr_gsm[]           = {0x05, 0x00, 0x00};
    uint8_t arr_utran[]         = {0x08, 0x00, 0x00};
    uint8_t arr_cdma1x[]        = {0x01, 0x00, 0x00};  // Not support
    uint8_t arr_cdmahrpd[]      = {0x01, 0x00, 0x00};  // Not support
    uint8_t arr_cdmaehrpd[]     = {0x02, 0x00, 0x00};  // Not support
    uint8_t arr_eutran[]        = {0x02, 0x00, 0x00};
    uint8_t arr_contract_less[] = {0x09, 0x00, 0x00};
    uint8_t arr_rsp_ctl[]       = {0x02, 0x00, 0x00};

    mid.buf         = (uint8_t *)matching_id;
    mid.size        = strlen(matching_id);

    gsm.buf         = arr_gsm;
    gsm.size        = sizeof(arr_gsm);

    utran.buf       = arr_utran;
    utran.size      = sizeof(arr_utran);

    cdma1x.buf      = arr_cdma1x;
    cdma1x.size     = sizeof(arr_cdma1x);

    cdmahrpd.buf        = arr_cdmahrpd;
    cdmahrpd.size       = sizeof(arr_cdmahrpd);

    cdmaehrpd.buf       = arr_cdmaehrpd;
    cdmaehrpd.size      = sizeof(arr_cdmaehrpd);

    eutran.buf          = arr_eutran;
    eutran.size         = sizeof(arr_eutran);

    contract_less.buf   = arr_contract_less;
    contract_less.size  = sizeof(arr_contract_less);

    rsp_ctl.buf         = arr_rsp_ctl;
    rsp_ctl.size        = sizeof(arr_rsp_ctl);

    ctx_params1.present = CtxParams1_PR_ctxParamsForCommonAuthentication;

    ctx_params1.choice.ctxParamsForCommonAuthentication.matchingId = &mid;

    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.tac.buf = arr_tac;
    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.tac.size = sizeof(arr_tac);

    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.deviceCapabilities.gsmSupportedRelease = &gsm;
    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.deviceCapabilities.utranSupportedRelease = &utran;
    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.deviceCapabilities.cdma2000onexSupportedRelease = &cdma1x;
    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.deviceCapabilities.cdma2000hrpdSupportedRelease = &cdmahrpd;
    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.deviceCapabilities.cdma2000ehrpdSupportedRelease = &cdmaehrpd;
    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.deviceCapabilities.eutranSupportedRelease = &eutran;
    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.deviceCapabilities.contactlessSupportedRelease = &contract_less;
    ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.deviceCapabilities.rspCrlSupportedVersion = &rsp_ctl;

    // ctx_params1.choice.ctxParamsForCommonAuthentication.deviceInfo.imei = NULL;

    clean_cb_data();
    ec = der_encode(&asn_DEF_CtxParams1, &ctx_params1, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        g_buf[0] = '\0';
        clean_cb_data();
        return RT_ERR_ASN1_ENCODE_FAIL;
    }

    return ret;
}

static int get_asn1_from_json(const cJSON *json, const char *key,
                    const struct asn_TYPE_descriptor_s *type_descriptor, void **req)
{
    int ret = RT_SUCCESS;
    uint16_t len;
    char *b64_str = NULL;
    uint8_t *asn1 = NULL;
    asn_dec_rval_t dc;

    b64_str = cJSON_GetObjectItem(json, key)->valuestring;
    RT_CHECK_GO(b64_str, RT_ERR_CJSON_ERROR, end);
    MSG_INFO("%s: %s\n", key, b64_str);

    asn1 = malloc(strlen(b64_str));
    RT_CHECK_GO(asn1, RT_ERR_OUT_OF_MEMORY, end);

    ret = rt_base64_decode(b64_str, asn1, &len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_INFO_ARRAY("asn1: ", asn1, len);

    dc = ber_decode(NULL, type_descriptor, req, asn1, len);
    if (dc.code != RC_OK) {
        MSG_ERR("Broken get_asn1_from_json decoding at byte %ld\n", (long)dc.consumed);
        goto end;
    }

end:
    if (asn1 != NULL) {
        free(asn1);
    }
    return ret;
}

static int get_signature_from_json(const cJSON *json, const char *key, void **req)
{
    int ret = RT_SUCCESS;
    uint16_t len;
    char *b64_str = NULL;
    uint8_t *asn1 = NULL;

    b64_str = cJSON_GetObjectItem(json, key)->valuestring;
    RT_CHECK_GO(b64_str, RT_ERR_CJSON_ERROR, end);
    MSG_INFO("%s: %s\n", key, b64_str);

    asn1 = malloc(strlen(b64_str));
    RT_CHECK_GO(asn1, RT_ERR_OUT_OF_MEMORY, end);

    ret = rt_base64_decode(b64_str, asn1, &len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_INFO_ARRAY("asn1: ", asn1, len);

    if ((asn1[0] = 0x5F) && (asn1[1] = 0x37)) {
        OCTET_STRING_fromBuf(*req, (char *)(asn1 + 3), asn1[2]);
    } else {
        MSG_ERR("Broken serverSignature1 decoding at byte 0\n");
        ret = RT_ERR_ASN1_DECODE_FAIL;
        goto end;
    }

end:
    if (asn1 != NULL) {
        free(asn1);
    }

    return ret;
}

static int get_transaction_id(TransactionId_t *tid, uint8_t *transaction_id, uint8_t *size)
{
    asn_enc_rval_t ec;
    clean_cb_data();
    // asn_fprint(stdout, &asn_DEF_TransactionId, tid);
    ec = der_encode(&asn_DEF_TransactionId, tid, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    *size = g_buf[1];
    memcpy(transaction_id, &g_buf[2], *size);

    return RT_SUCCESS;
}

static int get_cc_hash(const char *cc, const uint8_t *transaction_id, uint8_t id_size, uint8_t *hash)
{
    SHA256_CTX ctx;
    uint8_t tmp[SHA256_DIGEST_LENGTH + 16] = {0};  // 16 for transactionId

    RT_CHECK_EQ(SHA256_Init(&ctx), 1);
    RT_CHECK_EQ(SHA256_Update(&ctx, cc, strlen(cc)), 1);
    RT_CHECK_EQ(SHA256_Final(tmp, &ctx), 1);
    MSG_INFO_ARRAY("Hash: ", tmp, sizeof(tmp));

    memcpy(&tmp[SHA256_DIGEST_LENGTH], transaction_id, id_size);
    MSG_INFO_ARRAY("Hash: ", tmp, sizeof(tmp));

    RT_CHECK_EQ(SHA256_Init(&ctx), 1);
    RT_CHECK_EQ(SHA256_Update(&ctx, tmp, SHA256_DIGEST_LENGTH + id_size), 1);
    RT_CHECK_EQ(SHA256_Final(hash, &ctx), 1);
    MSG_INFO_ARRAY("Hash: ", hash, SHA256_DIGEST_LENGTH);

    return RT_SUCCESS;
}

int authenticate_server(const char *matching_id, const char *auth_data,
                        uint8_t *response, uint16_t *size /* out */)
{
    int ret = RT_SUCCESS;
    void *p = NULL;
    cJSON *content = NULL;

    asn_enc_rval_t ec;
    asn_dec_rval_t dc;
    AuthenticateServerRequest_t *req = NULL;
    cmd_manage_channel(CLOSE_CHANNEL);
    req = calloc(1, sizeof(AuthenticateServerRequest_t));
    RT_CHECK_GO(req, RT_ERR_OUT_OF_MEMORY, end);

    content = cJSON_Parse(auth_data);
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);

    p = &(req->serverSigned1);
    ret = get_asn1_from_json(content, "serverSigned1", &asn_DEF_ServerSigned1, (void **)&p);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    // asn_fprint(stdout, &asn_DEF_ServerSigned1, p);

    ret = get_transaction_id((TransactionId_t *)&(req->serverSigned1.transactionId), g_buf, (uint8_t *)&g_buf_size);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    ret = bytes2hexstring(get_cb_data(), get_cb_size(), g_transaction_id);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    p = &(req->serverSignature1);
    ret = get_signature_from_json(content, "serverSignature1", (void **)&p);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    p = &(req->euiccCiPKIdToBeUsed);
    ret = get_asn1_from_json(content, "euiccCiPKIdToBeUsed", &asn_DEF_SubjectKeyIdentifier, (void **)&p);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    // asn_fprint(stdout, &asn_DEF_SubjectKeyIdentifier, p);

    p = &(req->serverCertificate);
    ret = get_asn1_from_json(content, "serverCertificate", &asn_DEF_Certificate, (void **)&p);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    // asn_fprint(stdout, &asn_DEF_Certificate, p);


    p = &(req->ctxParams1);
    RT_CHECK(gen_ctx_params1(matching_id));  // ctxParams1 stored in g_buf
    MSG_INFO_ARRAY("ctxParams1:\n", get_cb_data(), get_cb_size());
    dc = ber_decode(NULL, &asn_DEF_CtxParams1, (void **)&p, get_cb_data(), get_cb_size());
    if (dc.code != RC_OK) {
        MSG_ERR("Broken CtxParams1 decoding at byte %ld\n", (long)dc.consumed);
        goto end;
    }

    clean_cb_data();
    ec = der_encode(&asn_DEF_AuthenticateServerRequest, req, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("AuthenticateServerRequest\n", get_cb_data(), get_cb_size());

    // RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), response, size));
    ret = cmd_store_data(get_cb_data(), get_cb_size(), response, size);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size -= 2;  // Remove sw 9000

end:
    cJSON_Delete(content);
    ASN_STRUCT_FREE(asn_DEF_AuthenticateServerRequest, req);

    return ret;
}

int authenticate_client(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size /* out */)
{
    int ret = RT_SUCCESS;
    cJSON *content = NULL;

    char *data = NULL;
    char *b64_in = NULL;
    if ((in == NULL) || (out == NULL) || (out_size == NULL)){
        return RT_ERR_NULL_POINTER;
    }
    MSG_INFO("transactionId: %s\n", g_transaction_id);

    b64_in = malloc(((in_size + 2) / 3) * 4 + 1);
    RT_CHECK_GO(b64_in, RT_ERR_OUT_OF_MEMORY, end);
    ret = rt_base64_encode(in, in_size, b64_in);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_INFO("b64_in:\n%s\n", b64_in);

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "transactionId", (char *)g_transaction_id), RT_ERR_CJSON_ERROR, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "authenticateServerResponse", b64_in), RT_ERR_CJSON_ERROR, end);

    data = (char *)g_buf;
    ret = cJSON_PrintPreallocated(content, data, sizeof(g_buf), 1);
    RT_CHECK_GO(ret == 1, RT_ERR_CJSON_ERROR, end);
    MSG_INFO("content:\n%s\n", data);

    ret = lpa_https_post(smdp_addr, API_AUTHENTICATE_CLIENT, data, out, out_size);
    RT_CHECK_GO(ret == 200, RT_ERR_HTTPS_POST_FAIL, end);

    ret = get_status_codes(out, (char *)b64_in, (char *)g_buf);
    if (ret == 1) {
        MSG_ERR("subjectCode: %s, reasonCode: %s\n", b64_in, g_buf);
        ret = RT_ERR_AUTHENTICATE_CLIENT;
        goto end;
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

end:
    if (b64_in != NULL) { free(b64_in);}
    cJSON_Delete(content);

    return ret;
}

int prepare_download(const char *req_str, const char *cc, uint8_t *out, uint16_t *out_size)
{
    int ret = RT_SUCCESS;
    cJSON *content = NULL;
    void *p = NULL;
    asn_enc_rval_t ec;
    PrepareDownloadRequest_t *req = NULL;

    uint8_t hash_cc[SHA256_DIGEST_LENGTH] = {0};

    content = cJSON_Parse(req_str);
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);

    req = calloc(1, sizeof(PrepareDownloadRequest_t));
    RT_CHECK_GO(req, RT_ERR_OUT_OF_MEMORY, end);

    p = &(req->smdpSigned2);
    ret = get_asn1_from_json(content, "smdpSigned2", &asn_DEF_SmdpSigned2, (void **)&p);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    // asn_fprint(stdout, &asn_DEF_SmdpSigned2, p);

    p = &(req->smdpSignature2);
    ret = get_signature_from_json(content, "smdpSignature2", (void **)&p);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    // xer_fprint(stdout, &asn_DEF_OCTET_STRING, p);

    if (cc != NULL) {
        ret = get_transaction_id((TransactionId_t *)&(req->smdpSigned2.transactionId), g_buf, (uint8_t *)&g_buf_size);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
        ret = get_cc_hash(cc, get_cb_data(), get_cb_size(), hash_cc);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
        req->hashCc = calloc(1, sizeof(Octet32_t));
        RT_CHECK_GO(req->hashCc, RT_ERR_OUT_OF_MEMORY, end);
        OCTET_STRING_fromBuf(req->hashCc, (char *)hash_cc, SHA256_DIGEST_LENGTH);
        // asn_fprint(stdout, &asn_DEF_OCTET_STRING, req->hashCc);
    }

    p = &(req->smdpCertificate);
    ret = get_asn1_from_json(content, "smdpCertificate", &asn_DEF_Certificate, (void **)&p);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    // xer_fprint(stdout, &asn_DEF_Certificate, p);

    clean_cb_data();
    ec = der_encode(&asn_DEF_PrepareDownloadRequest, req, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("PrepareDownloadRequest\n", get_cb_data(), get_cb_size());

    ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*out_size -= 2;  // Remove sw 9000

end:
    cJSON_Delete(content);
    ASN_STRUCT_FREE(asn_DEF_PrepareDownloadRequest, req);

    return ret;
}

int get_bound_profile_package(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size /* out */)
{
    int ret = RT_SUCCESS;
    cJSON *content = NULL;

    char *data = NULL;
    char *b64_in = NULL;
    if ((in == NULL) || (out == NULL) || (out_size == NULL)){
        return RT_ERR_NULL_POINTER;
    }
    MSG_INFO("transactionId: %s\n", g_transaction_id);

    b64_in = malloc(((in_size + 2) / 3) * 4 + 1);
    RT_CHECK_GO(b64_in, RT_ERR_OUT_OF_MEMORY, end);
    ret = rt_base64_encode(in, in_size, b64_in);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_INFO("b64_in[%d]:\n%s\n", strlen(b64_in), b64_in);

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "transactionId", (char *)g_transaction_id), RT_ERR_CJSON_ERROR, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "prepareDownloadResponse", b64_in), RT_ERR_CJSON_ERROR, end);

    data = (char *)g_buf;
    ret = cJSON_PrintPreallocated(content, data, sizeof(g_buf), 1);
    RT_CHECK_GO(ret == 1, RT_ERR_CJSON_ERROR, end);
    MSG_INFO("content:\n%s\n", data);

    ret = lpa_https_post(smdp_addr, API_GET_BOUND_PROFILE_PACKAGE, data, out, out_size);
    RT_CHECK_GO(ret == 200, RT_ERR_HTTPS_POST_FAIL, end);

    ret = get_status_codes(out, (char *)b64_in, (char *)g_buf);
    if (ret == 1) {
        MSG_ERR("subjectCode: %s, reasonCode: %s\n", b64_in, g_buf);
        ret = RT_ERR_GET_BOUND_PROFILE_PACKAGE;
        goto end;
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

end:
    if (b64_in != NULL) { free(b64_in);}
    cJSON_Delete(content);

    return ret;
}

int load_bound_profile_package(const char *smdp_addr, const char *get_bpp_rsp,
                            uint8_t *out, uint16_t *out_size /* out */)
{
    int i;
    int ret = RT_SUCCESS;
    cJSON *content = NULL;
    uint8_t *p = NULL;
    uint16_t len = 0;
    asn_enc_rval_t ec;
    BoundProfilePackage_t *bpp = NULL;
    OCTET_STRING_t *req = NULL;
    uint8_t cnt;
    uint8_t *buf;

    *out_size = 0;

    content = cJSON_Parse(get_bpp_rsp);
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);

    ret = get_asn1_from_json(content, "boundProfilePackage", &asn_DEF_BoundProfilePackage, (void **)&bpp);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    // asn_fprint(stdout, &asn_DEF_BoundProfilePackage, bpp);

    clean_cb_data();
    ec = der_encode(&asn_DEF_BoundProfilePackage, bpp, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("boundProfilePackage\n", get_cb_data(), get_cb_size());

    // ES8+ InitialiseSecureChannel
    clean_cb_data();
    g_buf_size += 5;  // Play a trick here, include the BPP TLV
    ec = der_encode(&asn_DEF_InitialiseSecureChannelRequest, &(bpp->initialiseSecureChannelRequest), encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("initialiseSecureChannelRequest\n", get_cb_data(), get_cb_size());
    ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size);  // Should only contain 9000
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    // ES8+ Configure ISDP
    clean_cb_data();
    ec = der_encode(asn_MBR_BoundProfilePackage_1[1].type, &(bpp->firstSequenceOf87), encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("firstSequenceOf87\n", get_cb_data(), get_cb_size());
    ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size);  // Should only contain 9000
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    // ES8+ Store Metadata
    clean_cb_data();
    ec = der_encode(asn_MBR_BoundProfilePackage_1[2].type, &(bpp->sequenceOf88), encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("sequenceOf88\n", get_cb_data(), get_cb_size());

    // Send sequenceOf88 TL
    buf = strchr(get_cb_data(),0x88);
    p = get_cb_data();
    len = buf-get_cb_data();  // TODO: Make it general
    MSG_INFO("len: %d\n", len);
    ret = cmd_store_data(p, len, out, out_size);  // Should only contain 9000
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    // Send sequenceOf88 Value
    cnt = bpp->sequenceOf88.list.count;
    for (i = 0; i < cnt; i++) {
        req = bpp->sequenceOf88.list.array[i];
        clean_cb_data();
        ec = der_encode(&asn_DEF_OS88, req, encode_cb, NULL);
        if(ec.encoded == -1) {
            MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
            return RT_ERR_ASN1_ENCODE_FAIL;
        }
        MSG_INFO_ARRAY("sequenceOf88TLV\n", get_cb_data(), get_cb_size());
        ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);  // Should only contain 9000
    }

    // ES8+ Replace Session Keys
    if (bpp->secondSequenceOf87 != NULL) {
        clean_cb_data();
        ec = der_encode(asn_MBR_BoundProfilePackage_1[3].type, bpp->secondSequenceOf87, encode_cb, NULL);
        if(ec.encoded == -1) {
            MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
            return RT_ERR_ASN1_ENCODE_FAIL;
        }
        MSG_INFO_ARRAY("secondSequenceOf87\n", get_cb_data(), get_cb_size());
        // TODO: Test this
        ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size);  // Should only contain 9000
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    }

    // ES8+ Load Profile Elements
    clean_cb_data();
    ec = der_encode(asn_MBR_BoundProfilePackage_1[4].type, &(bpp->sequenceOf86), encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_INFO_ARRAY("sequenceOf86\n", get_cb_data(), get_cb_size());

    // Send sequenceOf86 TL
    buf = strchr(get_cb_data(),0x86);
    p = get_cb_data();
    len = buf-get_cb_data();  // TODO: Make it general
    MSG_INFO("len: %d\n", len);
    ret = cmd_store_data(p, len, out, out_size);  // Should only contain 9000
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    // Send sequenceOf86 Value
    cnt = bpp->sequenceOf86.list.count;
    MSG_INFO("cnt: %d\n", cnt);
    for (i = 0; i < cnt; i++) {
        req = bpp->sequenceOf86.list.array[i];
        clean_cb_data();
        ec = der_encode(&asn_DEF_OS86, req, encode_cb, NULL);
        if(ec.encoded == -1) {
            MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
            return RT_ERR_ASN1_ENCODE_FAIL;
        }
        MSG_INFO_ARRAY("sequenceOf86TLV\n", get_cb_data(), get_cb_size());
        ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);  // Should only contain 9000
    }

    // Handle notification
    if (*out_size > 0) {
        int tmp = sizeof(g_buf);
        ret = handle_notification(smdp_addr, out, *out_size, (char *)g_buf, &tmp);
    }

end:
    cJSON_Delete(content);
    ASN_STRUCT_FREE(asn_DEF_BoundProfilePackage, bpp);

    return ret;
}

int handle_notification(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size /* out */)
{
    int ret = RT_SUCCESS;
    cJSON *content = NULL;

    char *data = NULL;
    char *b64_in = NULL;
    if ((in == NULL) || (out == NULL) || (out_size == NULL)){
        return RT_ERR_NULL_POINTER;
    }
    MSG_INFO("transactionId: %s\n", g_transaction_id);

    b64_in = malloc(((in_size + 2) / 3) * 4 + 1);
    RT_CHECK_GO(b64_in, RT_ERR_OUT_OF_MEMORY, end);
    ret = rt_base64_encode(in, in_size, b64_in);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_INFO("b64_in:\n%s\n", b64_in);

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "transactionId", (char *)g_transaction_id), RT_ERR_CJSON_ERROR, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "pendingNotification", b64_in), RT_ERR_CJSON_ERROR, end);

    data = (char *)g_buf;
    ret = cJSON_PrintPreallocated(content, data, sizeof(g_buf), 1);
    RT_CHECK_GO(ret == 1, RT_ERR_CJSON_ERROR, end);
    MSG_INFO("content:\n%s\n", data);

    ret = lpa_https_post(smdp_addr, API_HANDLE_NOTIFICATION, data, out, out_size);
    if (ret == 204) {
        ret = RT_SUCCESS;
    } else {
        // ret = RT_ERR_HTTPS_POST_FAIL;
        ret = ret < 0 ? ret : RT_ERR_HTTPS_POST_FAIL;
    }

end:
    if (b64_in != NULL) { free(b64_in);}
    cJSON_Delete(content);

    return ret;
}

int es9p_cancel_session(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size /* out */)
{
    int ret = RT_SUCCESS;
    cJSON *content = NULL;

    char *data = NULL;
    char *b64_in = NULL;
    if ((in == NULL) || (out == NULL) || (out_size == NULL)){
        return RT_ERR_NULL_POINTER;
    }
    MSG_INFO("transactionId: %s\n", g_transaction_id);

    b64_in = malloc(((in_size + 2) / 3) * 4 + 1);
    RT_CHECK_GO(b64_in, RT_ERR_OUT_OF_MEMORY, end);
    ret = rt_base64_encode(in, in_size, b64_in);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_INFO("b64_in:\n%s\n", b64_in);

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "transactionId", (char *)g_transaction_id), RT_ERR_CJSON_ERROR, end);
    RT_CHECK_GO(cJSON_AddStringToObject(content, "cancelSessionResponse", b64_in), RT_ERR_CJSON_ERROR, end);

    data = (char *)g_buf;
    ret = cJSON_PrintPreallocated(content, data, sizeof(g_buf), 1);
    RT_CHECK_GO(ret == 1, RT_ERR_CJSON_ERROR, end);
    MSG_INFO("content:\n%s\n", data);

    ret = lpa_https_post(smdp_addr, API_CANCEL_SESSION, data, out, out_size);
    if (ret == 200) {
        ret = RT_SUCCESS;
    } else {
        MSG_ERR("lpa_https_post: %d\n", ret);
        // ret = RT_ERR_HTTPS_POST_FAIL;
        ret = ret < 0 ? ret : RT_ERR_HTTPS_POST_FAIL;
    }

end:
    if (b64_in != NULL) { free(b64_in);}
    cJSON_Delete(content);

    return ret;
}

void close_session(void)
{
    lpa_https_close_socket();
}

