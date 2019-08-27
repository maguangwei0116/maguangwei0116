
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_quectel.h
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __AGENT_QUEUE_H__
#define __AGENT_QUEUE_H__

#include "rt_type.h"

typedef enum AGENT_MSG_ID {
    MSG_ID_CARD_MANAGER = 0,
    MSG_ID_LOG_MANAGER,
    MSG_ID_BOOT_STRAP,
    MSG_ID_OTA_UPGRADE,
    MSG_ID_PERSONLLISE,
    MSG_ID_NETWORK_DECTION,
    MSG_ID_REMOTE_CONFIG,
    MSG_ID_BROAD_CAST_NETWORK
} msg_id_e;

typedef enum MSG_MODE {
    MSG_CARD_FROM_MQTT = 0,
    MSG_CARD_SETTING_KEY,
    MSG_CARD_SETTING_PROFILE,
    MSG_CARD_SETTING_CERTIFICATE,
    MSG_ALL_SWITCH_CARD,
    MSG_NETWORK_RECONNECTED,
} msg_mode_e;

int32_t init_queue(void *arg);
int32_t msg_send_agent_queue(int32_t msgid, int32_t mode, void *buffer, int32_t len);
int32_t msg_send_upload_queue(void *buffer, int32_t len);
#endif // __AGENT_QUEUE_H__
