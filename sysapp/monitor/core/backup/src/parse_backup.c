
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
#include "rt_qmi.h"
#include "apdu.h"
#include "trigger.h"
#include "random.h"
#include "tlv.h"
#include "bertlv.h"

#define TAG_LPA_SET_ROOT_KEY_REQ                0xFF20

static int32_t g_operator_num = 0;

static uint8_t g_buf[10 * 1024];
static uint16_t g_buf_size;

static int32_t insert_profile(const uint8_t *buf, int32_t len)
{
    uint8_t channel;
    uint8_t rsp_buf[2048];
    uint16_t rsp_len;
    uint8_t apdu_enable[] = {0xBF, 0x31, 0x11, 0xA0, 0x0C, 0x5A ,0x0A, 0xFF, \
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x81, 0x01, 0xFF};   // enable profile
    uint8_t apdu_info[] = {0xBF, 0x2D, 0x00};  // get profile list
    int32_t ret = RT_ERROR;
    uint8_t num = 0, i;
    uint32_t offset, pi_offset, list_len, pi_len, iccid_len;
    uint8_t* pi;

    /******************************************************************************/
    uint32_t all_profile_len = 0;
    uint8_t *all_profile_buf = NULL;

    all_profile_buf = (uint8_t*)card_buf + bertlv_get_tl_length((uint8_t*)card_buf, &all_profile_len);

    MSG_INFO_ARRAY("all_profile_buf: ", all_profile_buf, all_profile_len);
    /******************************************************************************/
    uint32_t all_profile_no_hash_len = 0;
    uint8_t *all_profile_no_hash_buf = NULL;

    all_profile_no_hash_buf = all_profile_buf + bertlv_get_tl_length(all_profile_buf, &all_profile_no_hash_len);

    MSG_INFO_ARRAY("all_profile_no_hash_buf: ", all_profile_no_hash_buf, all_profile_no_hash_len);
    /******************************************************************************/
    uint32_t profile_info_len = 0;
    uint32_t profile_info_tag_len = 0;
    uint8_t *profile_info_buf = NULL;

    profile_info_tag_len = bertlv_get_tl_length(all_profile_no_hash_buf, &profile_info_len);
    profile_info_buf = all_profile_no_hash_buf + profile_info_tag_len;

    MSG_INFO_ARRAY("profile_info_buf: ", profile_info_buf, profile_info_len);
    /******************************************************************************/
    //need offset
    uint32_t root_key_len = 0;
    uint32_t root_key_tag_len = 0;
    uint8_t *root_key_buf = NULL;

    root_key_tag_len = bertlv_get_tl_length(profile_info_buf, &root_key_len);
    root_key_buf = profile_info_buf + root_key_tag_len;

    MSG_INFO_ARRAY("root_key_buf: ", root_key_buf, root_key_len);
    /******************************************************************************/
    //need offset
    uint32_t aes_key_len = 0;
    uint32_t aes_key_tag_len = 0;
    uint8_t *aes_key_buf = NULL;

    aes_key_tag_len = bertlv_get_tl_length(root_key_buf + root_key_len, &aes_key_len);
    aes_key_buf = root_key_buf + root_key_len + aes_key_tag_len;

    MSG_INFO_ARRAY("aes_key_buf: ", aes_key_buf, aes_key_len);
    /******************************************************************************/
    // build setRootKeyRequest
    g_buf_size = bertlv_build_tlv(0x80, root_key_len, root_key_buf, g_buf);
    g_buf_size += bertlv_build_tlv(0x81, aes_key_len, aes_key_buf, g_buf + g_buf_size);
    g_buf_size = bertlv_build_tlv(TAG_LPA_SET_ROOT_KEY_REQ, g_buf_size, g_buf, g_buf);

    MSG_INFO_ARRAY("g_buf: ", g_buf, g_buf_size);
    /******************************************************************************/
    ret = rt_open_channel(&channel);
    MSG_PRINTF(LOG_TRACE, "rt_open_channel ret is : %d \n", ret);
    ret = cmd_store_data(g_buf, g_buf_size, rsp_buf, &rsp_len, channel);
    MSG_PRINTF(LOG_TRACE, "root_aes_key_apdu cmd_store_data ret is : %d \n", ret);
    ret = rt_close_channel(channel);

    MSG_PRINTF(LOG_TRACE, "rt_close_channel ret is : %d \n", ret);
    /******************************************************************************/
    rt_open_channel(&channel);
    ret = cmd_store_data((const uint8_t *)apdu_info, 3, rsp_buf, &rsp_len, channel);

    MSG_INFO_ARRAY("list profile info: ", rsp_buf, rsp_len);

    // BF2D
    pi_offset = bertlv_get_tl_length(rsp_buf, NULL);
    // profileInfoListOk A0
    if (bertlv_get_tag(rsp_buf + pi_offset, NULL) != 0xA0) {
        ret = RT_ERROR;
        goto end;
    }
    pi_offset += bertlv_get_tl_length(rsp_buf + pi_offset, &list_len);
    pi = rsp_buf + pi_offset;

    for (i = 1; (pi_offset = bertlv_find_tag(pi, list_len, 0xE3, i)) != BERTLV_INVALID_OFFSET; i++) {
        pi_offset += bertlv_get_tl_length(pi + pi_offset, &pi_len);
        // find profileClass [21]
        offset = bertlv_find_tag(pi + pi_offset, pi_len, 0x95, 1);
        if (offset == BERTLV_INVALID_OFFSET) {
            continue;
        }
        if (bertlv_get_integer(pi + pi_offset + offset, NULL) == 0x01) {
            // find profileState [112]
            offset = bertlv_find_tag(pi + pi_offset, pi_len, 0x9F70, 1);
            if (offset == BERTLV_INVALID_OFFSET) {
                continue;
            }
            if (bertlv_get_integer(pi + pi_offset + offset, NULL) == 0x01) {
                // find iccid
                offset = bertlv_find_tag(pi + pi_offset, pi_len, 0x5A, 1);
                if (offset == BERTLV_INVALID_OFFSET) {
                    break;
                }
                offset += bertlv_get_tl_length(pi + pi_offset + offset, &iccid_len);
                rt_os_memcpy(&apdu_enable[7], pi + pi_offset + offset, iccid_len);
                MSG_INFO_ARRAY("Enable iccid: ", apdu_enable, sizeof(apdu_enable));
                ret = cmd_store_data((const uint8_t*)apdu_enable, sizeof(apdu_enable), rsp_buf, &rsp_len, channel);
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
    int32_t ret = RT_ERROR;
    int32_t operator_num;
    uint32_t profile_seq;
    uint32_t shared_profile_value_off, shared_profile_value_len;
    uint32_t file_info_value_off;
    uint32_t opt_profiles_value_off, opt_profiles_value_len;
    uint32_t profile_info_value_off, profile_info_value_len;
    uint32_t apn_off, apn_len;
    uint32_t apn_name_off, apn_name_len;
    uint32_t mccmnc_off, mccmnc_len;
    uint32_t profiles_off, profiles_len;
    uint32_t profile_off, profile_len;
    uint32_t profile_total_num;
    char *apn_name = NULL;
    char *mcc_mnc = NULL;

    // get value offset of sharedProfile
    shared_profile_value_off  = bertlv_get_tl_length(card_buf, NULL);
    shared_profile_value_off += bertlv_get_tl_length(card_buf + shared_profile_value_off, &shared_profile_value_len);

    // the first TLV is fileInfo. get value offset of fileInfo
    file_info_value_off = bertlv_get_tl_length(card_buf + shared_profile_value_off, NULL);
    file_info_value_off += shared_profile_value_off;
    // the first TLV is operatorNum
    operator_num = bertlv_get_integer(card_buf + file_info_value_off, NULL);

    MSG_PRINTF(LOG_INFO, "Operator num : %d\n", operator_num);
    if (g_operator_num >= operator_num) {
        g_operator_num = 0;
    }

    // find the optProfiles
    opt_profiles_value_off = bertlv_find_tag(card_buf + shared_profile_value_off, shared_profile_value_len, 0xA3, 1);
    opt_profiles_value_off += shared_profile_value_off;
    opt_profiles_value_off += bertlv_get_tl_length(card_buf + opt_profiles_value_off, &opt_profiles_value_len);

    // find the Specified OperatorProfile
    profile_info_value_off = bertlv_find_tag(card_buf + opt_profiles_value_off, shared_profile_value_len, 0x30, g_operator_num + 1);
    opt_profiles_value_off += profile_info_value_off;

    // the first TLV is profileInfo
    profile_info_value_off = bertlv_get_tl_length(card_buf + opt_profiles_value_off, &profile_info_value_len);
    profile_info_value_off += opt_profiles_value_off;

    // find total number
    profile_total_num = bertlv_find_tag(card_buf + profile_info_value_off, profile_info_value_len, 0x81, 1);
    profile_total_num = bertlv_get_integer(card_buf + profile_info_value_off + profile_total_num, NULL);

    // get content
    profiles_off = profile_info_value_off + profile_info_value_len;
    profiles_off += bertlv_get_tl_length(card_buf + profiles_off, &profiles_len);

    profile_seq = get_selecte_profile_index(profile_total_num);

    // get bootstrap profile
    profile_off = bertlv_find_tag(card_buf + profiles_off, profiles_len, 0xFF7F, profile_seq + 1);
    profile_off += profiles_off;
    profile_len = bertlv_get_tlv_length(card_buf + profile_off);

    MSG_INFO_ARRAY("insert profile buffer: ", card_buf + profile_off, profile_len);

    ret = insert_profile(card_buf + profile_off, profile_len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "insert profile fail, ret:%d !!\n", ret);
        goto end;
    }

    // the first TLV of profileInfo is apn
    apn_off = bertlv_get_tl_length(card_buf + profile_info_value_off, &apn_len);
    apn_off += profile_info_value_off;
    // find specified ApnList
    apn_name_off = bertlv_find_tag(card_buf + apn_off, apn_len, 0x30, g_operator_num + 1);
    apn_name_off += apn_off;

    // the first TLV is APN name
    apn_name_off += bertlv_get_tl_length(card_buf + apn_name_off, NULL);  // get apn_name_off
    apn_name_off += bertlv_get_tl_length(card_buf + apn_name_off, &apn_name_len);  // get apn name value off

    // get mcc mnc value off
    mccmnc_off = apn_name_off + apn_name_len;
    mccmnc_off += bertlv_get_tl_length(card_buf + mccmnc_off, &mccmnc_len);

    apn_name = (char *)rt_os_malloc(apn_name_len + 1);
    apn_name[apn_name_len] = '\0';
    mcc_mnc = (char*)rt_os_malloc(mccmnc_len + 1);
    mcc_mnc[mccmnc_len] = '\0';

    rt_qmi_modify_profile(1, 0, 0, apn_name, mcc_mnc);
    MSG_PRINTF(LOG_INFO, "set apn name  : %s [%s]\n", apn_name, mcc_mnc);
    ret = RT_SUCCESS;

end:
    g_operator_num ++;
    if (apn_name != NULL) {
        rt_os_free(apn_name);
    }
    if (mcc_mnc != NULL) {
        rt_os_free(mcc_mnc);
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
