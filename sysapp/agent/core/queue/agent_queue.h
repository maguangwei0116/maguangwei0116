
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
    MSG_ID_BOOT_STRAP = 0,
    MSG_ID_CARD_MANAGER,
    MSG_ID_LOG_MANAGER,
    MSG_ID_OTA_UPGRADE,
    MSG_ID_PERSONLLISE,
    MSG_ID_REMOTE_CONFIG
} agent_msg_id_e;

int32_t init_queue(void *arg);
int32_t msg_send_agent_queue(int32_t msgid, void *buffer, int32_t len);

#endif // __AGENT_QUEUE_H__
