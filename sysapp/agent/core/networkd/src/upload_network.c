
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : upload_network.c
 * Date        : 2019.09.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "upload_network.h"
#include "agent_queue.h"
#include "downstream.h"
#include "cJSON.h"
#include "md5.h"

static cJSON *upload_on_network_packer(void *arg)
{
    MSG_PRINTF(LOG_DBG, "Upload on network\n");
    
exit_entry:
    return (cJSON *)arg;
}

static int32_t network_parser(const void *in, char *tranid, void **out)
{
    int32_t ret = RT_ERROR;
    cJSON *agent_msg = NULL;
    cJSON *tran_id = NULL;
    cJSON *payload = NULL;
    cJSON *payload_info = NULL;
    cJSON *content = NULL;
    uint8_t *buf = NULL;
    int32_t len = 0;
    static int8_t md5_out_pro[MD5_STRING_LENGTH + 1];
    int8_t md5_out_now[MD5_STRING_LENGTH + 1];

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
        buf = (uint8_t *)rt_os_malloc(len + 1);
        if (!buf) {
            MSG_PRINTF(LOG_ERR, "Malloc buf failed!!\n");
            break;
        }
        rt_os_memcpy(buf, payload->valuestring, len);
        buf[len] = '\0';
        MSG_PRINTF(LOG_INFO, "payload:%s,len:%d\n", buf, len);
        ret = RT_SUCCESS;
    } while(0);
    
    *out = (void *)buf;
    if (agent_msg != NULL) {
        cJSON_Delete(agent_msg);
    }
    return ret;
}

static int32_t network_handler(const void *in, const char *event, void **out)
{
    uint8_t buf[1024];
    const int8_t *cmd = "ping 23.91.101.68 -W 10 -c 5";
    int32_t num;
    int32_t state = RT_SUCCESS;
    cJSON *payload = NULL;
    cJSON *content = NULL;

    num = shell_cmd(cmd, buf, sizeof(buf));
    buf[num] = '\0';
    MSG_PRINTF(LOG_INFO, "num:%d, buf:%s\n", num, buf);

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_ERR, "content buffer is empty\n");
        goto end;
    }
    cJSON_AddItemToObject(content, "network_info", cJSON_CreateString(buf));
    *out = content;
end:
    rt_os_free((void *)in);
    return state;
}

DOWNSTREAM_METHOD_OBJ_INIT(NETWORK, MSG_ID_IDLE, ON_NETWORK, network_parser, network_handler);
UPLOAD_EVENT_OBJ_INIT(ON_NETWORK, TOPIC_DEVICEID_OR_EID, upload_on_network_packer);

