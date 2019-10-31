
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : bootstrap.h
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __BOOTSTRAP_H__
#define __BOOTSTRAP_H__

#include "rt_type.h"

int32_t init_bootstrap(void *arg);
void    bootstrap_event(const uint8_t *buf, int32_t len, int32_t mode);
void    bootstrap_monitor_event(const uint8_t *buf, int32_t len, int32_t mode);
int32_t get_aes_key(uint8_t *data);
int32_t get_root_sk(uint8_t *data);
int32_t bootstrap_get_profile_version(char *batch_code, int32_t b_size, char *version, int32_t v_size);

#endif // __BOOTSTRAP_H__
