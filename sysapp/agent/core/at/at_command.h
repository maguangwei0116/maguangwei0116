
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : at_command.h
 * Date        : 2019.12.10
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __AT_COMMNAD_H__
#define __AT_COMMNAD_H__

#include "rt_type.h"

int32_t init_at_command(void *arg);
int32_t uicc_switch_card(profile_type_e type, uint8_t *iccid);

#endif // __AT_COMMNAD_H__

