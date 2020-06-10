
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

int32_t init_ping_task(void *arg);
int32_t network_detect_event(const uint8_t *buf, int32_t len, int32_t mode);

#endif // __NETWORK_DETECTION_H__

