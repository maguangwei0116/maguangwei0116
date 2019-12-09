
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ubi.h
 * Date        : 2019.12.09
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __UBI_H__
#define __UBI_H__

#include "rt_type.h"

typedef int32_t (*ubi_update_func)(const char *ubi_file);
typedef int32_t (*ubi_update_usr_func)(void *func, const char *ubi_file);

typedef enum UBI_UPDATE_RESULT {
    UBI_UPDATE_OK       = 0,
    UBI_UPDATE_FAIL     = -1,
} ubi_update_result_e;

typedef struct UBI_UPDATE_API {
    const char *            module;
    const char *            so_name;
    const char *            api_name;
    ubi_update_usr_func     func;
} ubi_update_api_t;

#define UBI_UPDATE_API_INIT(module, so, api, func)\
    static const ubi_update_api_t ubi_ota_api_##module##_obj \
    __attribute__((section(".ubi.update.api.obj"))) __attribute__((__used__)) = \
    {#module, #so, #api, func}

int32_t ubi_update(const char *ubi_file);

#endif // __UBI_H__

