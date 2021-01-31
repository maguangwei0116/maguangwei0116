#include <openssl/sha.h>
#include <string.h>
#include "cJSON.h"
#include "lpdd.h"
#include "lpa_config.h"
#include "apdu.h"
#include "convert.h"
#include "base64.h"
#include "https.h"
#include "hash.h"
#include "lpa_error_codes.h"
#include "rt_os.h"
#include "lpa_https.h"
#include "tlv.h"
#include "bertlv.h"

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

int list_notification(notification_t ne, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    /*
    * ListNotificationRequest ::= [40] SEQUENCE { -- Tag 'BF28'
        profileManagementOperation [1] NotificationEvent OPTIONAL }
    */

    if (ne == NE_INSTALL) {
        g_buf[0] = 7;
        g_buf[1] = 0x80;
    } else if (ne == NE_ENABLE) {
        g_buf[0] = 6;
        g_buf[1] = 0x40;
    } else if (ne == NE_DISABLE) {
        g_buf[0] = 5;
        g_buf[1] = 0x20;
    } else if (ne == NE_DELETE) {
        g_buf[0] = 4;
        g_buf[1] = 0x10;
    }

    if (ne == NE_ALL) {
        g_buf_size = bertlv_build_tlv(TAG_LPA_LIST_NOTIFICATION_REQ, 0, NULL, g_buf);
    } else {
        // profileManagementOperation [1] NotificationEvent
        g_buf_size = bertlv_build_tlv(0x81, 2, buf, buf);
        g_buf_size = bertlv_build_tlv(TAG_LPA_LIST_NOTIFICATION_REQ, len, g_buf, g_buf);
    }

    *out_size = 0;

    MSG_DUMP_ARRAY("ListNotificationRequest: ", get_cb_data(), get_cb_size());

    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel));
    //*out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int load_crl(const uint8_t *crl, uint16_t crl_size, uint8_t *out, uint16_t *out_size, uint8_t channel)
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
    // MSG_DUMP_ARRAY("LoadCRLRequest: ", get_cb_data(), get_cb_size());
    // RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size));
    // *out_size -= 2;  // Remove sw 9000

    // Impl for crl prepared by caller
    RT_CHECK(cmd_store_data(crl, crl_size, out, out_size, channel));
    *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int retrieve_notification_list(notification_t ne, long *seq, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    int ret = RT_SUCCESS;

    /*RetrieveNotificationsListRequest ::= [43] SEQUENCE { -- Tag 'BF2B'
        searchCriteria CHOICE {
            seqNumber [0] INTEGER,
            profileManagementOperation [1] NotificationEvent
        } OPTIONAL
    }
    */

    if (ne == NE_ALL) {
        g_buf_size = bertlv_build_tlv(TAG_LPA_RETRIEVE_NOTIFICATION_REQ, 0, NULL, g_buf);
    } else {
        if (ne == NE_SEQ_NUM) {
            // seqNumber [0] INTEGER,
            g_buf_size = bertlv_build_integer_tlv(0x80, (uint32_t)*seq, g_buf);
        } else {
            // profileManagementOperation [1] NotificationEvent
            if (ne == NE_INSTALL) {
                g_buf[0] = 7;
                g_buf[1] = 0x80;
            } else if (ne == NE_ENABLE) {
                g_buf[0] = 6;
                g_buf[1] = 0x40;
            } else if (ne == NE_DISABLE) {
                g_buf[0] = 5;
                g_buf[1] = 0x20;
            } else if (ne == NE_DELETE) {
                g_buf[0] = 4;
                g_buf[1] = 0x10;
            }
            g_buf_size = bertlv_build_tlv(0x81, 2, g_buf, g_buf);
        }
        g_buf_size = bertlv_build_tlv(TAG_LPA_RETRIEVE_NOTIFICATION_REQ, g_buf_size, g_buf, g_buf);
    }

    *out_size = 0;

    MSG_DUMP_ARRAY("RetrieveNotificationsListRequest: ", get_cb_data(), get_cb_size());
    ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*out_size -= 2;  // Remove sw 9000

    return ret;
}

int remove_notification_from_list(long seq, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    /*
    NotificationSentRequest ::= [48] SEQUENCE { -- Tag 'BF30' seqNumber [0] INTEGER }
    */
    *out_size = 0;

    g_buf_size = bertlv_build_integer_tlv(0x80, (uint32_t)*seq, g_buf);
    g_buf_size = bertlv_build_tlv(TAG_LPA_REMOVE_NOTIFICATION_REQ, g_buf_size, g_buf, g_buf);

    MSG_DUMP_ARRAY("ListNotificationRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel));
    *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int get_euicc_info(uint8_t *info1, uint16_t *size1, uint8_t *info2, uint16_t *size2, uint8_t channel)
{
    int ret = RT_SUCCESS;

    if ((info1 == NULL) || (size1 == NULL) ) {
        ret = RT_ERR_NULL_POINTER;
        goto end;
    }

    /*
    GetEuiccInfo1Request ::= [32] SEQUENCE { -- Tag 'BF20' }
    */
    g_buf_size = bertlv_build_tlv(TAG_LPA_GET_EUICC_INFO1_REQ, 0, NULL, g_buf);
    MSG_DUMP_ARRAY("GetEuiccInfo1Request: ", get_cb_data(), get_cb_size());

    ret = cmd_store_data(get_cb_data(), get_cb_size(), info1, size1, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size1 -= 2;  // Remove sw 9000

    if ((info2 != NULL) && (size2 != NULL)) {
        /*
        GetEuiccInfo2Request ::= [34] SEQUENCE { -- Tag 'BF22' }
        */
        g_buf_size = bertlv_build_tlv(TAG_LPA_GET_EUICC_INFO2_REQ, 0, NULL, g_buf);
        MSG_DUMP_ARRAY("GetEuiccInfo1Request: ", get_cb_data(), get_cb_size());

        ret = cmd_store_data(get_cb_data(), get_cb_size(), info2, size2, channel);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
        //*size2 -= 2;  // Remove sw 9000
    }

    return ret;
}

int get_euicc_challenge(uint8_t challenge[16], uint8_t channel)
{
    int ret = RT_SUCCESS;
    uint16_t rlen;
    uint8_t rsp[23];  // 8F2E128010-CHALLENAGE-9000

    /*
    GetEuiccChallengeRequest ::= [46] SEQUENCE { -- Tag 'BF2E' }
    */
    g_buf_size = bertlv_build_tlv(TAG_LPA_GET_EUICC_CHALLENGE_REQ, 0, NULL, g_buf);

    MSG_DUMP_ARRAY("GetEuiccChallengeRequest: ", get_cb_data(), get_cb_size());
    // RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), rsp, &rlen));
    ret = cmd_store_data(get_cb_data(), get_cb_size(), rsp, &rlen, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    memcpy(challenge, rsp + 5, 16);

    return ret;
}

int get_rat(uint8_t *rat, uint16_t *size, uint8_t channel)
{
    int ret = RT_SUCCESS;

    /*
    GetRatRequest ::= [67] SEQUENCE { -- Tag ' BF43' 
        -- No input data }
    */
    g_buf_size = bertlv_build_tlv(TAG_LPA_GET_RAT_REQ, 0, NULL, g_buf);

    MSG_DUMP_ARRAY("GetEuiccInfo1Request: ", get_cb_data(), get_cb_size());
    // RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), rat, size));
    ret = cmd_store_data(get_cb_data(), get_cb_size(), rat, size, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size -= 2;  // Remove sw 9000

    return ret;
}

int cancel_session(const uint8_t *tid, uint8_t tid_size, uint8_t reason, uint8_t *csr, uint16_t *size, uint8_t channel)
{
    int ret = RT_SUCCESS;
    
    /*
    CancelSessionRequest ::= [65] SEQUENCE { -- Tag 'BF41' 
        transactionId TransactionId, -- The TransactionID generated by the RSP Server 
        reason CancelSessionReason 
    }

    CancelSessionReason ::= INTEGER {
        endUserRejection(0), 
        postponed(1), 
        timeout(2), 
        pprNotAllowed(3), 
        metadataMismatch(4), 
        loadBppExecutionError(5), 
        undefinedReason(127)
    }
    */

    // transactionId TransactionId
    g_buf_size = bertlv_build_tlv(0x80, tid_size, tid, g_buf);
    // CancelSessionReason ::= INTEGER
    g_buf_size += bertlv_build_integer_tlv(0x81, (uint32_t)*seq, g_buf + g_buf_size);
    // CancelSessionRequest ::= [65] SEQUENCE { -- Tag 'BF41'
    g_buf_size = bertlv_build_tlv(TAG_LPA_CACEL_SESSION_REQ, g_buf_size, g_buf, g_buf);

    ret = cmd_store_data(get_cb_data(), get_cb_size(), csr, size, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size -= 2;  // Remove sw 9000

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
    p2 = strstr(p1, "\"");  // Find the second " after :
    RT_CHECK_GO(p2, RT_ERR_HTTPS_SMDP_ERROR, end);
    MSG_DBG("p1: %s\np2: %s\n", p1, p2);

    // Now we get subjectCode, and p1 points to the start and p2 points to the end of the subjectCode
    memcpy(sc, p1, p2 - p1);
    sc[p2-p1] = '\0';
    MSG_DBG("sc: %s\n", sc);

    p2 = strstr(p2, "\"reasonCode\""); // Find "reasonCode"
    RT_CHECK_GO(p2, RT_ERR_HTTPS_SMDP_ERROR, end);

    p2 += 12;  // Skip "reasonCode"
    p1 = strstr(p2, "\"");  // Find the first " after :
    RT_CHECK_GO(p1, RT_ERR_HTTPS_SMDP_ERROR, end);

    p1 += 1;  // Skipp "
    p2 = strstr(p1, "\"");  // Find the secoond " after :
    RT_CHECK_GO(p2, RT_ERR_HTTPS_SMDP_ERROR, end);
    MSG_DBG("p1: %s\np2: %s\n", p1, p2);

    // Now we get subjectCode, and p1 points to the start and p2 points to the end of the reasonCode
    memcpy(rc, p1, p2 - p1);
    rc[p2-p1] = '\0';
    MSG_DBG("rc: %s\n", rc);

    ret = 1;

end:
    return ret;
}

int initiate_authentication(const char *smdp_addr, char *auth_data, int *size, uint8_t channel)
{
    int ret = RT_SUCCESS;
    char *data = NULL;
    cJSON *content = NULL;
    uint8_t tmp[1024] = {0};
    uint16_t tmp_size = 0;

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);

    RT_CHECK_GO((ret = get_euicc_challenge(tmp, channel)) == RT_SUCCESS, ret, end);
    MSG_DUMP_ARRAY("euiccChallenge: ", tmp, 16);
    RT_CHECK_GO((ret = rt_base64_encode(tmp, 16, (char *)g_buf)) == RT_SUCCESS, ret, end);
    cJSON_AddStringToObject(content, "euiccChallenge", (char *)g_buf);

    RT_CHECK_GO((ret = get_euicc_info(tmp, &tmp_size, NULL, NULL, channel)) == RT_SUCCESS, ret, end);
    MSG_DUMP_ARRAY("euiccInfo1: ", tmp, tmp_size);
    RT_CHECK_GO((ret = rt_base64_encode(tmp, tmp_size, (char *)g_buf)) == RT_SUCCESS, ret, end);
    cJSON_AddStringToObject(content, "euiccInfo1", (char *)g_buf);

    cJSON_AddStringToObject(content, "smdpAddress", smdp_addr);

    data = cJSON_PrintUnformatted(content);
    if (data != NULL) {
        ret = lpa_https_post(smdp_addr, API_INITIATE_AUTHENTICATION, data, auth_data, size);
    }

    // RT_CHECK_GO(ret == 200, RT_ERR_HTTPS_POST_FAIL, end);
    if (ret == 200) {
        ret = RT_SUCCESS;
    } else {
        MSG_ERR("lpa https post: %d\n", ret);
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
    if (data) {
        cJSON_free(data);
    }
    cJSON_Delete(content);
    return ret;
}

int gen_ctx_params1(const char *matching_id, char* buff, uint16_t* len)
{
    int ret = RT_SUCCESS;
    uint32_t tac_len;
    uint32_t dev_cap_len;

    uint8_t arr_tac[]           = {0x00, 0x00, 0x00, 0x00};
    uint8_t arr_gsm[]           = {0x05, 0x00, 0x00};
    uint8_t arr_utran[]         = {0x08, 0x00, 0x00};
    uint8_t arr_cdma1x[]        = {0x01, 0x00, 0x00};  // Not support
    uint8_t arr_cdmahrpd[]      = {0x01, 0x00, 0x00};  // Not support
    uint8_t arr_cdmaehrpd[]     = {0x02, 0x00, 0x00};  // Not support
    uint8_t arr_eutran[]        = {0x02, 0x00, 0x00};
    uint8_t arr_contract_less[] = {0x09, 0x00, 0x00};
    uint8_t arr_rsp_ctl[]       = {0x02, 0x00, 0x00};

    /*
    CtxParams1 ::= CHOICE {
        ctxParamsForCommonAuthentication CtxParamsForCommonAuthentication-- New contextual data objects MAY be defined for extensibility.
    }

    CtxParamsForCommonAuthentication ::= SEQUENCE {
        matchingId UTF8String OPTIONAL, -- The MatchingId could be the Activation code token or EventID or empty
        deviceInfo DeviceInfo -- The Device information
    }
    DeviceInfo ::= SEQUENCE {
        tac Octet4,
        deviceCapabilities DeviceCapabilities,
        imei Octet8 OPTIONAL
    }

    DeviceCapabilities ::= SEQUENCE { -- Highest fully supported release for each definition 
        -- The device SHALL set all the capabilities it supports 
        gsmSupportedRelease VersionType OPTIONAL, 
        utranSupportedRelease VersionType OPTIONAL, 
        cdma2000onexSupportedRelease VersionType OPTIONAL, 
        cdma2000hrpdSupportedRelease VersionType OPTIONAL, 
        cdma2000ehrpdSupportedRelease VersionType OPTIONAL, 
        eutranSupportedRelease VersionType OPTIONAL, 
        contactlessSupportedRelease VersionType OPTIONAL, 
        rspCrlSupportedVersion VersionType OPTIONAL 
    }

    VersionType ::= OCTET STRING(SIZE(3))
    */

    // matchingId UTF8String OPTIONAL
    if (matching_id != NULL) {
        *len = bertlv_build_tlv(0x80, strlen(matching_id), matching_id, buff);
    } else {
        *len = 0;
    }

    // tac Octet4,
    tac_len = bertlv_build_tlv(0x80, sizeof(arr_tac), arr_tac, buff + *len);

    dev_cap_len = bertlv_build_tlv(0x80, sizeof(arr_gsm), arr_gsm, buff + *len + tac_len);
    dev_cap_len += bertlv_build_tlv(0x81, sizeof(arr_utran), arr_utran, buff + *len + tac_len + dev_cap_len);
    dev_cap_len += bertlv_build_tlv(0x82, sizeof(arr_cdma1x), arr_cdma1x, buff + *len + tac_len + dev_cap_len);
    dev_cap_len += bertlv_build_tlv(0x83, sizeof(arr_cdmahrpd), arr_cdmahrpd, buff + *len + tac_len + dev_cap_len);
    dev_cap_len += bertlv_build_tlv(0x84, sizeof(arr_cdmaehrpd), arr_cdmaehrpd, buff + *len + tac_len + dev_cap_len);
    dev_cap_len += bertlv_build_tlv(0x85, sizeof(arr_eutran), arr_eutran, buff + *len + tac_len + dev_cap_len);
    dev_cap_len += bertlv_build_tlv(0x86, sizeof(arr_contract_less), arr_contract_less, buff + *len + tac_len + dev_cap_len);
    dev_cap_len += bertlv_build_tlv(0x87, sizeof(arr_rsp_ctl), arr_rsp_ctl, buff + *len + tac_len + dev_cap_len);
    // DeviceCapabilities ::= SEQUENCE {
    dev_cap_len = bertlv_build_tlv(0xA1, dev_cap_len, buff + *len + tac_len, buff + *len + tac_len);

    // imei Octet8 OPTIONAL = NULL

    // DeviceInfo ::= SEQUENCE
    tac_len = bertlv_build_tlv(0xA1, tac_len + dev_cap_len, buff + *len, buff + *len);

    // CtxParams1 ::= CHOICE ctxParamsForCommonAuthentication CtxParamsForCommonAuthentication
    *len = bertlv_build_tlv(0xA0, *len + tac_len, buff, buff);

    return ret;
}

static int get_data_from_json(cJSON* json, const char* key, char* buf, uint16_t *len)
{
    int ret = RT_SUCCESS;
    char* b64_str = NULL;

    b64_str = cJSON_GetObjectItem(json, key)->valuestring;
    RT_CHECK_GO(b64_str, RT_ERR_CJSON_ERROR, end);
    MSG_DBG("%s: %s\n", key, b64_str);

    RT_CHECK_GO(buf, RT_ERR_OUT_OF_MEMORY, end);

    ret = rt_base64_decode(b64_str, buf, len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DUMP_ARRAY("data: ", buf, len);

end:
    return ret;
}

static int get_transaction_id(uint8_t* server_signed, uint8_t *transaction_id, uint8_t *size)
{
    uint16_t offset;
    uint16_t offset_2;
    uint32_t value_len;

    // get ServerSigned1 value
    offset = bertlv_get_tl_length(server_signed, &value_len);
    // find transactionId tag
    offset_2 = (uint16_t)bertlv_find_tag(server_signed + offset, value_len, 0x80, 1);

    if (offset_2 == BERTLV_INVALID_OFFSET) {
        MSG_ERR("Could not find transactionId !\n");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    offset += offset_2;

    // get transactionId value
    offset_2 = bertlv_get_tl_length(server_signed + offset, &value_len);
    offset += offset_2;

    *size = (uint8_t)value_len;
    memcpy(transaction_id, server_signed + offset, *size);

    return RT_SUCCESS;
}

static int get_cc_hash(const char *cc, const uint8_t *transaction_id, uint8_t id_size, uint8_t *hash)
{
    SHA256_CTX ctx;
    uint8_t tmp[SHA256_DIGEST_LENGTH + 16] = {0};  // 16 for transactionId

    RT_CHECK_EQ(SHA256_Init(&ctx), 1);
    RT_CHECK_EQ(SHA256_Update(&ctx, cc, strlen(cc)), 1);
    RT_CHECK_EQ(SHA256_Final(tmp, &ctx), 1);
    MSG_DUMP_ARRAY("Hash: ", tmp, sizeof(tmp));

    memcpy(&tmp[SHA256_DIGEST_LENGTH], transaction_id, id_size);
    MSG_DUMP_ARRAY("Hash: ", tmp, sizeof(tmp));

    RT_CHECK_EQ(SHA256_Init(&ctx), 1);
    RT_CHECK_EQ(SHA256_Update(&ctx, tmp, SHA256_DIGEST_LENGTH + id_size), 1);
    RT_CHECK_EQ(SHA256_Final(hash, &ctx), 1);
    MSG_DUMP_ARRAY("Hash: ", hash, SHA256_DIGEST_LENGTH);

    return RT_SUCCESS;
}

int authenticate_server(const char *matching_id, const char *auth_data,
                        uint8_t *response, uint16_t *size , uint8_t channel/* out */)
{
    uint8_t transaction_id[16];
    uint8_t transaction_id_len;
    uint16_t len = 0;
    int ret = RT_SUCCESS;
    void *p = NULL;
    cJSON *content = NULL;

    /*
    AuthenticateServerRequest ::= [56] SEQUENCE { -- Tag 'BF38' 
        serverSigned1 ServerSigned1, -- Signed information 
        serverSignature1 [APPLICATION 55] OCTET STRING, -- tag ¡®5F37¡¯ 
        euiccCiPKIdToBeUsed SubjectKeyIdentifier, -- CI Public Key Identifier to be used 
        serverCertificate Certificate, -- RSP Server Certificate CERT.XXauth.ECDSA 
        ctxParams1 CtxParams1 
    }

    ServerSigned1 ::= SEQUENCE { 
        transactionId [0] TransactionId, -- The Transaction ID generated by the RSP Server 
        euiccChallenge [1] Octet16, -- The eUICC Challenge 
        serverAddress [3] UTF8String, -- The RSP Server address as an FQDN 
        serverChallenge [4] Octet16 -- The RSP Server Challenge 
    }
    */

    content = cJSON_Parse(auth_data);
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);

    // serverSigned1 ServerSigned1
    ret = get_data_from_json(content, "serverSigned1", g_buf, &len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    g_buf_size = len;

    ret = get_transaction_id(g_buf, transaction_id, &transaction_id_len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    ret = bytes2hexstring(transaction_id, transaction_id_len, g_transaction_id);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    // serverSignature1 [APPLICATION 55] OCTET STRING
    ret = get_data_from_json(content, "serverSignature1", g_buf + g_buf_size, &len);
    if (bertlv_get_tag(g_buf, NULL) != 0x5F37) {
        MSG_ERR("Broken serverSignature1 decoding at byte 0\n");
        ret = RT_ERR_ASN1_DECODE_FAIL;
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    g_buf_size += len;

    // euiccCiPKIdToBeUsed SubjectKeyIdentifier
    ret = get_data_from_json(content, "euiccCiPKIdToBeUsed", g_buf + g_buf_size, &len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    g_buf_size += len;

    // serverCertificate Certificate
    ret = get_data_from_json(content, "serverCertificate", g_buf + g_buf_size, &len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    g_buf_size += len;

    // ctxParams1 CtxParams1
    RT_CHECK(gen_ctx_params1(matching_id, g_buf + g_buf_size, &len));
    MSG_DUMP_ARRAY("ctxParams1:\n", g_buf + g_buf_size, len);
    g_buf_size += len;

    // AuthenticateServerRequest
    g_buf_size = bertlv_build_tlv(TAG_LPA_AUTH_SERVER_REQ, g_buf_size, g_buf, g_buf);
    MSG_DUMP_ARRAY("AuthenticateServerRequest\n", get_cb_data(), get_cb_size());

    ret = cmd_store_data(get_cb_data(), get_cb_size(), response, size, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size -= 2;  // Remove sw 9000

end:
    cJSON_Delete(content);

    return ret;
}

int authenticate_client(const char *smdp_addr, const uint8_t *in, uint16_t in_size,
                        char *out, int *out_size /* out */)
{
    int ret = RT_SUCCESS;
    cJSON *content = NULL;
    char *data = NULL;
    char *b64_in = NULL;
    int b64_len = 0;

    if ((in == NULL) || (out == NULL) || (out_size == NULL)){
        return RT_ERR_NULL_POINTER;
    }
    MSG_DBG("transactionId: %s\n", g_transaction_id);

    b64_len = ((in_size + 2) / 3) * 4 + 1;
    b64_in = malloc(b64_len);
    rt_os_memset(b64_in, 0, b64_len);
    RT_CHECK_GO(b64_in, RT_ERR_OUT_OF_MEMORY, end);
    ret = rt_base64_encode(in, in_size, b64_in);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DBG("b64_in:\n%s\n", b64_in);

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);
    cJSON_AddStringToObject(content, "transactionId", (char *)g_transaction_id);
    cJSON_AddStringToObject(content, "authenticateServerResponse", b64_in);

    data = cJSON_PrintUnformatted(content);
    if (data != NULL) {
        ret = lpa_https_post(smdp_addr, API_AUTHENTICATE_CLIENT, data, out, out_size);
    }

    RT_CHECK_GO(ret == 200, RT_ERR_HTTPS_POST_FAIL, end);

    ret = get_status_codes(out, (char *)b64_in, (char *)g_buf);
    if (ret == 1) {
        MSG_ERR("subjectCode: %s, reasonCode: %s\n", b64_in, g_buf);
        ret = RT_ERR_AUTHENTICATE_CLIENT;
        goto end;
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

end:
    if (data != NULL) {
        cJSON_free(data);
    }
    if (b64_in != NULL) { free(b64_in);}
    cJSON_Delete(content);

    return ret;
}

int prepare_download(const char *req_str, const char *cc, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    int ret = RT_SUCCESS;
    cJSON *content = NULL;
    uint8_t transaction_id[16];
    uint8_t transaction_id_len;
    uint8_t hash_cc[SHA256_DIGEST_LENGTH] = { 0 };
    uint16_t len;

    /*
    PrepareDownloadRequest ::= [33] SEQUENCE { -- Tag 'BF21' 
        smdpSigned2 SmdpSigned2, -- Signed information 
        smdpSignature2 [APPLICATION 55] OCTET STRING, -- tag '5F37' 
        hashCc Octet32 OPTIONAL, -- Hash of confirmation code 
        smdpCertificate Certificate -- CERT.DPpb.ECDSA 
    } 
    
    SmdpSigned2 ::= SEQUENCE { 
        transactionId [0] TransactionId, -- The TransactionID generated by the SM-DP+ 
        ccRequiredFlag BOOLEAN, --Indicates if the Confirmation Code is required 
        bppEuiccOtpk [APPLICATION 73] OCTET STRING OPTIONAL -- otPK.EUICC.ECKA already used for binding the BPP, tag '5F49' 
    }
    */

    content = cJSON_Parse(req_str);
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);

    // smdpSigned2 SmdpSigned2
    ret = get_data_from_json(content, "smdpSigned2", g_buf, &len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    g_buf_size += len;

    // smdpSignature2 [APPLICATION 55] OCTET STRING
    ret = get_data_from_json(content, "smdpSignature2", g_buf + g_buf_size, &len);
    if (bertlv_get_tag(g_buf, NULL) != 0x5F37) {
        MSG_ERR("Broken serverSignature1 decoding at byte 0\n");
        ret = RT_ERR_ASN1_DECODE_FAIL;
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    // hashCc Octet32 OPTIONAL
    if (cc != NULL) {
        ret = get_transaction_id(g_buf, transaction_id, &transaction_id_len);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

        ret = get_cc_hash(cc, transaction_id, transaction_id_len, hash_cc);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

        len = bertlv_build_tlv(ASN1_TAG_OCTET_STRING, SHA256_DIGEST_LENGTH, hash_cc, g_buf + g_buf_size);
        g_buf_size += len;
    }

    // smdpCertificate Certificate
    ret = get_data_from_json(content, "smdpCertificate", g_buf + g_buf_size, &len);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    g_buf_size += len;

    // PrepareDownloadRequest
    g_buf_size = bertlv_build_tlv(TAG_LPA_PREPARE_DOWNLOAD_REQ, g_buf_size, g_buf, g_buf);
    MSG_DUMP_ARRAY("PrepareDownloadRequest\n", get_cb_data(), get_cb_size());

    ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*out_size -= 2;  // Remove sw 9000

end:
    cJSON_Delete(content);

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
    MSG_DBG("transactionId: %s\n", g_transaction_id);

    b64_in = malloc(((in_size + 2) / 3) * 4 + 1);
    RT_CHECK_GO(b64_in, RT_ERR_OUT_OF_MEMORY, end);
    ret = rt_base64_encode(in, in_size, b64_in);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DBG("b64_in[%d]:\n%s\n", strlen(b64_in), b64_in);

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);
    cJSON_AddStringToObject(content, "transactionId", (char *)g_transaction_id);
    cJSON_AddStringToObject(content, "prepareDownloadResponse", b64_in);

    data = cJSON_PrintUnformatted(content);
    if (data != NULL) {
        ret = lpa_https_post(smdp_addr, API_GET_BOUND_PROFILE_PACKAGE, data, out, out_size);
    }

    RT_CHECK_GO(ret == 200, RT_ERR_HTTPS_POST_FAIL, end);

    ret = get_status_codes(out, (char *)b64_in, (char *)g_buf);
    if (ret == 1) {
        MSG_ERR("subjectCode: %s, reasonCode: %s\n", b64_in, g_buf);
        ret = RT_ERR_GET_BOUND_PROFILE_PACKAGE;
        goto end;
    }
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

end:
    if (data != NULL) {
        cJSON_free(data);
    }
    if (b64_in != NULL) { free(b64_in);}
    cJSON_Delete(content);

    return ret;
}

int load_bound_profile_package(const char *smdp_addr, const char *get_bpp_rsp,
                            uint8_t *out, uint16_t *out_size , uint8_t channel/* out */)
{
    int i;
    int ret = RT_SUCCESS;
    cJSON *content = NULL;
    uint8_t *p = NULL;
    uint16_t len = 0;
    uint16_t offset;
    uint16_t sub_off;
    uint32_t value_len;

    *out_size = 0;

    /*
    BoundProfilePackage ::= [54] SEQUENCE { -- Tag 'BF36' 
        initialiseSecureChannelRequest [35] InitialiseSecureChannelRequest, -- Tag 'BF23' 
        firstSequenceOf87 [0] SEQUENCE OF [7] OCTET STRING, -- sequence of '87' TLVs 
        sequenceOf88 [1] SEQUENCE OF [8] OCTET STRING, -- sequence of '88' TLVs 
        secondSequenceOf87 [2] SEQUENCE OF [7] OCTET STRING OPTIONAL, -- sequence of '87' TLVs 
        sequenceOf86 [3] SEQUENCE OF [6] OCTET STRING -- sequence of '86' TLVs 
    }
    */

    content = cJSON_Parse(get_bpp_rsp);
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);

    ret = get_data_from_json(content, "boundProfilePackage", g_buf, &g_buf_size);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DUMP_ARRAY("boundProfilePackage\n", get_cb_data(), get_cb_size());

    // Tag and length fields of the BoundProfilePackage TLV plus the initialiseSecureChannelRequest TLV
    // ES8+ InitialiseSecureChannel
    RT_CHECK_GO(bertlv_get_tag(g_buf, NULL) == 0xBF36, ret, end);
    len = bertlv_get_tl_length(g_buf, NULL);  // Play a trick here, include the BPP TLV

    RT_CHECK_GO(bertlv_get_tag(g_buf + len, NULL) == 0xBF23, ret, end);
    len += bertlv_get_tlv_length(g_buf + len);  // TLV of InitialiseSecureChannel
    MSG_DUMP_ARRAY("initialiseSecureChannelRequest\n", g_buf, len);
    ret = cmd_store_data(g_buf, len, out, out_size, channel);  // Should only contain 9000
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    offset = len;

    // Tag and length fields of the first sequenceOf87 TLV plus the first '87' TLV
    // ES8+ Configure ISDP
    RT_CHECK_GO(bertlv_get_tag(g_buf + offset, NULL) == 0xA0, ret, end);
    len = bertlv_get_tlv_length(g_buf + offset);  // TLV of Configure ISDP
    MSG_DUMP_ARRAY("firstSequenceOf87\n", g_buf + offset, len);
    ret = cmd_store_data(g_buf + offset, len, out, out_size, channel);  // Should only contain 9000
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    if (*out_size > 0) {
        ret = parse_install_profile_result(out, *out_size);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    }
    offset += len;

    // Tag and length fields of the sequenceOf88 TLV
    RT_CHECK_GO(bertlv_get_tag(g_buf + offset, NULL) == 0xA1, ret, end);
    len = bertlv_get_tl_length(g_buf + offset, &value_len);
    // ES8+ Store Metadata
    MSG_DUMP_ARRAY("sequenceOf88\n", g_buf + offset, len + value_len);
    ret = cmd_store_data(g_buf + offset, len, out, out_size, channel);  // Should only contain 9000
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    offset += len;

    // Each of the '88' TLVs
    for (sub_off = 0; sub_off < value_len; ) {
        RT_CHECK_GO(bertlv_get_tag(g_buf + offset + sub_off, NULL) == 0x88, ret, end);
        len = bertlv_get_tlv_length(g_buf + offset + sub_off);
        MSG_DUMP_ARRAY("sequenceOf88TLV\n", g_buf + offset + sub_off, len);
        ret = cmd_store_data(g_buf + offset + sub_off, len, out, out_size, channel);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);  // Should only contain 9000
        sub_off += len;

        /* check result code */
        MSG_DUMP_ARRAY("sequenceOf88TLV out\n", out, *out_size);
        if (*out_size > 0) {
            ret = parse_install_profile_result(out, *out_size);
            RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
        }
    }
    offset += value_len;

    // Tag and length fields of the sequenceOf87 TLV plus the first '87' TLV
    // ES8+ Replace Session Keys
    if (bertlv_get_tag(g_buf + offset, NULL) == 0xA2) {
        len = bertlv_get_tlv_length(g_buf + offset);
        MSG_DUMP_ARRAY("secondSequenceOf87\n", g_buf + offset, len);
        // TODO: Test this
        ret = cmd_store_data(g_buf + offset, len, out, out_size, channel);  // Should only contain 9000
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
        if (*out_size > 0) {
            ret = parse_install_profile_result(out, *out_size);
            RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
        }
        offset += len;
    }

    // Tag and length fields of the sequenceOf86 TLV
    RT_CHECK_GO(bertlv_get_tag(g_buf + offset, NULL) == 0xA3, ret, end);
    len = bertlv_get_tl_length(g_buf + offset, &value_len);
    // ES8+ Load Profile Elements
    MSG_DUMP_ARRAY("sequenceOf86\n", g_buf + offset, len + value_len);
    ret = cmd_store_data(g_buf + offset, len, out, out_size, channel);  // Should only contain 9000
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

    // Each of the '86' TLVs
    for (sub_off = 0; sub_off < value_len; ) {
        RT_CHECK_GO(bertlv_get_tag(g_buf + offset + sub_off, NULL) == 0x86, ret, end);
        len = bertlv_get_tlv_length(g_buf + offset + sub_off);
        MSG_DUMP_ARRAY("sequenceOf86TLV\n", g_buf + offset + sub_off, len);
        ret = cmd_store_data(g_buf + offset + sub_off, len, out, out_size, channel);
        RT_CHECK_GO(ret == RT_SUCCESS, ret, end);  // Should only contain 9000
        sub_off += len;
    }

    /* check result code */
    MSG_DUMP_ARRAY("ProfileInstallationResult: \n", out, *out_size);

end:
    cJSON_Delete(content);

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
    MSG_DBG("transactionId: %s\n", g_transaction_id);

    b64_in = malloc(((in_size + 2) / 3) * 4 + 1);
    RT_CHECK_GO(b64_in, RT_ERR_OUT_OF_MEMORY, end);
    ret = rt_base64_encode(in, in_size, b64_in);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DBG("b64_in:\n%s\n", b64_in);

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);
    cJSON_AddStringToObject(content, "transactionId", (char *)g_transaction_id);
    cJSON_AddStringToObject(content, "pendingNotification", b64_in);

    data = cJSON_PrintUnformatted(content);
    if (data != NULL) {
        ret = lpa_https_post(smdp_addr, API_HANDLE_NOTIFICATION, data, out, out_size);
    }

    if (ret == 204) {
        ret = RT_SUCCESS;
    } else {
        // ret = RT_ERR_HTTPS_POST_FAIL;
        ret = ret < 0 ? ret : RT_ERR_HTTPS_POST_FAIL;
    }

end:
    if (!data) {
        cJSON_free(data);
    }
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
    MSG_DBG("transactionId: %s\n", g_transaction_id);

    b64_in = malloc(((in_size + 2) / 3) * 4 + 1);
    RT_CHECK_GO(b64_in, RT_ERR_OUT_OF_MEMORY, end);
    ret = rt_base64_encode(in, in_size, b64_in);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    MSG_DBG("b64_in:\n%s\n", b64_in);

    content = cJSON_CreateObject();
    RT_CHECK_GO(content, RT_ERR_CJSON_ERROR, end);
    cJSON_AddStringToObject(content, "transactionId", (char *)g_transaction_id);
    cJSON_AddStringToObject(content, "cancelSessionResponse", b64_in);

    data = cJSON_PrintUnformatted(content);
    if (data != NULL) {
        ret = lpa_https_post(smdp_addr, API_CANCEL_SESSION, data, out, out_size);
    }

    if (ret == 200) {
        ret = RT_SUCCESS;
    } else {
        MSG_ERR("lpa https post: %d\n", ret);
        // ret = RT_ERR_HTTPS_POST_FAIL;
        ret = ret < 0 ? ret : RT_ERR_HTTPS_POST_FAIL;
    }

end:
    if (!data) {
        cJSON_free(data);
    }
    if (b64_in != NULL) { free(b64_in);}
    cJSON_Delete(content);

    return ret;
}

int load_customized_data(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel)
{
    int ret = RT_SUCCESS;

    ret = cmd_store_data(data, data_len, g_buf, &g_buf_size, channel);  // Should only contain 9000
    if (ret == RT_SUCCESS) {
        if ((rsp != NULL) && (rsp_len != NULL)) {
            memcpy(rsp, g_buf, g_buf_size);
            *rsp_len = g_buf_size;
        }
        return g_buf[5];
    }

    return ret;
}

void close_session(void)
{
    lpa_https_close();
}

