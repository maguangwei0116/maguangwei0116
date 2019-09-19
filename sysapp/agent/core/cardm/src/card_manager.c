
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_manager.c
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "card_manager.h"
#include "agent_queue.h"
#include "msg_process.h"
#include "agent_main.h"

static card_info_t g_p_info;

static rt_bool eid_check_memory(const void *buf, int32_t len, int32_t value)
{
    int32_t i = 0;
    const uint8_t *p = (const uint8_t *)buf;

    for (i = 0; i < len; i++) {
        if (p[i] != value) {
            return RT_FALSE;
        }
    }
    return RT_TRUE;
}

static int32_t card_update_eid(void)
{
    int32_t ret = RT_ERROR;

    uint8_t eid[MAX_EID_HEX_LEN] = {0};
    ret = lpa_get_eid(eid);
    bytes2hexstring(eid, sizeof(eid), g_p_info.eid);
    MSG_PRINTF(LOG_INFO, "ret=%d, g_p_info.eid=%s\r\n", ret, g_p_info.eid);

    if (eid_check_memory(g_p_info.eid, 2 * MAX_EID_HEX_LEN, '0')) {
        ret = RT_ERROR;
    }

    return ret;
}

int32_t card_update_profile_info(judge_term_e bootstrap_flag)
{
    int32_t ret = RT_ERROR;
    int32_t i;

    ret = lpa_get_profile_info(g_p_info.info, &g_p_info.num);
    MSG_PRINTF(LOG_INFO, "num:%d\n", g_p_info.num);
    if (ret == RT_SUCCESS) {
        /* get current profile type */
        for (i = 0; i < g_p_info.num; i++) {
            if (g_p_info.info[i].state == 1) {
                g_p_info.type = g_p_info.info[i].class;
                rt_os_memcpy(g_p_info.iccid, g_p_info.info[i].iccid, THE_MAX_CARD_NUM);
                g_p_info.iccid[THE_MAX_CARD_NUM] = '\0';
                break;
            }
        }
        if (i == g_p_info.num) {
            g_p_info.type = PROFILE_TYPE_TEST;
        }
        if ((g_p_info.type == PROFILE_TYPE_TEST) ||
            (g_p_info.type == PROFILE_TYPE_PROVISONING)) {
            if (bootstrap_flag == UPDATE_JUDGE_BOOTSTRAP) {
                msg_send_agent_queue(MSG_ID_BOOT_STRAP, MSG_BOOTSTRAP_SELECT_CARD, NULL, 0);
            }
        }
    }
    return ret;
}

static int32_t card_enable_profile(const uint8_t *iccid)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0;

    for (ii = 0; ii < g_p_info.num; ii++) {
        if (rt_os_strncmp(g_p_info.info[ii].iccid, iccid, THE_ICCID_LENGTH) == 0) {
            if (g_p_info.info[ii].state == 0) {
                if (g_p_info.info[ii].class == PROFILE_TYPE_PROVISONING) {
                    ret = lpa_enable_profile(iccid);
                } else if (g_p_info.info[ii].class == PROFILE_TYPE_OPERATIONAL) {
                    ret = msg_enable_profile(iccid);
                }
                if (ret != RT_SUCCESS) {
                    MSG_PRINTF(LOG_ERR, "Card enable failed ret:%d\n", ret);
                }
                card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
            } else {
                ret = 2;
            }
        }
    }
    msg_send_agent_queue(MSG_ID_NETWORK_DECTION, MSG_ALL_SWITCH_CARD, NULL, 0);
    return ret;
}

static int32_t card_load_profile(const uint8_t *buf, int32_t len)
{
    int32_t ret = RT_SUCCESS;

    ret = card_enable_profile(g_p_info.info[0].iccid);
    rt_os_sleep(1); // must have
    if ((ret == RT_SUCCESS) || (ret == 2)) {
        ret = lpa_load_profile(buf, len);
    }
    rt_os_sleep(1); // must have
    card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
    return ret;
}



static int32_t card_load_cert(const uint8_t *buf, int32_t len)
{
    int32_t ret = RT_ERROR;

    ret = lpa_load_cert(buf, len);
    ret = card_update_eid();
    if (!ret) {
        MSG_PRINTF(LOG_WARN, "EID changed, upload INIT event\n");
        upload_event_report("INIT", NULL, 0, NULL);
        msg_send_agent_queue(MSG_ID_MQTT, MSG_MQTT_SUBSCRIBE_EID, NULL, 0);
    }

    return ret;
}

int32_t init_card_manager(void *arg)
{
    int32_t ret = RT_ERROR;

    ((public_value_list_t *)arg)->card_info = &g_p_info;
    init_msg_process(&g_p_info);
    rt_os_memset(&g_p_info, 0x00, sizeof(g_p_info));
    card_update_eid();
    rt_os_sleep(1);
    ret = card_update_profile_info(UPDATE_JUDGE_BOOTSTRAP);

    return ret;
}

int32_t card_manager_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t ret = RT_ERROR;
    switch (mode) {
        case MSG_CARD_SETTING_KEY:
            break;
        case MSG_CARD_SETTING_PROFILE:
            ret = card_load_profile(buf, len);
            break;
        case MSG_CARD_SETTING_CERTIFICATE:
            ret = card_load_cert(buf, len);
            break;
        case MSG_FROM_MQTT:
            ret = mqtt_msg_event(buf, len);
            break;
        case MSG_NETWORK_DISCONNECTED:
            ret = lpa_get_profile_info(g_p_info.info, &g_p_info.num);
            break;
        case MSG_CARD_ENABLE_EXIST_CARD:
            MSG_PRINTF(LOG_INFO, "iccid:%s, len:%d\n", buf, rt_os_strlen(buf));
            card_enable_profile(buf);
        default:
            MSG_PRINTF(LOG_ERR, "unknow command\n");
            break;
    }
}