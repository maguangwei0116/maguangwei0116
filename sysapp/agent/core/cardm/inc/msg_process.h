
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : msg_process.h
 * Date        : 2019.09.04
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __MSG_PROCESS_H__
#define __MSG_PROCESS_H__

#include "rt_type.h"

int32_t msg_push_ac(const uint8_t *buf, int32_t len);
int32_t msg_enable(const uint8_t *buf, int32_t len);
int32_t msg_disable(const uint8_t *buf, int32_t len);
int32_t msg_delete(const uint8_t *buf, int32_t len);

#endif // __MSG_PROCESS_H__
