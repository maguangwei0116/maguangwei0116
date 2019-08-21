
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_manager.h
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __CARD_MANAGER_H__
#define __CARD_MANAGER_H__

#include "rt_type.h"

typedef enum CARD_MSG_MODE {
    CARD_MSG_SETING_KEY = 0,
    CARD_MSG_SETING_PROFILE,
    CARD_MSG_SETING_CERTIFICATE,
    CARD_MSG_NETWORK_RECONNECTED,
    CARD_MSG_FROM_MQTT
} card_manager_mode_e;

int32_t init_card_manager(void *arg);
int32_t card_manager_event(uint8_t *buf, int32_t len, int32_t mode);

#endif // __CARD_MANAGER_H__
