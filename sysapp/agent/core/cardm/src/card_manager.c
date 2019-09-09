
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

static int32_t card_enable_profile(const int8_t *iccid)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0;

    for (ii = 0; ii < g_p_info.num; ii++) {
        if (rt_os_strncmp(g_p_info.info[ii].iccid, iccid, THE_ICCID_LENGTH) == 0) {
            if (g_p_info.info[ii].state == 0) {
                ret = lpa_enable_profile(iccid);
                if (ret != RT_SUCCESS) {
                    MSG_PRINTF(LOG_ERR, "Card enable failed ret:%d\n", ret);
                }
            } else {
                ret = 2;
            }
        }
    }
    lpa_get_profile_info(g_p_info.info, &g_p_info.num);
    msg_send_agent_queue(MSG_ID_NETWORK_DECTION, MSG_ALL_SWITCH_CARD, NULL, 0);

    return ret;
}

static int32_t card_load_profile(const uint8_t *buf, int32_t len)
{
    int32_t ret = RT_SUCCESS;

    ret = card_enable_profile(g_p_info.info[0].iccid);
    rt_os_sleep(3); // must have
    if ((ret == RT_SUCCESS) || (ret == 2)) {
        ret = lpa_load_profile(buf, len);
    }

    return ret;
}

static int32_t card_load_cert(const uint8_t *buf, int32_t len)
{
    return lpa_load_cert(buf, len);
}

int32_t init_card_manager(void *arg)
{
    int32_t ret = RT_ERROR;
    uint8_t eid[16];
    int32_t i;
    
    ((public_value_list_t *)arg)->card_info = &g_p_info;

    lpa_get_eid(eid);
    bytes2hexstring(eid, sizeof(eid), g_p_info.eid);
    
    rt_os_sleep(1);

    ret = lpa_get_profile_info(g_p_info.info, &g_p_info.num);
    MSG_PRINTF(LOG_INFO, "num:%d\n", g_p_info.num);
    if (ret == RT_SUCCESS) {
        if ((g_p_info.info[0].class == 1) && (g_p_info.num == 1)) {
            msg_send_agent_queue(MSG_ID_BOOT_STRAP, 0, NULL, 0);
        }

        /* get current profile type */
        for (i = 0; i < g_p_info.num; i++) {
            if (g_p_info.info[i].state == 1) {
                g_p_info.type = g_p_info.info[i].class;
                break;
            }
        }
    }

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
            ret = card_deal_mqtt_msg(buf, len);
            break;
        case MSG_NETWORK_DISCONNECTED:
            ret = lpa_get_profile_info(g_p_info.info, &g_p_info.num);
            break;
        default:
            MSG_PRINTF(LOG_ERR, "unknow command\n");
            break;
    }
}