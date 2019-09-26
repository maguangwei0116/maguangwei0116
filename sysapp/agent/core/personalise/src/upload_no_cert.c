
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : upload_no_cert.c
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
#include "profile_parse.h"
#include "file.h"
#include "bootstrap.h"

#include "cJSON.h"

extern const devicde_info_t *g_personalise_device_info;

static cJSON *upload_no_cert_packer(void *arg)
{
    int32_t ret                 = RT_ERROR;
    cJSON *content              = NULL;
    const char *imei            = g_personalise_device_info->imei;
    const char *deviceId        = g_personalise_device_info->device_id;
    const char *model           = g_personalise_device_info->model;
    uint8_t fileVersion[128]    = {0};

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        ret = RT_ERROR;
        goto exit_entry;
    }
    ret = get_file_version(fileVersion);
    if (ret != RT_SUCCESS){
        MSG_PRINTF(LOG_WARN, "The fileVersion is error\n");
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(content, imei);
    CJSON_ADD_NEW_STR_OBJ(content, deviceId);
    CJSON_ADD_NEW_STR_OBJ(content, model);
    CJSON_ADD_NEW_STR_OBJ(content, fileVersion);

    ret = RT_SUCCESS;

exit_entry:
    return !ret ? content : NULL;
}

UPLOAD_EVENT_OBJ_INIT(NO_CERT, TOPIC_DEVICEID, upload_no_cert_packer);

