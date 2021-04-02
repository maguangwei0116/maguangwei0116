
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

#if 0
IMEI less than 15 bytes, model less than 8 bytes, serialNo less than 12 bytes; fill up with 0x00
#endif

#define MAX_DEVICE_ID_LEN               32
#define MAX_DEVICE_IMEI_LEN             15
#define MAX_DEVICE_MODEL_LEN            8
#define MAX_DEVICE_SN_LEN               12

typedef enum CARD_TYPE {
    CARD_TYPE_vUICC               	= 0,
    CARD_TYPE_eUICC               	= 1,
} card_type_e;

typedef struct DEVICE_INFO {
    uint8_t device_id[MAX_DEVICE_ID_LEN + 1];
    uint8_t imei[MAX_DEVICE_IMEI_LEN + 1];
    uint8_t sn[MAX_DEVICE_SN_LEN + 1];
    uint8_t model[MAX_DEVICE_MODEL_LEN*4 + 1];
    card_type_e card_type;
} devicde_info_t;

int32_t init_device_info(void  *arg);

#endif // __DEVICE_INFO_H__
