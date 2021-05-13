
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_prov_ctrl.h
 * Date        : 2021.04.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __CARD_PROV_CTRL_H__
#define __CARD_PROV_CTRL_H__

#include "rt_type.h"
#include "card_manager.h"

int32_t init_card_prov_ctrl(void *arg);
int32_t card_prov_ctrl_event(const uint8_t *buf, int32_t len, int32_t mode);
rt_bool card_prov_ctrl_increase(init_profile_type_e type);
rt_bool card_prov_ctrl_judgement(init_profile_type_e type);

#endif // __CARD_PROV_CTRL_H__
