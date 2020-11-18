
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ping_task.h
 * Date        : 2020.05.10
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __PING_TASK_H__
#define __PING_TASK_H__

#include "rt_type.h"
#include "dial_up.h"

// most times
#define RT_PING_MAX_TIMES           2

// delay
#define RT_EXCELLENT_DELAY          500
#define RT_GOOD_DELAY               1000

// lost
#define RT_EXCELLENT_LOST           2
#define RT_GOOD_LOST                5
#define RT_COMMON_LOST              10
#define RT_PROVISONING_LOST         10

// mdev
#define RT_EXCELLENT_MDEV           200
#define RT_GOOD_MDEV                500

// level
#define RT_COMMON                   1
#define RT_GOOD                     2
#define RT_EXCELLENT                3

#define RT_OR                       0
#define RT_AND                      1

#define RT_CARD_CHANGE_WAIT_TIME    60
#define RT_WAIT_TIME                60
#define RT_DEVICE_TIME              60
#define RT_INIT_TIME                90
#define RT_PROVISONING_IP           "23.91.101.68"

typedef enum REDTEAREADY_CARD_STATUS_CMD {
    SIM_NO_INTERNET                 = 1,
    PROVISONING_NO_INTERNET         = 2,
    OPERATIONAL_NO_INTERNET         = 3,
} redteaready_card_status_cmd_e;

int32_t init_ping_task(void *arg);
int32_t ping_task_network_event(const uint8_t *buf, int32_t len, int32_t mode);
int32_t sync_downstream_event(const uint8_t *buf, int32_t len, int32_t mode);

#endif // __NETWORK_DETECTION_H__
