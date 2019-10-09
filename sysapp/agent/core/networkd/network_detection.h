
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : network_detection.h
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __NETWORK_DETECTION_H__
#define __NETWORK_DETECTION_H__

#include "rt_type.h"

int32_t init_network_detection(void *arg);
void    network_state_update(int32_t timeout);
void    network_state_force_update(int32_t new_state);

#endif // __NETWORK_DETECTION_H__

