#include "luid.h"
#include "lpdd.h"
#include "lpa_config.h"
#include "lpa_error_codes.h"
#include "lpa.h"
#include "apdu.h"
#include "ber_tlv.h"

int enable_profile(profile_id_t pid, uint8_t id[16], bool refresh, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    uint8_t refresh_value;
    int ret = RT_SUCCESS;

    /*
    EnableProfileRequest ::= [49] SEQUENCE { -- Tag 'BF31' 
        profileIdentifier CHOICE { 
            isdpAid [APPLICATION 15] OctetTo16, -- AID, tag '4F' 
            iccid Iccid -- ICCID, tag '5A' 
        }, 
        refreshFlag BOOLEAN -- indicating whether REFRESH is required
    }
    */

    if (pid == PID_ISDP_AID) {
        // isdpAid [APPLICATION 15] OctetTo16
        g_buf_size = ber_tlv_build_tlv(0x4F, 16, id, g_buf);
    } else if (pid == PID_ICCID) {
        // iccid Iccid
        g_buf_size = ber_tlv_build_tlv(0x5A, 10, id, g_buf);
    }
    // profileIdentifier CHOICE
    g_buf_size = ber_tlv_build_tlv(0xA0, g_buf_size, g_buf, g_buf);
    // refreshFlag BOOLEAN
    refresh_value = refresh ? 0xFF : 0x00;
    g_buf_size += ber_tlv_build_tlv(0x81, 1, &refresh_value, g_buf + g_buf_size);
    // EnableProfileRequest -- Tag 'BF31' 
    g_buf_size = ber_tlv_build_tlv(TAG_LPA_ENABLE_PROFILE_REQ, g_buf_size, g_buf, g_buf);
    MSG_DUMP_ARRAY("EnableProfileRequest: ", g_buf, g_buf_size);

    *out_size = 0;
    ret = cmd_store_data(g_buf, g_buf_size, out, out_size, channel);
    if (ret == RT_SUCCESS){
        // *out_size -= 2;  // Remove sw 9000
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
    uint8_t refresh_value;
    int ret = RT_SUCCESS;

    /*
    DisableProfileRequest ::= [50] SEQUENCE { -- Tag 'BF32' 
        profileIdentifier CHOICE { 
            isdpAid [APPLICATION 15] OctetTo16, -- AID, tag '4F'
            iccid Iccid -- ICCID, tag '5A' 
        } 
        refreshFlag BOOLEAN -- indicating whether REFRESH is required 
    }
    */

    if (pid == PID_ISDP_AID) {
        // isdpAid [APPLICATION 15] OctetTo16
        g_buf_size = ber_tlv_build_tlv(0x4F, 16, id, g_buf);
    } else if (pid == PID_ICCID) {
        // iccid Iccid
        g_buf_size = ber_tlv_build_tlv(0x5A, 10, id, g_buf);
    }

    // profileIdentifier CHOICE
    g_buf_size = ber_tlv_build_tlv(0xA0, g_buf_size, g_buf, g_buf);
    // refreshFlag BOOLEAN
    refresh_value = refresh ? 0xFF : 0x00;
    g_buf_size += ber_tlv_build_tlv(0x81, 1, &refresh_value, g_buf + g_buf_size);
    // DisableProfileRequest -- Tag 'BF32'
    g_buf_size = ber_tlv_build_tlv(TAG_LPA_DISABLE_PROFILE_REQ, g_buf_size, g_buf, g_buf);
    MSG_DUMP_ARRAY("EnableProfileRequest: ", g_buf, g_buf_size);

    *out_size = 0;
    ret = cmd_store_data(g_buf, g_buf_size, out, out_size, channel);
    if (ret == RT_SUCCESS){
        // *out_size -= 2;  // Remove sw 9000
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
    /*
    DeleteProfileRequest ::= [51] CHOICE { -- Tag 'BF33' 
        isdpAid [APPLICATION 15] OctetTo16, -- AID, tag '4F' 
        iccid Iccid -- ICCID, tag '5A' 
    }
    */

    if (pid == PID_ISDP_AID) {
        // isdpAid [APPLICATION 15] OctetTo16
        g_buf_size = ber_tlv_build_tlv(0x4F, 16, id, g_buf);
    } else if (pid == PID_ICCID) {
        // iccid Iccid
        g_buf_size = ber_tlv_build_tlv(0x5A, 10, id, g_buf);
    }
    // DeleteProfileRequest -- Tag 'BF33'
    g_buf_size = ber_tlv_build_tlv(TAG_LPA_DELETE_PROFILE_REQ, g_buf_size, g_buf, g_buf);
    MSG_DUMP_ARRAY("DeleteProfileRequest: ", g_buf, g_buf_size);

    *out_size = 0;
    RT_CHECK(cmd_store_data(g_buf, g_buf_size, out, out_size, channel));
    // *out_size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int get_eid(uint8_t *eid, uint16_t *size, uint8_t channel)
{
    /*
    GetEuiccDataRequest ::= [62] SEQUENCE { -- Tag 'BF3E' 
        tagList [APPLICATION 28] Octet1 -- tag '5C', the value SHALL be set to '5A' 
    }
    */

    g_buf[0] = 0x5A;
    g_buf_size = ber_tlv_build_tlv(0x5C, 1, g_buf, g_buf);
    g_buf_size = ber_tlv_build_tlv(TAG_LPA_GET_EUICC_DATA_REQ, g_buf_size, g_buf, g_buf);

    MSG_DUMP_ARRAY("GetEuiccDataRequest: ", g_buf, g_buf_size);
    RT_CHECK(cmd_store_data(g_buf, g_buf_size, eid, size, channel));
    MSG_DBG("get_eid  *size: %d\n", *size);
    // *size -= 2;  // Remove sw 9000

    return RT_SUCCESS;
}

int switch_eid(uint8_t *eid, uint16_t size,uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    // no available now
    return RT_SUCCESS;
}

int get_eid_list(uint8_t *eid, uint16_t *size, uint8_t channel)
{
    // no available now
    return RT_SUCCESS;
}

int get_profiles_info(search_criteria_t sc, uint8_t *criteria, uint16_t c_size,
                    uint8_t *profile_info, uint16_t *size , uint8_t channel/* out */)
{
    int ret = RT_SUCCESS;

    /*
    ProfileInfoListRequest ::= [45] SEQUENCE { -- Tag 'BF2D' 
        searchCriteria [0] CHOICE { 
            isdpAid [APPLICATION 15] OctetTo16, -- AID of the ISD-P, tag '4F' 
            iccid Iccid, -- ICCID, tag '5A' 
            profileClass [21] ProfileClass -- Tag '95' 
        } OPTIONAL, 
        tagList [APPLICATION 28] OCTET STRING OPTIONAL -- tag '5C' 
    }
    */

    g_buf_size = 0;
    if (sc != SEARCH_NONE) {
        if (sc == SEARCH_ISDP_AID) {
            // isdpAid [APPLICATION 15] OctetTo16
            g_buf_size = ber_tlv_build_tlv(0x4F, c_size, criteria, g_buf);
        } else if (sc == SEARCH_ICCID) {
            // iccid Iccid
            g_buf_size = ber_tlv_build_tlv(0x5A, c_size, criteria, g_buf);
        } else if (sc == SEARCH_PROFILE_CLASS) {
            // profileClass [21] ProfileClass
            g_buf_size = ber_tlv_build_tlv(0x95, c_size, criteria, g_buf);
        }
        // searchCriteria [0] CHOICE
        g_buf_size = ber_tlv_build_tlv(0xA0, g_buf_size, g_buf, g_buf);
    }

    // ProfileInfoListRequest -- Tag 'BF2D' 
    g_buf_size = ber_tlv_build_tlv(TAG_LPA_PROFILE_INFO_LIST_REQ, g_buf_size, g_buf, g_buf);

    MSG_DUMP_ARRAY("ProfileInfoListRequest_t: ", g_buf, g_buf_size);
    
    ret = cmd_store_data(g_buf, g_buf_size, profile_info, size, channel);
    RT_CHECK_GO(ret == RT_SUCCESS, ret, end);

end:

    return ret;
}

// TODO: Fix 6A80
int set_nickname(uint8_t iccid[10], const char *nickname, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    /*
    -- Definition of Profile Nickname Information 
    SetNicknameRequest ::= [41] SEQUENCE { -- Tag 'BF29' 
        iccid Iccid, 
        profileNickname [16] UTF8String (SIZE(0..64)) 
    }
    */

    // iccid Iccid
    g_buf_size = ber_tlv_build_tlv(0x5A, 10, iccid, g_buf);
    // profileNickname [16] UTF8String (SIZE(0..64))
    g_buf_size += ber_tlv_build_tlv(0x90, strlen(nickname), nickname, g_buf + g_buf_size);

    // ProfileInfoListRequest -- Tag 'BF2D' 
    g_buf_size = ber_tlv_build_tlv(TAG_LPA_SET_NICK_NAME_REQ, g_buf_size, g_buf, g_buf);

    MSG_DUMP_ARRAY("GetEuiccInfo1Request: ", g_buf, g_buf_size);
    RT_CHECK(cmd_store_data(g_buf, g_buf_size, out, out_size, channel));

    return RT_SUCCESS;
}

int euicc_memory_reset(memory_reset_t mrt, uint8_t *out, uint16_t *out_size, uint8_t channel)
{
    uint8_t bit_string[2] = {0};

    /*
    EuiccMemoryResetRequest ::= [52] SEQUENCE { -- Tag 'BF34' 
        resetOptions [2] BIT STRING { 
            deleteOperationalProfiles(0), 
            deleteFieldLoadedTestProfiles(1), 
            resetDefaultSmdpAddress(2)
        } 
    }
    */

    if (mrt & RESET_OPERATIONAL_PROFILE) {
        bit_string[0] = 7;
        bit_string[1] |= 0x80;
    }

    if (mrt & RESET_TEST_PROFILE) {
        bit_string[0] = 6;
        bit_string[1] |= 0x40;
    }

    if (mrt & RESET_DEFAULT_SDMP_ADDRESS) {
        bit_string[0] = 5;
        bit_string[1] |= 0x20;
    }

    // resetOptions [2] BIT STRING
    g_buf_size = ber_tlv_build_tlv(0x82, 2, bit_string, g_buf);

    // EuiccMemoryResetRequest -- Tag 'BF34' 
    g_buf_size = ber_tlv_build_tlv(TAG_LPA_EUICC_MEMORY_RESET_REQ, g_buf_size, g_buf, g_buf);

    *out_size = 0;
    MSG_DUMP_ARRAY("ListNotificationRequest: ", g_buf, g_buf_size);
    RT_CHECK(cmd_store_data(g_buf, g_buf_size, out, out_size, channel));

    return RT_SUCCESS;
}
