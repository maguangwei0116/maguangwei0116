
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_flow_control.h
 * Date        : 2019.12.24
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __CARD_FLOW_CONTROL_H__
#define __CARD_FLOW_CONTROL_H__

typedef enum CARD_FLOW_SWITCH {
    CARD_FLOW_DISABLE = 0,
    CARD_FLOW_ENABLE
} card_flow_switch_e;

#include "rt_type.h"

int32_t init_flow_control(void *arg);

#endif // __CARD_FLOW_CONTROL_H__
