
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

static int32_t g_operator_num = 0;

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

    uint8_t root_aes_key_apdu[] = {0xFF, 0x20, 0x82, 0x01, 0x40, 0x80, 0x81, 0xA5, 0x66, 0x7B, 0x5C, \
           0xFA, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, \
           0x23, 0xA1, 0x80, 0x2B, 0x19, 0xD5, 0x3B, 0x1D, 0xAA, 0xD2, 0x06, 0x66, 0xFF, 0x05, 0x10, \
           0xFF, 0x14, 0x92, 0x9C, 0x6A, 0xBE, 0x48, 0x10, 0x2A, 0xE3, 0x91, 0x09, 0x83, 0x0C, 0x96, \
           0xA2, 0x19, 0x58, 0xB0, 0x52, 0xA7, 0xAA, 0x23, 0x63, 0xF6, 0x3E, 0x62, 0xED, 0x4A, 0xCE, \
           0x37, 0x6B, 0x00, 0x00, 0x00, 0x01, 0x21, 0xC7, 0xEE, 0xD3, 0xE2, 0x21, 0xE7, 0x36, 0x14, \
           0x69, 0xD0, 0x93, 0xB0, 0xC9, 0x33, 0x44, 0x56, 0x06, 0x49, 0xB2, 0x4A, 0x1D, 0x50, 0x0E, \
           0x00, 0x00, 0x00, 0x20, 0x15, 0xE0, 0xD4, 0xBE, 0xB9, 0x93, 0x99, 0x6D, 0x0B, 0x42, 0x61, \
           0x54, 0x8B, 0x15, 0xF9, 0xAD, 0x54, 0xAA, 0xB5, 0x0A, 0x1D, 0xF3, 0x2C, 0xB6, 0x57, 0x98, \
           0x2D, 0x05, 0xBC, 0x9C, 0x52, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9, 0x78, 0xE1, 0x98, 0x83, 0x61, 0x50, 0xE4, \
           0xAF, 0xB3, 0xF9, 0xFA, 0x1D, 0x90, 0xE1, 0xFB, 0xF8, 0xE7, 0xDF, 0x53, 0x81, 0x81, 0x95, \
           0x0A, 0x66, 0xAB, 0x1C, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, \
           0x00, 0x00, 0x09, 0x23, 0xA1, 0x80, 0x2B, 0x19, 0xD5, 0x3B, 0x1D, 0xAA, 0xD2, 0x06, 0x66, \
           0xFF, 0x05, 0x10, 0xFF, 0x14, 0x92, 0x9C, 0x6A, 0xBE, 0x48, 0x10, 0x2A, 0xE3, 0x91, 0x09, \
           0x83, 0x0C, 0x96, 0xA2, 0x03, 0x46, 0xFA, 0xFB, 0x0F, 0xFF, 0x6B, 0xBD, 0x56, 0x53, 0x4C, \
           0xD8, 0x7B, 0x3C, 0xF8, 0x56, 0x00, 0x00, 0x00, 0x01, 0x65, 0x29, 0xDD, 0x6C, 0x15, 0xED, \
           0x6F, 0x0E, 0x54, 0x79, 0xFB, 0xD4, 0x3B, 0x41, 0x24, 0x7F, 0x3F, 0x64, 0x20, 0xDF, 0x3F, \
           0xF4, 0xC7, 0xCB, 0x00, 0x00, 0x00, 0x10, 0x98, 0x7D, 0x65, 0x0A, 0x41, 0x9E, 0x1D, 0x6C, \
           0x76, 0xD0, 0x2B, 0xEE, 0x7E, 0x12, 0x0B, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE9, 0x0E, 0x7E, 0xC4, 0x6F, 0x7E, \
           0xAD, 0xAD, 0x52, 0xA7, 0x1A, 0x01, 0x98, 0xED, 0xA1, 0xAE, 0x8C, 0x3A, 0x34, 0xE4};

    ret = rt_open_channel(&channel);
    MSG_PRINTF(LOG_INFO, "rt_open_channel ret is : %d \n", ret);
    ret = cmd_store_data(root_aes_key_apdu, sizeof(root_aes_key_apdu), rsp_buf, &rsp_len, channel);
    MSG_PRINTF(LOG_INFO, "root_aes_key_apdu cmd_store_data ret is : %d \n", ret);
    ret = rt_close_channel(channel);
    MSG_PRINTF(LOG_INFO, "rt_close_channel ret is : %d \n", ret);

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
