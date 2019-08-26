
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

int32_t init_card_manager(void *arg);
int32_t card_manager_event(const uint8_t *buf, int32_t len, int32_t mode);

#endif // __CARD_MANAGER_H__
