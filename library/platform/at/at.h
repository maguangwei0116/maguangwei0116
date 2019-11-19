
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : at.h
 * Date        : 2019.11.18
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __AT_H__
#define __AT_H__

#include "rt_type.h"

int32_t init_at(void *arg);
int32_t at_send_recv(const char *cmd, char *rsp, int32_t rsp_len, int32_t timeout_ms);

#endif // __AT_H__

