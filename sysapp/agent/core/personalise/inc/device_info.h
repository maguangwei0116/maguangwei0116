
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : device_info.h
 * Date        : 2019.09.05
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __DEVICE_INFO_H__
#define __DEVICE_INFO_H__

#include "rt_type.h"

#define     DEVICE_ID_LEN                32
#define     DEVICE_IMEI_LEN              15
#define     DEVICE_SN_LEN                12
#define     DEVICE_MODEL_LEN             8

typedef struct DEVICE_INFO {
    uint8_t device_id[DEVICE_ID_LEN + 1];
    uint8_t imei[DEVICE_IMEI_LEN + 1];
    uint8_t sn[DEVICE_SN_LEN + 1];
    uint8_t model[DEVICE_MODEL_LEN*4 + 1];
} devicde_info_t;

int32_t init_device_info(void  *arg);

#endif // __DEVICE_INFO_H__
