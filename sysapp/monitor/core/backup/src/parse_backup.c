
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : parse_backup.c
 * Date        : 2019.10.16
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "parse_backup.h"
#include "backup_profile.h"
#include "ProfileFile.h"
#include "rt_qmi.h"
#include "apdu.h"
#include "trigger.h"
#include "random.h"
#include "ProfileInfoListResponse.h"
#include "SetRootKeyRequest.h"
#include "tlv.h"

static int32_t g_operator_num = 0;

static uint8_t *g_buf = NULL;
static uint16_t g_buf_size = 0;
static int32_t encode_cb_fun(const void *buffer, size_t size, void *app_key)
{
    g_buf = rt_os_realloc(g_buf, g_buf_size + size);
    if (!g_buf) {
        MSG_PRINTF(LOG_ERR, "realloc failed!!\n");
        return RT_ERROR;
    }
    rt_os_memcpy(g_buf + g_buf_size, (void *) buffer, size);
    g_buf_size += size;
    return RT_SUCCESS;
}

static int32_t insert_profile(const uint8_t *buf, int32_t len)
{
    uint8_t channel;
    uint8_t rsp_buf[2048];
    uint16_t rsp_len;
    uint8_t apdu_enable[] = {0xBF, 0x31, 0x11, 0xA0, 0x0C, 0x5A ,0x0A, 0xFF, \
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x81, 0x01, 0xFF};   // enable profile
    uint8_t apdu_info[] = {0xBF, 0x2D, 0x00};  // get profile list
    int32_t ret = RT_ERROR;
    asn_dec_rval_t dc;
    ProfileInfoListResponse_t *rsp = NULL;
    ProfileInfo_t **p = NULL;
    uint8_t num = 0, i;

    /******************************************************************************/
    uint32_t all_profile_len = 0;
    uint8_t *all_profile_buf = NULL;

    all_profile_len = get_length((uint8_t *)card_buf, 0);
    all_profile_buf = get_value_buffer((uint8_t *)card_buf);

    MSG_INFO_ARRAY("all_profile_buf: ", all_profile_buf, all_profile_len);
    /******************************************************************************/
    uint32_t all_profile_no_hash_len = 0;
    uint8_t *all_profile_no_hash_buf = NULL;

    all_profile_no_hash_len = get_length(all_profile_buf, 0);
    all_profile_no_hash_buf = get_value_buffer(all_profile_buf);

    MSG_INFO_ARRAY("all_profile_no_hash_buf: ", all_profile_no_hash_buf, all_profile_no_hash_len);
    /******************************************************************************/
    uint32_t profile_info_len = 0;
    uint32_t profile_info_tag_len = 0;
    uint8_t *profile_info_buf = NULL;

    profile_info_len = get_length(all_profile_no_hash_buf, 0);
    profile_info_tag_len = get_length(all_profile_no_hash_buf, 1);
    profile_info_buf = get_value_buffer(all_profile_no_hash_buf);

    MSG_INFO_ARRAY("profile_info_buf: ", profile_info_buf, profile_info_len);
    /******************************************************************************/
    //need offset
    uint32_t root_key_len = 0;
    uint32_t root_key_tag_len = 0;
    uint8_t *root_key_buf = NULL;

    root_key_len = get_length(all_profile_no_hash_buf+profile_info_len+profile_info_tag_len, 0);
    root_key_tag_len = get_length(all_profile_no_hash_buf+profile_info_len+profile_info_tag_len, 1);
    root_key_buf = get_value_buffer(all_profile_no_hash_buf+profile_info_len+profile_info_tag_len);

    MSG_INFO_ARRAY("root_key_buf: ", root_key_buf, root_key_len);
    /******************************************************************************/
    //need offset
    uint32_t aes_key_len = 0;
    uint32_t aes_key_tag_len = 0;
    uint8_t *aes_key_buf = NULL;

    aes_key_len = get_length(all_profile_no_hash_buf+profile_info_len+profile_info_tag_len+root_key_len+root_key_tag_len, 0);
    aes_key_tag_len = get_length(all_profile_no_hash_buf+profile_info_len+profile_info_tag_len+root_key_len+root_key_tag_len, 1);
    aes_key_buf = get_value_buffer(all_profile_no_hash_buf+profile_info_len+profile_info_tag_len+root_key_len+root_key_tag_len);

    MSG_INFO_ARRAY("aes_key_buf: ", aes_key_buf, aes_key_len);
    /******************************************************************************/
    asn_enc_rval_t ec;
    SetRootKeyRequest_t key_request = {0};

    key_request.rootEccSk = OCTET_STRING_new_fromBuf(&asn_DEF_SetRootKeyRequest, root_key_buf, root_key_len);
    key_request.rootAesKey = OCTET_STRING_new_fromBuf(&asn_DEF_SetRootKeyRequest, aes_key_buf, aes_key_len);

    g_buf_size = 0;
    ec = der_encode(&asn_DEF_SetRootKeyRequest, &key_request, encode_cb_fun, NULL);
    if (ec.encoded == -1) {
        MSG_PRINTF(LOG_ERR, "encoded:%ld\n", ec.encoded);
    }

    MSG_INFO_ARRAY("g_buf: ", g_buf, g_buf_size);
    /******************************************************************************/
    ret = rt_open_channel(&channel);
    MSG_PRINTF(LOG_INFO, "rt_open_channel ret is : %d \n", ret);
    ret = cmd_store_data(g_buf, g_buf_size, rsp_buf, &rsp_len, channel);
    MSG_PRINTF(LOG_INFO, "root_aes_key_apdu cmd_store_data ret is : %d \n", ret);
    ret = rt_close_channel(channel);
    MSG_PRINTF(LOG_INFO, "rt_close_channel ret is : %d \n", ret);
    rt_os_free(g_buf);
    g_buf = NULL;
    /******************************************************************************/
    rt_open_channel(&channel);
    ret = cmd_store_data((const uint8_t *)apdu_info, 3, rsp_buf, &rsp_len, channel);

    dc = ber_decode(NULL, &asn_DEF_ProfileInfoListResponse, (void **)&rsp, rsp_buf, rsp_len);
    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "Broken ProfileInfoListmResponse decoding at byte %ld\n", (long)dc.consumed);
        ret = RT_ERROR;
        goto end;
    }
    MSG_PRINTF(LOG_INFO, "present: %d\n", rsp->present);
    MSG_PRINTF(LOG_INFO, "count: %d\n", rsp->choice.profileInfoListOk.list.count);

    if (rsp->present != ProfileInfoListResponse_PR_profileInfoListOk) {
        ret = RT_ERROR;
        goto end;
    }

    p = (ProfileInfo_t **)(rsp->choice.profileInfoListOk.list.array);
    num = rsp->choice.profileInfoListOk.list.count;
    for (i = 0; i < num; i++) {
        if (*((p[i])->profileClass) == 1) {
            if (*((p[i])->profileState) != 1) {   // enable provisioning card
                rt_os_memcpy(&apdu_enable[7], (p[i])->iccid->buf, (p[i])->iccid->size);
                MSG_INFO_ARRAY("Enable iccid: ", apdu_enable, sizeof(apdu_enable));
                ret = cmd_store_data((const uint8_t *)apdu_enable, sizeof(apdu_enable), rsp_buf, &rsp_len, channel);
                if (ret != RT_SUCCESS) {
                    MSG_PRINTF(LOG_ERR, "Enable failed!!");
                }
            }
            break;
        }
    }

    ret = cmd_store_data((const uint8_t *)buf, len, rsp_buf, &rsp_len, channel);
    rt_close_channel(channel);

end:
    ASN_STRUCT_FREE(asn_DEF_ProfileInfoListResponse, rsp);
    return ret;
}

static uint32_t get_selecte_profile_index(uint32_t total_num)
{
    static uint32_t g_selected_index = 0xFFFFFFFF;
    uint32_t random;
    uint32_t index;

    while (1) {
        random = (uint32_t)rt_get_random_num();
        index = random % total_num;

        /* never select the last selected card */
        if (g_selected_index != index) {
            g_selected_index = index;
            break;
        }

        rt_os_msleep(100);
    }

    MSG_PRINTF(LOG_INFO, "The selected index/total = [%d/%d], random = %u\n", index+1, total_num, random);
    return index;
}

/*According rand num to select one profile. Insert vuicc and set apn name*/
static int32_t parse_profile(void)
{
    ProfileFile_t *profile_file = NULL;
    ProfileInfo1_t *profile_info = NULL;
    int32_t ret = RT_ERROR;
    uint8_t *profiles = NULL;
    asn_dec_rval_t dc;
    int32_t size;
    int32_t operator_num;
    uint32_t profile_seq;
    char *apn_name = NULL;
    char *mcc_mnc = NULL;

    dc = ber_decode(NULL, &asn_DEF_ProfileFile, (void **) &profile_file, card_buf, sizeof(card_buf));
    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "backup profile decode fail, len: %d, Consumed : %ld\n", sizeof(card_buf), dc.consumed);
        goto end;
    }

    operator_num = profile_file->sharedProfile.fileInfo.operatorNum;
    MSG_PRINTF(LOG_INFO, "Operator num : %d\n", operator_num);
    if (g_operator_num >= operator_num) {
        g_operator_num = 0;
    }

    profile_info = &(profile_file->sharedProfile.optProfiles.list.array[g_operator_num]->profileInfo);
    profiles = profile_file->sharedProfile.optProfiles.list.array[g_operator_num]->content.buf;
    size = profile_file->sharedProfile.optProfiles.list.array[g_operator_num]->content.size;
    size = size / profile_info->totalNum;  // one profile size

    profile_seq = get_selecte_profile_index(profile_info->totalNum);

    MSG_INFO_ARRAY("insert profile buffer: ", profiles + (profile_seq * size), size);

    ret = insert_profile(profiles + (profile_seq * size), size);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "insert profile fail, ret:%d !!\n", ret);
        goto end;
    }

    apn_name = profile_info->apn.list.array[g_operator_num]->apnName.buf;
    mcc_mnc = profile_info->apn.list.array[g_operator_num]->mccMnc.buf;
    rt_qmi_modify_profile(1, 0, 0, apn_name, mcc_mnc);
    MSG_PRINTF(LOG_INFO, "set apn name  : %s [%s]\n", apn_name, mcc_mnc);
    ret = RT_SUCCESS;

end:
    g_operator_num ++;
    if (profile_file != NULL) {
        ASN_STRUCT_FREE(asn_DEF_ProfileFile, profile_file);
    }

    return ret;
}

int32_t backup_process(lpa_channel_type_e channel_mode)
{
    int32_t ret;

    MSG_PRINTF(LOG_INFO, "Begin to select profile from backup-profile ...\r\n");
    ret = parse_profile();

    if (channel_mode == LPA_CHANNEL_BY_IPC) {
        trigger_swap_card(1);  // reset card
    }

    return ret;
}
