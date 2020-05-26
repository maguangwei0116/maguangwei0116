
#include "EnableProfileRequest.h"
#include "DisableProfileRequest.h"
#include "DeleteProfileRequest.h"
#include "GetEuiccDataRequest.h"
#include "ProfileInfoListRequest.h"
#include "SetNicknameRequest.h"
#include "EuiccMemoryResetRequest.h"
#include "MoreEIDOperateRequest.h"
#include "luid.h"
#include "lpa_config.h"
#include "lpa_error_codes.h"
#include "apdu.h"

// Defined in lpdd.c
extern int encode_cb(const void *buffer, size_t size, void *app_key);
extern void clean_cb_data(void);
extern uint8_t *get_cb_data(void);
extern uint16_t get_cb_size(void);

int enable_profile(profile_id_t pid, uint8_t id[16], bool refresh, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    int ret = RT_SUCCESS;
    asn_enc_rval_t ec;
    EnableProfileRequest_t req = {0};

    if (pid == PID_ISDP_AID) {
        req.profileIdentifier.present = EnableProfileRequest__profileIdentifier_PR_isdpAid;
        req.profileIdentifier.choice.isdpAid.buf = id;
        req.profileIdentifier.choice.isdpAid.size = 16;
    } else if (pid == PID_ICCID) {
        req.profileIdentifier.present = EnableProfileRequest__profileIdentifier_PR_iccid;
        req.profileIdentifier.choice.iccid.buf = id;
        req.profileIdentifier.choice.iccid.size = 10;
    }
    req.refreshFlag = refresh ? 0xFF : 0x00;
    *out_size = 0;
    clean_cb_data();
    ec = der_encode(&asn_DEF_EnableProfileRequest, &req, encode_cb, NULL);
    MSG_INFO("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_DUMP_ARRAY("EnableProfileRequest: ", get_cb_data(), get_cb_size());
    ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel);
    if (ret == RT_SUCCESS){
        *out_size -= 2;  // Remove sw 9000
    } else if (ret == RT_ERR_AT_WRONG_RSP) {
        // BF31038001009000
        // With refresh request, it might be failed to get response, this also indicates success
        *out_size = 6;
        out[5] = 0x00;
    }
    return ret;
}

int disable_profile(profile_id_t pid, uint8_t id[16], bool refresh, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    int ret = RT_SUCCESS;
    asn_enc_rval_t ec;
    DisableProfileRequest_t req = {0};

    if (pid == PID_ISDP_AID) {
        req.profileIdentifier.present = DisableProfileRequest__profileIdentifier_PR_isdpAid;
        req.profileIdentifier.choice.isdpAid.buf = id;
        req.profileIdentifier.choice.isdpAid.size = 16;
    } else if (pid == PID_ICCID) {
        req.profileIdentifier.present = DisableProfileRequest__profileIdentifier_PR_iccid;
        req.profileIdentifier.choice.iccid.buf = id;
        req.profileIdentifier.choice.iccid.size = 10;
    }
    req.refreshFlag = refresh ? 0xFF : 0x00;
    *out_size = 0;
    clean_cb_data();
    ec = der_encode(&asn_DEF_DisableProfileRequest, &req, encode_cb, NULL);
    MSG_INFO("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_DUMP_ARRAY("DisableProfileRequest: ", get_cb_data(), get_cb_size());
    ret = cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel);
    if (ret == RT_SUCCESS){
        *out_size -= 2;  // Remove sw 9000
    } else if (ret == RT_ERR_AT_WRONG_RSP) {
        // BF32038001009000
        // With refresh request, it might be failed to get response, this also indicates success
        *out_size = 6;
        out[5] = 0x00;
    }

    return ret;
}

int delete_profile(profile_id_t pid, uint8_t id[16], uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    asn_enc_rval_t ec;
    DeleteProfileRequest_t req = {0};

    if (pid == PID_ISDP_AID) {
        req.present = DeleteProfileRequest_PR_isdpAid;
        req.choice.isdpAid.buf = id;
        req.choice.isdpAid.size = 16;
    } else if (pid == PID_ICCID) {
        req.present = DeleteProfileRequest_PR_iccid;
        req.choice.iccid.buf = id;
        req.choice.iccid.size = 10;
    }
    *out_size = 0;
    clean_cb_data();
    ec = der_encode(&asn_DEF_DeleteProfileRequest, &req, encode_cb, NULL);
    MSG_INFO("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_DUMP_ARRAY("DeleteProfileRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel));
    *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int get_eid(uint8_t *eid, uint16_t *size, uint8_t channel)
{
    asn_enc_rval_t ec;
    GetEuiccDataRequest_t req = {0};
    uint8_t tag[1] = {0x5A};

    req.tagList.buf = tag;
    req.tagList.size = 1;
    clean_cb_data();
    ec = der_encode(&asn_DEF_GetEuiccDataRequest, &req, encode_cb, NULL);
    MSG_DBG("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_DUMP_ARRAY("GetEuiccDataRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), eid, size, channel));
    MSG_DBG("get_eid  *size: %d\n", *size);
    *size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int switch_eid(uint8_t *eid, uint16_t size,uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    asn_enc_rval_t ec;
    MoreEIDOperateRequest_t req = {0};
    req.present = MoreEIDOperateRequest_PR_eidValue;
    req.choice.eidValue.buf = eid;
    req.choice.eidValue.size = size;
    clean_cb_data();
    ec = der_encode(&asn_DEF_MoreEIDOperateRequest, &req, encode_cb, NULL);
    MSG_INFO("ec.encoded: %d\n", (int)ec.encoded);
    MSG_DUMP_ARRAY("GetEuicclistRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel));
    *out_size -= 2;  // Remove sw 9000
    return RT_SUCCESS;
}

int get_eid_list(uint8_t *eid, uint16_t *size, uint8_t channel)
{
    asn_enc_rval_t ec;
    MoreEIDOperateRequest_t req = {0};
    req.present = MoreEIDOperateRequest_PR_listAllEID;
    clean_cb_data();
    ec = der_encode(&asn_DEF_MoreEIDOperateRequest, &req, encode_cb, NULL);
    MSG_INFO("ec.encoded: %d\n", (int)ec.encoded);
    MSG_DUMP_ARRAY("GetEuicclistRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), eid, size, channel));
    //*size -= 2;  // Remove sw 9000
    return RT_SUCCESS;
}

int get_profiles_info(search_criteria_t sc, uint8_t *criteria, uint16_t c_size,
                    uint8_t *profile_info, uint16_t *size , uint8_t channel/* out */)
{
    int ret = RT_SUCCESS;
    asn_enc_rval_t ec;
    ProfileInfoListRequest_t *req = NULL;
    req = calloc(1, sizeof(ProfileInfoListRequest_t));
    RT_CHECK_GO(req, RT_ERR_OUT_OF_MEMORY, end);
    if (sc != SEARCH_NONE) {
        req->searchCriteria = calloc(1, sizeof(struct ProfileInfoListRequest__searchCriteria));
        RT_CHECK_GO(req->searchCriteria, RT_ERR_OUT_OF_MEMORY, end);
        if (sc == SEARCH_ISDP_AID) {
            req->searchCriteria->present = ProfileInfoListRequest__searchCriteria_PR_isdpAid;
            OCTET_STRING_fromBuf(&(req->searchCriteria->choice.isdpAid), (char *)criteria, c_size);
        } else if (sc == SEARCH_ICCID) {
            req->searchCriteria->present = ProfileInfoListRequest__searchCriteria_PR_iccid;
            OCTET_STRING_fromBuf(&(req->searchCriteria->choice.iccid), (char *)criteria, c_size);
        } else if (sc == SEARCH_PROFILE_CLASS) {
            req->searchCriteria->present = ProfileInfoListRequest__searchCriteria_PR_profileClass;
            req->searchCriteria->choice.profileClass = *criteria;
        }
    }
    clean_cb_data();
    ec = der_encode(&asn_DEF_ProfileInfoListRequest, req, encode_cb, NULL);
    MSG_DBG("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        ret = RT_ERR_ASN1_ENCODE_FAIL;
        goto end;
    }

    MSG_DUMP_ARRAY("ProfileInfoListRequest_t: ", get_cb_data(), get_cb_size());
    // RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), profile_info, size));
    ret = cmd_store_data(get_cb_data(), get_cb_size(), profile_info, size, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);
    //*size -= 2;  // Remove sw 9000

end:
    ASN_STRUCT_FREE(asn_DEF_ProfileInfoListRequest, req);

    return ret;
}

// TODO: Fix 6A80
int set_nickname(uint8_t iccid[10], const char *nickname, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    asn_enc_rval_t ec;
    SetNicknameRequest_t req = {0};

    req.iccid.buf = iccid;
    req.iccid.size = 10;

    req.profileNickname.buf = (uint8_t *)nickname;
    req.profileNickname.size = strlen(nickname);
    clean_cb_data();
    ec = der_encode(&asn_DEF_SetNicknameRequest, &req, encode_cb, NULL);
    MSG_INFO("ec.encoded: %d\n", (int)ec.encoded);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_DUMP_ARRAY("GetEuiccInfo1Request: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel));
    *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int euicc_memory_reset(memory_reset_t mrt, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    asn_enc_rval_t ec;
    uint8_t bit_string[1] = {0};
    EuiccMemoryResetRequest_t req = {0};

    if (mrt & RESET_OPERATIONAL_PROFILE) {
        bit_string[0] |= 0x80;
    }

    if (mrt & RESET_TEST_PROFILE) {
        bit_string[0] |= 0x40;
    }

    if (mrt & RESET_DEFAULT_SDMP_ADDRESS) {
        bit_string[0] |= 0x20;
    }
    req.resetOptions.buf = bit_string;
    req.resetOptions.size = 1;
    req.resetOptions.bits_unused = 5;

    *out_size = 0;
    clean_cb_data();
    ec = der_encode(&asn_DEF_EuiccMemoryResetRequest, &req, encode_cb, NULL);
    if(ec.encoded == -1) {
        MSG_ERR("Could not encode: %s\n", ec.failed_type ? ec.failed_type->name : "unknow");
        return RT_ERR_ASN1_ENCODE_FAIL;
    }
    MSG_DUMP_ARRAY("ListNotificationRequest: ", get_cb_data(), get_cb_size());
    RT_CHECK(cmd_store_data(get_cb_data(), get_cb_size(), out, out_size, channel));
    *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}
