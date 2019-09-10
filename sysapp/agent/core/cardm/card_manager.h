
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
#include "lpa.h"

#define THE_MAX_CARD_NUM            20
#define THE_ICCID_LENGTH            10
#define MAX_EID_LEN                 32
#define MAX_EID_HEX_LEN             16

typedef enum _profile_type_e {
    PROFILE_TYPE_TEST               = 0,
    PROFILE_TYPE_PROVISONING        = 1,
    PROFILE_TYPE_OPERATIONAL        = 2,
} profile_type_e;

typedef struct PROFILE_INFO {
    profile_info_t                  info[THE_MAX_CARD_NUM];
    uint8_t                         num;
    uint8_t                         eid[MAX_EID_LEN + 1];
    profile_type_e                  type;
} card_info_t;

int32_t init_card_manager(void *arg);
int32_t card_manager_event(const uint8_t *buf, int32_t len, int32_t mode);

#endif // __CARD_MANAGER_H__
