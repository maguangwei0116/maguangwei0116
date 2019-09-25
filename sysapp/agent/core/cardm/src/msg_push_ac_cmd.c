
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : msg_push_ac_cmd.c
 * Date        : 2019.09.04
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "msg_process.h"
#include "agent_queue.h"
#include "downstream.h"
#include "rt_timer.h"
#include "rt_type.h"
#include "cJSON.h"
#include "md5.h"

static uint8_t g_iccid[THE_ICCID_LENGTH + 1] = {0};

static cJSON *upload_push_ac_packer(void *arg)
{
    MSG_PRINTF(LOG_WARN, "Upload push ac\n");
exit_entry:
    return (cJSON *)arg;
}

static void push_ac_timer(void)
{
    MSG_PRINTF(LOG_INFO, "g_iccid:%s\n", g_iccid);
    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_ENABLE_EXIST_CARD, g_iccid, rt_os_strlen(g_iccid));
}

static int32_t push_ac_parser(const void *in, char *tranid, void **out)
{
    int32_t ret = RT_ERROR;
    cJSON *agent_msg = NULL;
    cJSON *tran_id = NULL;
    cJSON *payload = NULL;
    cJSON *payload_info = NULL;
    static int8_t md5_out_pro[MD5_STRING_LENGTH + 1];
    int8_t md5_out_now[MD5_STRING_LENGTH + 1];
    uint8_t *buf = NULL;
    int32_t len = 0;
    MSG_PRINTF(LOG_INFO, "In buffer:%s\n", in);
    get_md5_string((int8_t *)in, md5_out_now);
    md5_out_now[MD5_STRING_LENGTH] = '\0';
    if (rt_os_strncmp(md5_out_pro, md5_out_now, MD5_STRING_LENGTH) == 0) {
        MSG_PRINTF(LOG_ERR, "The data are the same!!\n");
        return ret;
    }
    rt_os_memcpy(md5_out_pro, md5_out_now, MD5_STRING_LENGTH);
    MSG_INFO_ARRAY("md5_out_pro", md5_out_pro, MD5_STRING_LENGTH);
    MSG_INFO_ARRAY("md5_out_now", md5_out_now, MD5_STRING_LENGTH);
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

static int32_t download_one_profile(uint8_t *iccid, cJSON *command_content, int32_t *prio)
{
    cJSON *activation_code = NULL;
    cJSON *confirmation_code = NULL;
    cJSON *priority = NULL;
    int32_t state = RT_ERROR;
    uint8_t count = 0;
    const char *cc = NULL;

    //debug_json_data(command_content, command_content);
    activation_code = cJSON_GetObjectItem(command_content, "ac");
    if (!activation_code) {
        MSG_PRINTF(LOG_ERR, "Error before JSON: [%s] (No AC)\n", cJSON_GetErrorPtr());
        return ERROR_NO_REQUIRED_DATA;
    }
    confirmation_code = cJSON_GetObjectItem(command_content, "cc");
    if (!confirmation_code) {
        MSG_PRINTF(LOG_ERR, "Error before JSON: [%s] (No CC)\n", cJSON_GetErrorPtr());
    } else {
        cc = confirmation_code->valuestring;
    }

    priority = cJSON_GetObjectItem(command_content, "priority");
    if (!priority) {
        MSG_PRINTF(LOG_ERR, "Error before JSON: [%s] (No priority)\n", cJSON_GetErrorPtr());
    }
    *prio = priority->valueint;
    while(1) {
        //debug_json_data(activation_code, activation_code);
        MSG_PRINTF(LOG_INFO, "AC: %s\r\n", activation_code->valuestring);
        state = msg_download_profile(activation_code->valuestring, cc, iccid);
        if ((state == -302) || (state == -309) || (state == -310) || (state == -311)) {  // retry three times
            count++;
            rt_os_sleep(10);
            if (count > 2) {        // Retry 2 times. Max total 3 times !!!
                break;
            }
        } else {
            iccid[THE_ICCID_LENGTH] = '\0';
            break;
        }
    }

    if (state != RT_SUCCESS) {  // Download error,can not get iccid
        iccid[0] = '\0';
    } else {  // Store apn name
        cJSON *new_command_content;
        cJSON *apn_list;
        uint8_t *apn_list_c;

        new_command_content = cJSON_CreateObject();
        if (new_command_content == NULL) {
            MSG_PRINTF(LOG_ERR, "The new_command_content is error!!\n");
        } else {
            /* get apn list information */
            apn_list = cJSON_GetObjectItem(command_content, "apnInfos");
            /* add iccid information */
            cJSON_AddItemToObject(new_command_content, "iccid", cJSON_CreateString((const char *)iccid));
            /* add apn list information */
            apn_list_c = cJSON_PrintUnformatted(apn_list);
            /* create a new apn list json object */
            cJSON_AddItemToObject(new_command_content, "apnInfos", cJSON_Parse((const char *)apn_list_c));
            //debug_json_data(new_command_content, new_command_content);
            msg_analyse_apn(new_command_content, iccid);
            cJSON_Delete(new_command_content);
            cJSON_free(apn_list_c);
        }
    }
    return state;
}

static int32_t push_ac_handler(const void *in, const char *event, void **out)
{
    int32_t item = 0;
    int32_t ii = 0;
    int32_t fail_times = 0;
    int32_t state = RT_SUCCESS;
    int32_t code = RT_ERROR;
    int32_t priority = 0;
    int32_t prio = 0;
    uint8_t iccid_t[THE_ICCID_LENGTH + 1] = {0};

    cJSON *up_content = NULL;
    cJSON *payload = NULL;
    cJSON *ac_infos = NULL;
    cJSON *code_info = NULL;
    cJSON *install_result = NULL;
    cJSON *to_enable = NULL;
    cJSON *down_content = NULL;

    install_result = cJSON_CreateArray();
    if (!install_result) {
        MSG_PRINTF(LOG_ERR, "Install result buffer is empty\n");
        goto end;
    }
    MSG_PRINTF(LOG_WARN, "install_result [%p] !!!\r\n", install_result);
    do {
        MSG_PRINTF(LOG_INFO, "payload:%s\n", (uint8_t *)in);
        payload = cJSON_Parse((uint8_t *)in);
        if (!payload) {
            MSG_PRINTF(LOG_ERR, "Parse payload failed!!\n");
            break;
        }
        down_content = cJSON_GetObjectItem(payload, "content");
        if (!down_content) {
            MSG_PRINTF(LOG_ERR, "Parse content failed!!\n");
            break;
        }
        to_enable = cJSON_GetObjectItem(down_content, "toEnable");
        if (!to_enable) {
            MSG_PRINTF(LOG_ERR, "Parse to_enable failed!!\n");
            break;
        }
        ac_infos = cJSON_GetObjectItem(down_content, "acInfos");
        if (!ac_infos) {
            MSG_PRINTF(LOG_ERR, "Parse acInfos failed!!\n");
            break;
        }
        MSG_PRINTF(LOG_INFO, "to_enable:%d\r\n", to_enable->valueint);
        item = cJSON_GetArraySize(ac_infos);

        for(ii = 0; ii < item; ii++) {
            code = download_one_profile(iccid_t, cJSON_GetArrayItem(ac_infos, ii) ,&priority);
            code_info = cJSON_CreateObject();
            if (!code_info) {
                MSG_PRINTF(LOG_ERR, "Code info create failed\n");
                continue;
            }
            cJSON_AddItemToObject(code_info, "code", cJSON_CreateNumber(code));
            cJSON_AddItemToObject(code_info, "iccid", cJSON_CreateString(iccid_t));
            cJSON_AddItemToArray(install_result, code_info);
            MSG_PRINTF(LOG_WARN, "add %d, code:%d, iccid_t=%s\r\n", ii, code, iccid_t);
            if (code == RT_SUCCESS) {
                if (prio == 0 || priority <= prio) {
                    rt_os_memcpy(g_iccid, iccid_t, THE_ICCID_LENGTH);
                    g_iccid[THE_ICCID_LENGTH] = '\0';
                }
                prio = priority;
            } else {
                fail_times++;
                state = 1;
            }
        }
        if (fail_times == item) {
            state = -1;  // All failed
        }
    } while(0);

    MSG_PRINTF(LOG_WARN, "Add install result\n");
    if (install_result != NULL) {
        up_content = cJSON_CreateObject();
        if (!up_content) {
            MSG_PRINTF(LOG_ERR, "content buffer is empty\n");
            goto end;
        }
        cJSON_AddItemToObject(up_content, "results", install_result);
    }
    *out = (void *)up_content;
    card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
    if ((to_enable->valueint == RT_TRUE) && (state != -1)) {
        register_timer(15, 0, &push_ac_timer);
    }
end:
    if (payload != NULL) {
        cJSON_Delete(payload);
    }
    rt_os_free((void *)in);
    return state;
}

DOWNSTREAM_METHOD_OBJ_INIT(PUSH_AC, MSG_ID_CARD_MANAGER, ON_PUSH_AC, push_ac_parser, push_ac_handler);
UPLOAD_EVENT_OBJ_INIT(ON_PUSH_AC, upload_push_ac_packer);
