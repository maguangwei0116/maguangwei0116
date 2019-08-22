
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
#include "lpa.h"

static profile_info_t g_p_info;

int32_t init_card_manager(void *arg)
{
    uint8_t eid[32];
    uint8_t num = 0;
    int32_t ret = RT_ERROR;

    ret = lpa_get_eid(eid);
    MSG_PRINTF(LOG_INFO, "ret:%d\n", ret);
    lpa_get_profile_info(&g_p_info, &num);
    MSG_PRINTF(LOG_INFO, "num:%d\n", num);
}

int32_t card_manager_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    switch (mode) {
        case CARD_MSG_SETTING_KEY:
        break;
        case CARD_MSG_SETTING_PROFILE:
            lpa_load_profile(buf, len);
        break;
        case CARD_MSG_SETTING_CERTIFICATE:
            lpa_load_cert(buf, len);
        break;
        case CARD_MSG_FROM_MQTT:
        break;
        default:
            MSG_PRINTF(LOG_ERR, "unknow command\n");
        break;
    }
}