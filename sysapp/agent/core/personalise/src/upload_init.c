
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : upload_init.c
 * Date        : 2019.09.25
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"
#include "device_info.h"

#include "cJSON.h"

extern const devicde_info_t *g_upload_device_info;

static cJSON *upload_init_packer(void *arg)
{
    int32_t ret         = 0;
    cJSON *deviceInfo   = NULL;
    const char *imei    = g_upload_device_info->imei;
    const char *deviceId= g_upload_device_info->device_id;
    const char *sn      = g_upload_device_info->sn;
    const char *model   = g_upload_device_info->model;

    deviceInfo = cJSON_CreateObject();
    if (!deviceInfo) {
        MSG_PRINTF(LOG_WARN, "The deviceInfo is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(deviceInfo, imei);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, deviceId);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, sn);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, model);
    
    ret = 0;
    
exit_entry:

    return !ret ? deviceInfo : NULL;
}

UPLOAD_EVENT_OBJ_INIT(INIT, upload_init_packer);
