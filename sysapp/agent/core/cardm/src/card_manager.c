
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
#include "lpa.h"

static profile_info_t g_p_info;

uint8_t g_buf[] = {0x01,0x02,0x03,0x04,0x05};

static int32_t card_load_profile(const uint8_t *buf, int32_t len)
{
    return lpa_load_profile(buf, len);
}

static int32_t card_enabele_profile(const int8_t *iccid)
{
    int32_t ret = RT_ERROR;
    ret = lpa_enable_profile(iccid);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Card enable failed ret:%d\n",ret);
    }
    msg_send_agent_queue(MSG_ID_NETWORK_DECTION, MSG_ALL_SWITCH_CARD, NULL, 0);
    return ret;
}

static int32_t card_load_cert(const uint8_t *buf, int32_t len)
{
    return lpa_load_cert(buf, len);
}

int32_t init_card_manager(void *arg)
{
    uint8_t eid[32];
    uint8_t num = 0;
    int32_t ret = RT_ERROR;

    lpa_get_eid(eid);
    ret = lpa_get_profile_info(&g_p_info, &num);
    if (ret == RT_SUCCESS) {
        if ((g_p_info.class == 1) && (num == 1)) {
            card_enabele_profile(g_p_info.iccid);
            msg_send_agent_queue(MSG_ID_BOOT_STRAP, 0, NULL, 0);
        }
    }
    MSG_PRINTF(LOG_INFO, "num:%d, g_p_info.class:%d, state:%d\n", num, g_p_info.class, g_p_info.state);
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
        case MSG_CARD_FROM_MQTT:
        break;
        default:
            MSG_PRINTF(LOG_ERR, "unknow command\n");
        break;
    }
}