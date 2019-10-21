
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

static int32_t insert_profile(const uint8_t *buf, int32_t len)
{
    uint8_t channel;
    uint8_t rsp[512];
    uint16_t rsp_len;
    int32_t ret = RT_ERROR;

    rt_open_channel(&channel);
    ret = cmd_store_data((const uint8_t *)buf, len, rsp, &rsp_len, channel);
    rt_close_channel(channel);

    return ret;
}

static int32_t parse_profile(int32_t rand_num)
{
    ProfileFile_t *profile_file = NULL;
    ProfileInfo1_t *profile_info = NULL;
    uint8_t *profiles = NULL;
    int32_t size;
    asn_dec_rval_t dc;
    int32_t ret = RT_ERROR;

    dc = ber_decode(NULL, &asn_DEF_ProfileFile, (void **) &profile_file, card_buf, sizeof(card_buf));
    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "Consumed:%ld\n", dc.consumed);
        goto end;
    }

    profile_info = &(profile_file->sharedProfile.optProfiles.list.array[0]->profileInfo);
    profiles = profile_file->sharedProfile.optProfiles.list.array[0]->content.buf;
    size = profile_file->sharedProfile.optProfiles.list.array[0]->content.size;
    size = size / profile_info->totalNum;  // one profile size

    MSG_PRINTF(LOG_INFO, "Profiles size:%d!!\n", size);

    insert_profile(profiles, size);
    MSG_PRINTF(LOG_INFO, "apn:%s\n", profile_info->apn.list.array[0]->apnName.buf);
    rt_qmi_modify_profile(1, 0, profile_info->apn.list.array[0]->apnName.buf, 0);
    MSG_PRINTF(LOG_INFO, "Apn size:%d!!\n", profile_info->apn.list.array[0]->apnName.size);
    MSG_PRINTF(LOG_INFO, "Apn name:%s!!\n", profile_info->apn.list.array[0]->apnName.buf);
    MSG_PRINTF(LOG_INFO, "Totle number:%d!!\n", profile_info->totalNum);
    MSG_PRINTF(LOG_INFO, "Backup profile size:%d!!\n", profile_file->sharedProfile.optProfiles.list.size);
    MSG_PRINTF(LOG_INFO, "Backup profile count:%d!!\n", profile_file->sharedProfile.optProfiles.list.count);
    MSG_PRINTF(LOG_INFO, "Decode success, backup profile size:%d!!\n", sizeof(card_buf));

end:
    if (profile_file != NULL) {
        ASN_STRUCT_FREE(asn_DEF_ProfileFile, profile_file);
    }

    return ret;
}

int32_t backup_process(lpa_channel_type_e type)
{
    init_apdu_channel(type);
    return parse_profile(1);
}
