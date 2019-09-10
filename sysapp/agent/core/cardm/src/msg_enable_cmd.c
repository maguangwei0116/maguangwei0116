
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : msg_enable_cmd.c
 * Date        : 2019.09.09
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "msg_process.h"
#include "downstream.h"
#include "rt_type.h"
#include "cJSON.h"

static cJSON *upload_on_enable_packer(void *arg)
{
    MSG_PRINTF(LOG_WARN, "Upload on enable\n");
exit_entry:
    return (cJSON *)arg;
}

static int32_t enable_parser(const void *in, char *tranid, void **out)
{
    int32_t ret = RT_ERROR;
    cJSON *agent_msg = NULL;
    cJSON *tran_id = NULL;
    cJSON *payload = NULL;
    cJSON *payload_info = NULL;
    cJSON *content = NULL;
    static int8_t md5_out_pro[MD5_STRING_LENGTH + 1];
    int8_t md5_out_now[MD5_STRING_LENGTH + 1];
    uint8_t *buf = NULL;
    int32_t len = 0;

    get_md5_string((int8_t *)in, md5_out_now);
    md5_out_now[MD5_STRING_LENGTH] = '\0';
    if (rt_os_strcmp(md5_out_pro, md5_out_now) == 0) {
        MSG_PRINTF(LOG_ERR, "The data are the same!!\n");
        return ret;
    }
    rt_os_strcpy(md5_out_pro, md5_out_now);

    do {
        agent_msg = cJSON_Parse(in);
        if (!agent_msg) {
            MSG_PRINTF(LOG_ERR, "message parse failed!!\n");
            break;
        }
        tran_id = cJSON_GetObjectItem(agent_msg, "tranId");
        if (!tran_id) {
            MSG_PRINTF(LOG_ERR, "Prase tranId failed!!\n");
            break;
        }
        rt_os_memcpy(tranid, tran_id->valuestring, rt_os_strlen(tran_id->valuestring));
        payload = cJSON_GetObjectItem(agent_msg, "payload");
        if (!payload) {
            MSG_PRINTF(LOG_ERR, "Prase payload failed!!\n");
            break;
        }
        len = rt_os_strlen(payload->valuestring);
        buf = (uint8_t *)rt_os_malloc(len);
        if (!buf) {
            MSG_PRINTF(LOG_ERR, "Malloc buf failed!!\n");
            break;
        }
        rt_os_memcpy(buf, payload->valuestring, len);
        MSG_PRINTF(LOG_INFO, "payload:%s,len:%d\n", buf, len);
        ret = RT_SUCCESS;
    } while(0);
    *out = (void *)buf;
    if (agent_msg != NULL) {
        cJSON_Delete(agent_msg);
    }
    return ret;
}

static int32_t enable_handler(const void *in, const char *event, void **out)
{
    int32_t state = RT_SUCCESS;
    cJSON *payload = NULL;
    cJSON *content = NULL;
    cJSON *content_d = NULL;
    cJSON *iccid = NULL;

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_ERR, "content buffer is empty\n");
        goto end;
    }
    do {
        MSG_PRINTF(LOG_INFO, "payload:%s\n", (uint8_t *)in);
        payload = cJSON_Parse((uint8_t *)in);
        if (!payload) {
            MSG_PRINTF(LOG_ERR, "Parse payload failed!!\n");
            break;
        }
        content_d = cJSON_GetObjectItem(payload, "content");
        if (!content_d) {
            MSG_PRINTF(LOG_ERR, "Parse content failed!!\n");
            break;
        }
        iccid = cJSON_GetObjectItem(content_d, "iccid");
        if (!iccid) {
            MSG_PRINTF(LOG_ERR, "Parse iccid failed!!\n");
            break;
        }
        if (msg_analyse_apn(content_d, iccid->valuestring) == RT_SUCCESS) {
            // return 0;
        }
        state = msg_enable_profile(iccid->valuestring);
        cJSON_AddItemToObject(content, "iccid", cJSON_CreateString(iccid->valuestring));
    } while(0);
    card_update_profile_info(UPDATE_JUDGE_BOOTSTRAP);
    *out = content;
end:
    return state;
}

DOWNSTREAM_METHOD_OBJ_INIT(ENABLE, MSG_ID_CARD_MANAGER, ON_ENABLE, enable_parser, enable_handler);
UPLOAD_EVENT_OBJ_INIT(ON_ENABLE, upload_on_enable_packer);

