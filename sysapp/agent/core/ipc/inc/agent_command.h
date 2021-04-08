
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_command.h
 * Date        : 2021.03.13
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __AGENT_COMMAND_H__
#define __AGENT_COMMAND_H__

#include "rt_type.h"

int32_t init_agent_cmd(void *arg);

uint16_t agent_cmd(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len);

#endif // __AGENT_COMMAND_H__
