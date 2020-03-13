
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
#include "convert.h"

#define SIGN_DATA_LEN         64
extern const devicde_info_t *g_personalise_device_info;

static cJSON *upload_no_cert_packer(void *arg)
{
    int32_t ret                 = RT_ERROR;
    uint16_t len = 0;
    cJSON *content              = NULL;
    uint8_t sign_apdu[] = {0xFF, 0x21, 0x12, 0x80, 0x10, 0x72, 0x65, 0x64, 0x74, \
          0x65, 0x61, 0x6D, 0x6F, 0x62, 0x69, 0x6C, 0x65, 0x6F, 0x62, 0x69, 0x6C};
    uint8_t buf[256];
    uint8_t buf_temp[256];

    const char *imei            = g_personalise_device_info->imei;
    const char *deviceId        = g_personalise_device_info->device_id;
    const char *model           = g_personalise_device_info->model;
    const char *signature       = buf_temp;
    char fileVersion[256]       = {0};

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        goto exit_entry;
    }
    ret = bootstrap_get_profile_version(NULL, 0, fileVersion, sizeof(fileVersion));
    if (ret != RT_SUCCESS){
        MSG_PRINTF(LOG_WARN, "The file version is error\n");
        goto exit_entry;
    }
    hexstring2bytes(deviceId, &sign_apdu[5],(uint16_t *)&len);
    ret = lpa_load_customized_data(sign_apdu, sizeof(sign_apdu), buf, &len);
    if ((ret != 0) && (len < SIGN_DATA_LEN)) {
        MSG_PRINTF(LOG_ERR, "Sign data error\n");
        goto exit_entry;
    }
    if (buf[3] == 0x80) {
        len = buf[4];
    }
    MSG_PRINTF(LOG_INFO, "sign len:%d\n", len);
    rt_base64_encode(&buf[5], len, buf_temp);

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

