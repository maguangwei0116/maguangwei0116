
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
    const char *signature       = "123";
    char fileVersion[256]       = {0};

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        ret = RT_ERROR;
        goto exit_entry;
    }
    ret = bootstrap_get_profile_version(NULL, 0, fileVersion, sizeof(fileVersion));
    if (ret != RT_SUCCESS){
        MSG_PRINTF(LOG_WARN, "The file version is error\n");
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(content, imei);
    CJSON_ADD_NEW_STR_OBJ(content, deviceId);
    CJSON_ADD_NEW_STR_OBJ(content, model);
    CJSON_ADD_NEW_STR_OBJ(content, signature);
    CJSON_ADD_NEW_STR_OBJ(content, fileVersion);

    ret = RT_SUCCESS;

exit_entry:

    if (ret) {
        cJSON_free(content);
        content = NULL;
    }

    return content;
}

UPLOAD_EVENT_OBJ_INIT(NO_CERT, TOPIC_DEVICEID, upload_no_cert_packer);

