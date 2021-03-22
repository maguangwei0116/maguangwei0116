
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

static cJSON *upload_on_detect_packer(void *arg)
{
    MSG_PRINTF(LOG_INFO, "Upload on network\n");
    
exit_entry:
    return (cJSON *)arg;
}

static int32_t detect_parser(const void *in, char *tranid, void **out)
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
        MSG_PRINTF(LOG_TRACE, "payload:%s,len:%d\n", buf, len);
        ret = RT_SUCCESS;
    } while(0);
    
    *out = (void *)buf;
    if (agent_msg != NULL) {
        cJSON_Delete(agent_msg);
    }
    return ret;
}

static int32_t exec_ping_cmd(const char* domain, int32_t times, uint8_t* out, int32_t out_size)
{
    int32_t out_len = 0;
    int8_t cmd_buf[256];
    int8_t temp[4];
    rt_os_memset(cmd_buf, 0, sizeof(cmd_buf));
    rt_os_memset(temp, 0, sizeof(temp));
    rt_os_strcat(cmd_buf, "ping ");
    rt_os_strcat(cmd_buf, domain);
    rt_os_strcat(cmd_buf, " -c ");
    snprintf(temp, sizeof(temp), "%d", times),
    rt_os_strcat(cmd_buf, temp);
    out_len = shell_cmd(cmd_buf, out, out_size);
    out[out_len++] = '\0';
    return out_len;
}

static int32_t detect_handler(const void *in, const char *event, void **out)
{
    int32_t num;
    int32_t all_domains_num = 0;
    int32_t ii = 0;
    int32_t state = RT_SUCCESS;
    uint8_t buf[2048];
    int32_t out_len = 0;
    double  packet_loss = 0;
    double  min_latency = 0;
    double  avg_latency = 0;
    double  max_latency = 0;
    cJSON *payload = NULL;
    cJSON *content = NULL;
    cJSON *ping = NULL;
    cJSON *times = NULL;
    cJSON *domains = NULL;
    cJSON *domain = NULL;
    cJSON *content_r = NULL;
    cJSON *ping_info = NULL;
    cJSON *pong = NULL;
    char  *ptr = NULL;
    const char* PACK_LOSS_HEAD = "received, ";
    const char* RTT_HEAD = "min/avg/max";

    content_r = cJSON_CreateObject();
    if (!content_r) {
        MSG_PRINTF(LOG_ERR, "content_r buffer is empty\n");
        goto end;
    }
    pong = cJSON_CreateArray();
    if (!pong) {
        MSG_PRINTF(LOG_ERR, "pong buffer is empty\n");
        goto end;
    }
    do {
        payload = cJSON_Parse((uint8_t *)in);
        if (!payload) {
            MSG_PRINTF(LOG_ERR, "Parse payload failed!!\n");
            break;
        }
        content = cJSON_GetObjectItem(payload, "content");
        if (!content) {
            MSG_PRINTF(LOG_ERR, "Parse content failed!!\n");
            break;
        }
        ping = cJSON_GetObjectItem(content, "ping");
        if (!ping) {
            MSG_PRINTF(LOG_ERR, "Parse ping failed!!\n");
            break;
        }
        times = cJSON_GetObjectItem(ping, "times");
        if (!times) {
            MSG_PRINTF(LOG_ERR, "Parse times failed!!\n");
            break;
        }
        num = times->valueint;
        domains = cJSON_GetObjectItem(ping, "domains");
        if (!domains) {
            MSG_PRINTF(LOG_ERR, "Parse domains failed!!\n");
            break;
        }
        all_domains_num = cJSON_GetArraySize(domains);
        for (ii = 0; ii < all_domains_num; ii++) {
            domain = cJSON_GetArrayItem(domains, ii);
            if (!domain) {
                MSG_PRINTF(LOG_ERR, "Parse domain failed!!\n");
                continue;
            }
            ping_info = cJSON_CreateObject();
            if (!ping_info) {
                MSG_PRINTF(LOG_ERR, "Ping info create error\n");
                continue;
            }
            rt_os_memset(buf, 0, sizeof(buf));
            out_len = exec_ping_cmd(domain->valuestring, num, buf, sizeof(buf));
            ptr = rt_os_strstr(buf, PACK_LOSS_HEAD);
            if (!ptr) {
                packet_loss = 100;
            } else {
                packet_loss = strtod(ptr + rt_os_strlen(PACK_LOSS_HEAD), &ptr);
            }
            ptr = rt_os_strstr(buf, RTT_HEAD);
            if (ptr != NULL) {
                ptr = rt_os_strstr(ptr, "=");
                if (ptr != NULL) {
                    min_latency = strtod(ptr + 1, &ptr);
                    avg_latency = strtod(ptr + 1, &ptr);
                    max_latency = strtod(ptr + 1, &ptr);
                }
            }
            MSG_PRINTF(LOG_TRACE, "packet_loss:%f, min:%f, avg:%f, max:%f \n", packet_loss, min_latency, avg_latency, max_latency);
            cJSON_AddItemToObject(ping_info, "domain", cJSON_CreateString(domain->valuestring));
            cJSON_AddItemToObject(ping_info, "result", cJSON_CreateString(buf));            
            cJSON_AddItemToObject(ping_info, "packetLoss", cJSON_CreateNumber(packet_loss));
            cJSON_AddItemToObject(ping_info, "maxLatency", cJSON_CreateNumber(max_latency));
            cJSON_AddItemToObject(ping_info, "minLatency", cJSON_CreateNumber(min_latency));
            cJSON_AddItemToObject(ping_info, "avgLatency", cJSON_CreateNumber(avg_latency));
            cJSON_AddItemToArray(pong, ping_info);
        }
        cJSON_AddItemToObject(content_r, "pong", pong);
    } while(0);
    *out = content_r;
end:
    rt_os_free((void *)in);
    return state;
}

DOWNSTREAM_METHOD_OBJ_INIT(DETECT, MSG_ID_IDLE, ON_DETECT, detect_parser, detect_handler);
UPLOAD_EVENT_OBJ_INIT(ON_DETECT, TOPIC_DEVICEID_OR_EID, upload_on_detect_packer);

