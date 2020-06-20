
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : msg_delete_cmd.c
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
#include "md5.h"
#include "lpa_error_codes.h"

static cJSON *upload_on_delete_packer(void *arg)
{
    MSG_PRINTF(LOG_INFO, "Upload on delete\n");
exit_entry:
    return (cJSON *)arg;
}

static int32_t delete_parser(const void *in, char *tranid, void **out)
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
            MSG_PRINTF(LOG_ERR, "Message parse failed!!\n");
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

static int32_t delete_handler(const void *in, const char *event, void **out)
{
    int32_t state = RT_SUCCESS;
    int32_t all_iccid_num = 0;
    int32_t ii = 0;
    int32_t code = RT_ERROR;
    int32_t fail_times = 0;
    rt_bool opr_iccid_using = RT_FALSE;
    judge_term_e bootstrap_flag = UPDATE_NOT_JUDGE_BOOTSTRAP;
    cJSON *payload = NULL;
    cJSON *content = NULL;
    cJSON *content_d = NULL;
    cJSON *iccids = NULL;
    cJSON *iccid = NULL;
    cJSON *code_info = NULL;
    cJSON *delete_result = NULL;

    delete_result = cJSON_CreateArray();
    if (!delete_result) {
        MSG_PRINTF(LOG_ERR, "Install result buffer is empty\n");
        goto end;
    }

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_ERR, "content buffer is empty\n");
        goto end;
    }
    do {
        MSG_PRINTF(LOG_TRACE, "payload:%s\n", (uint8_t *)in);
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
        iccids = cJSON_GetObjectItem(content_d, "iccids");
        if (!iccids) {
            MSG_PRINTF(LOG_ERR, "Parse iccid failed!!\n");
            break;
        }
        all_iccid_num = cJSON_GetArraySize(iccids);

        /* update current profile list */
        if (card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP) == RT_SUCCESS) {
            rt_os_sleep(1);
        }
        
        for (ii = 0; ii < all_iccid_num; ii++) {
            iccid = cJSON_GetArrayItem(iccids, ii);
            if (!iccid) {
                MSG_PRINTF(LOG_ERR, "Parse iccid failed!!\n");
                continue;
            }
            
            code_info = cJSON_CreateObject();
            if (!code_info) {
                MSG_PRINTF(LOG_ERR, "Code info create error\n");
                continue;
            }

            opr_iccid_using = RT_FALSE;
            code = msg_delete_profile(iccid->valuestring, &opr_iccid_using);
            if (opr_iccid_using && code == RT_SUCCESS) {
                MSG_PRINTF(LOG_TRACE, "delete using operational profile, start bootstrap ...\n");
                bootstrap_flag = UPDATE_JUDGE_BOOTSTRAP;
            }
            if (RT_ERR_APDU_SEND_FAIL == code) {
                /*
                Delete again when lpa-delete-profile return -204, but profile was deleteed actually.
                Check the second delete operation, when it return code 1, it means profile isn't exist !
                */
                MSG_PRINTF(LOG_WARN, "delete profile with unkown result, delete again ...\n");
                rt_os_sleep(3);
                code = msg_delete_profile(iccid->valuestring, &opr_iccid_using);
                if (code == 1) {
                    code = RT_SUCCESS;
                    if (opr_iccid_using) {
                        MSG_PRINTF(LOG_TRACE, "delete using operational profile, start bootstrap ...\n");
                        bootstrap_flag = UPDATE_JUDGE_BOOTSTRAP;
                    }
                }
            }
            cJSON_AddItemToObject(code_info, "code", cJSON_CreateNumber((double)code));
            cJSON_AddItemToObject(code_info, "iccid", cJSON_CreateString(iccid->valuestring));
            cJSON_AddItemToArray(delete_result, code_info);
            if (code != RT_SUCCESS) {
                state = 1;
                fail_times ++;
            }
        }
        if (fail_times == all_iccid_num) {
            state = -1;  // All failed
        }
        cJSON_AddItemToObject(content, "results", delete_result);
    } while(0);

#if 0
    rt_os_sleep(1);
    card_update_profile_info(bootstrap_flag);    
    if (bootstrap_flag == UPDATE_JUDGE_BOOTSTRAP) {
        rt_os_sleep(5);  // wait some time for provisioning network disconnected, or it may report ON_DELETE twice !!!
    }
#else
    /* 
    when delete all oprtional profiles ok, it will changed to provisoning profile.
    So it will accur network disconnected from oprtional profiles to provisoning profile.
    Need to enable provisoning profile frist.
    It will call network_set_apn_handler, and start bootstrap.
    It will avoid repporting ON_DELETE twice.
    */ 
    if (bootstrap_flag == UPDATE_JUDGE_BOOTSTRAP) {
        card_force_enable_provisoning_profile();
    }
    rt_os_sleep(3);
    card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);

#endif
    *out = content;
end:
    rt_os_free((void *)in);
    return state;
}

DOWNSTREAM_METHOD_OBJ_INIT(DELETE, MSG_ID_CARD_MANAGER, ON_DELETE, delete_parser, delete_handler);
UPLOAD_EVENT_OBJ_INIT(ON_DELETE, TOPIC_DEVICEID_OR_EID, upload_on_delete_packer);

