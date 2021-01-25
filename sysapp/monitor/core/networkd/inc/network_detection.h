
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

typedef enum MODE_TYPE {
    MODE_TYPE_VUICC                 = 0,
    MODE_TYPE_SIM_FIRST             = 1,
    MODE_TYPE_SIM_ONLY              = 2,
    MODE_TYPE_EUICC                 = 3,
} mode_type_e;


void network_detection_task(void);

#endif // __NETWORK_DETECTION_H__
