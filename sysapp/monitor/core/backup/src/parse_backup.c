
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

static int32_t g_operator_num = 0;

static int32_t insert_profile(const uint8_t *buf, int32_t len)
{
    uint8_t channel;
    uint8_t rsp[512];
    uint16_t rsp_len;
    int32_t ret = RT_ERROR;

    rt_open_channel(&channel);
    ret = cmd_store_data((const uint8_t *)buf, len, rsp, &rsp_len, channel);
    rt_close_channel(channel);

    trigger_swap_card(1);  // reset card

    return ret;
}

/*According rand num to select one profile. Insert vuicc and set apn name*/
static int32_t parse_profile(int32_t rand_num)
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
    MSG_PRINTF(LOG_INFO, "Apn size:%d!!\n", profile_info->apn.list.array[g_operator_num]->apnName.size);
    MSG_PRINTF(LOG_INFO, "Apn name:%s!!\n", profile_info->apn.list.array[g_operator_num]->apnName.buf);
    MSG_PRINTF(LOG_INFO, "Totle number:%d!!\n", profile_info->totalNum);
    MSG_PRINTF(LOG_INFO, "Backup profile size:%d!!\n", profile_file->sharedProfile.optProfiles.list.size);
    MSG_PRINTF(LOG_INFO, "Backup profile count:%d!!\n", profile_file->sharedProfile.optProfiles.list.count);
    MSG_PRINTF(LOG_INFO, "Decode success, backup profile size:%d!!\n", sizeof(card_buf));
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
    return parse_profile((int32_t)rt_get_random_num());
}
