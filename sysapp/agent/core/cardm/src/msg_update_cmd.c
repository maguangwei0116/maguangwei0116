
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : msg_update_cmd.c
 * Date        : 2020.04.21
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "agent_queue.h"
#include "msg_process.h"
#include "downstream.h"
#include "usrdata.h"
#include "rt_type.h"
#include "cJSON.h"
#include "md5.h"

static cJSON *upload_on_update_packer(void *arg)
{
    MSG_PRINTF(LOG_DBG, "Upload on update\n");
exit_entry:
    return (cJSON *)arg;
}

static int32_t update_parser(const void *in, char *tranid, void **out)
{
    int32_t ret = RT_ERROR;
    int32_t len = 0;
    static int8_t md5_out_pro[MD5_STRING_LENGTH + 1];
    int8_t md5_out_now[MD5_STRING_LENGTH + 1];
    uint8_t *buf = NULL;
    cJSON *agent_msg = NULL;
    cJSON *tran_id = NULL;
    cJSON *payload = NULL;
    cJSON *payload_info = NULL;
    cJSON *content = NULL;

    get_md5_string((int8_t *)in, md5_out_now);
    md5_out_now[MD5_STRING_LENGTH] = '\0';
    if (rt_os_strcmp(md5_out_pro, md5_out_now) == 0) {
        MSG_PRINTF(LOG_DBG, "The data are the same!!\n");
        return ret;
    }
    rt_os_strcpy(md5_out_pro, md5_out_now);

    do {
        agent_msg = cJSON_Parse(in);
        if (!agent_msg) {
            MSG_PRINTF(LOG_WARN, "message parse failed!!\n");
            break;
        }
        tran_id = cJSON_GetObjectItem(agent_msg, "tranId");
        if (!tran_id) {
            MSG_PRINTF(LOG_WARN, "Prase tranId failed!!\n");
            break;
        }
        rt_os_memcpy(tranid, tran_id->valuestring, rt_os_strlen(tran_id->valuestring));
        payload = cJSON_GetObjectItem(agent_msg, "payload");
        if (!payload) {
            MSG_PRINTF(LOG_WARN, "Prase payload failed!!\n");
            break;
        }
        len = rt_os_strlen(payload->valuestring);
        buf = (uint8_t *)rt_os_malloc(len + 1);
        if (!buf) {
            MSG_PRINTF(LOG_WARN, "Malloc buf failed!!\n");
            break;
        }
        rt_os_memcpy(buf, payload->valuestring, len);
        buf[len] = '\0';
        MSG_PRINTF(LOG_TRACE, "payload:%s,len:%d\n", buf, len);
        ret = RT_SUCCESS;
    } while(0);

    *out = (void *)buf;
    if (agent_msg != NULL) {
        cJSON_Delete(agent_msg);
    }
    return ret;
}

static int32_t update_handler(const void *in, const char *event, void **out)
{
    cJSON *payload = NULL;
    cJSON *content = NULL;
    cJSON *content_d = NULL;
    cJSON *properties = NULL;
    cJSON *switchparams= NULL;
    cJSON *card_type = NULL;
    cJSON *apnparams_list = NULL;
    cJSON *monitorstrategyparams = NULL;
    uint8_t *content_s;
    int32_t state = RT_ERROR;

    // MSG_PRINTF(LOG_INFO, "payload:%s\n", (uint8_t *)in);
    payload = cJSON_Parse((uint8_t *)in);
    if (!payload) {
        MSG_PRINTF(LOG_WARN, "Parse payload failed!!\n");
        goto end;
    }
    content_d = cJSON_GetObjectItem(payload, "content");
    if (!content_d) {
        MSG_PRINTF(LOG_WARN, "Parse content failed!!\n");
        goto end;
    }
    properties = cJSON_GetObjectItem(content_d, "properties");
    if (!properties) {
        MSG_PRINTF(LOG_WARN, "properties content failed!!\n");
        goto end;
    }

    // switchparams
    switchparams = cJSON_GetObjectItem(properties, "switchparams");
    if (switchparams != NULL) {
        state = card_switch_type(switchparams);
    } else {
        MSG_PRINTF(LOG_DBG, "switchparams is NULL!\n");
    }

    // apnparams
    apnparams_list = cJSON_GetObjectItem(properties, "apnparams");
    if (apnparams_list != NULL) {
        state = msg_apnlist_handler(apnparams_list);
    } else {
        MSG_PRINTF(LOG_DBG, "apnparams is NULL!\n");
    }

    // monitorstrategyparams
    monitorstrategyparams = cJSON_GetObjectItem(properties, "monitorstrategyparams");
    if (monitorstrategyparams != NULL) {
        state = msg_analyse_strategy(monitorstrategyparams);
    } else {
        MSG_PRINTF(LOG_DBG, "monitor strategyparams is NULL!!\n");
    }

    // upload
    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_ERR, "content buffer is empty\n");
        goto end;
    }
    content_s = cJSON_PrintUnformatted(properties);
    if (content_s != NULL) {
        MSG_PRINTF(LOG_TRACE, "content_s : %s\n", content_s);
        cJSON_AddItemToObject(content, "properties", cJSON_CreateString(content_s));
        cJSON_free(content_s);
    }

    *out = content;
end:
    if (payload != NULL) {
        cJSON_Delete(payload);
    }

    rt_os_free((void *)in);
    return state;
}

DOWNSTREAM_METHOD_OBJ_INIT(UPDATE, MSG_ID_CARD_MANAGER, ON_UPDATE, update_parser, update_handler);
UPLOAD_EVENT_OBJ_INIT(ON_UPDATE, TOPIC_DEVICEID_OR_EID, upload_on_update_packer);
