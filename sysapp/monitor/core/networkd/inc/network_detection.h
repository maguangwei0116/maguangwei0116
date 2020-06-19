
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : network_detection.h
 * Date        : 2019.10.17
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

typedef enum SIM_MODE_TYPE {
    SIM_MODE_TYPE_VUICC_ONLY = 0,     // vUICC mode
    SIM_MODE_TYPE_SIM_FIRST,
    SIM_MODE_TYPE_SIM_ONLY
} sim_mode_type_e;


void network_detection_task(void);

#endif // __NETWORK_DETECTION_H__
