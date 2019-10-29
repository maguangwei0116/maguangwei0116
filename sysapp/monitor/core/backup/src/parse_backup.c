
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
#include "card.h"
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

    rt_open_channel(&channel);
    ret = cmd_store_data((const uint8_t *)apdu_info, 3, rsp_buf, &rsp_len, channel);
    MSG_INFO_ARRAY("profile_list", rsp_buf, rsp_len);

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
                MSG_INFO_ARRAY("Enable iccid", apdu_enable, sizeof(apdu_enable));
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

    trigger_swap_card(1);  // reset card

end:
    ASN_STRUCT_FREE(asn_DEF_ProfileInfoListResponse, rsp);
    return ret;
}

/*According rand num to select one profile. Insert vuicc and set apn name*/
static int32_t parse_profile(uint32_t rand_num)
{
    ProfileFile_t *profile_file = NULL;
    ProfileInfo1_t *profile_info = NULL;
    int32_t ret = RT_ERROR;
    uint8_t *profiles = NULL;
    asn_dec_rval_t dc;
    int32_t size;
    int32_t operator_num;
    int32_t profile_seq;

    dc = ber_decode(NULL, &asn_DEF_ProfileFile, (void **) &profile_file, card_buf, sizeof(card_buf));
    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "Consumed:%ld\n", dc.consumed);
        goto end;
    }

    operator_num = profile_file->sharedProfile.fileInfo.operatorNum;
    MSG_PRINTF(LOG_INFO, "Operator num:%d!!\n", operator_num);
    if (g_operator_num >= operator_num) {
        g_operator_num = 0;
    }

    profile_info = &(profile_file->sharedProfile.optProfiles.list.array[g_operator_num]->profileInfo);
    profiles = profile_file->sharedProfile.optProfiles.list.array[g_operator_num]->content.buf;
    size = profile_file->sharedProfile.optProfiles.list.array[g_operator_num]->content.size;
    size = size / profile_info->totalNum;  // one profile size
    profile_seq = rand_num % profile_info->totalNum;

    MSG_PRINTF(LOG_INFO, "Profiles size:%d!!\n", size);
    MSG_PRINTF(LOG_INFO, "profile_seq:%d!!\n", profile_seq);

    insert_profile(profiles + (profile_seq * size), size);
    MSG_PRINTF(LOG_INFO, "apn:%s\n", profile_info->apn.list.array[g_operator_num]->apnName.buf);
    rt_qmi_modify_profile(1, 0, profile_info->apn.list.array[g_operator_num]->apnName.buf, 0);
    MSG_PRINTF(LOG_INFO, "Apn name:%s!!\n", profile_info->apn.list.array[g_operator_num]->apnName.buf);
    MSG_PRINTF(LOG_INFO, "Totle number:%d!!\n", profile_info->totalNum);
    ret = RT_SUCCESS;

end:
    g_operator_num ++;
    if (profile_file != NULL) {
        ASN_STRUCT_FREE(asn_DEF_ProfileFile, profile_file);
    }

    return ret;
}

int32_t backup_process(void)
{
    return parse_profile((uint32_t)rt_get_random_num());
}
