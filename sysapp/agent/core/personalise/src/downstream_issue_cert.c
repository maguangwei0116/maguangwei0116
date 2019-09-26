
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : downstream_issue_cert.c
 * Date        : 2019.09.25
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "http_client.h"
#include "config.h"
#include "cJSON.h"
#include "md5.h"
#include "upgrade.h"
#include "file.h"

typedef struct _issue_cert_struct_t {
    char                    tranId[64];
    char                    ticket[64];
    char                    fileHash[72];
} issue_cert_struct_t;

#define RT_CERTIFICATE      "/data/redtea/rt_cert"

#define cJSON_GET_STR_DATA(json, item, item_str_out, len, tmp)\
    do {\
        tmp = cJSON_GetObjectItem(json, #item);\
        if (tmp) {\
            snprintf((item_str_out), (len), "%s", tmp->valuestring);\
        }\
    } while(0)

#define cJSON_GET_JSON_DATA(json, item)\
    do {\
        item = cJSON_GetObjectItem(json, #item);\
    } while(0)

#define OTA_CHK_PINTER_NULL(p, ret_value)\
    do {\
        if (!p) {\
            MSG_PRINTF(LOG_WARN, #p" error\n");\
            ret = ret_value;\
            goto exit_entry;\
        }\
    } while(0)

extern rt_bool upload_check_eid_empty(void);

static int32_t downstream_issue_cert_parser(const void *in, char *tran_id, void **out)
{
    int32_t ret = RT_ERROR;
    cJSON *msg = NULL;
    cJSON *tranId = NULL;
    cJSON *payload = NULL;
    char *payload_str = NULL;
    cJSON *payload_json = NULL;
    cJSON *content = NULL;
    cJSON *target = NULL;
    cJSON *tmp = NULL;
    issue_cert_struct_t *param = NULL;

    if (!upload_check_eid_empty()) {
        MSG_PRINTF(LOG_WARN, "eid exist, ignore issue cert download !\r\n");
        goto exit_entry;
    }

    msg =  cJSON_Parse((const char *)in);

    cJSON_GET_JSON_DATA(msg, tranId);
    
    rt_os_strcpy(tran_id, tranId->valuestring);

    param = (issue_cert_struct_t *)rt_os_malloc(sizeof(issue_cert_struct_t)); //todo 非空判断
    if (!param){
        MSG_PRINTF(LOG_WARN, "The param is null!\n");
        goto exit_entry;
    }
    rt_os_memset(param, 0, sizeof(issue_cert_struct_t));
    strncpy(param->tranId, tranId->valuestring, rt_os_strlen(tranId->valuestring));

    cJSON_GET_JSON_DATA(msg, payload);

    payload_json = cJSON_Parse((const char *)payload->valuestring);

    cJSON_GET_JSON_DATA(payload_json, content);

    cJSON_GET_STR_DATA(content, fileHash, param->fileHash, sizeof(param->fileHash), tmp);
    cJSON_GET_STR_DATA(content, ticket, param->ticket, sizeof(param->ticket), tmp);

    *out = param;
    ret = RT_SUCCESS;

exit_entry:

    if (msg) {
        cJSON_Delete(msg);
        msg = NULL;
    }

    if (payload_json) {
        cJSON_Delete(payload_json);
        payload_json = NULL;
    }

    if (ret && param) {
        rt_os_free(param);
        param = NULL;
    }

    return ret;
}

#if 0
static rt_bool on_issue_cert_install(const void *arg)
{
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;
    rt_bool ret = RT_FALSE;
    
    MSG_PRINTF(LOG_INFO, "tmpFileName=%s, targetFileName=%s\r\n", upgrade->tmpFileName, upgrade->targetFileName);
    if (rt_os_rename(upgrade->tmpFileName, upgrade->targetFileName) != 0) {
        MSG_PRINTF(LOG_WARN, "re-name error\n");
        goto exit_entry;
    }

    if (rt_os_chmod(upgrade->targetFileName, RT_S_IRWXU | RT_S_IRWXG | RT_S_IRWXO) != 0) {
        MSG_PRINTF(LOG_WARN, "change mode error\n");
        goto exit_entry;
    }

    //SET_UPGRADE_STATUS(upgrade, 1);

    rt_os_sync();
    rt_os_sync();

    ret = RT_TRUE;
    
exit_entry:
    
    return ret;
}

static rt_bool on_issue_cert_file_check(const void *arg)
{
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;

    return RT_TRUE;
}
#endif

static rt_bool on_issue_cert_cleanup(const void *arg)
{
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;

    rt_os_unlink(upgrade->tmpFileName);

    return RT_TRUE;
}

static rt_bool on_issue_cert_upload_event(const void *arg)
{
    uint8_t *buf = NULL;
    rt_fshandle_t fp;
    int32_t length;
    int32_t ret = RT_ERROR;
    int32_t on_issue_cert_status = 0;
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;

    if (upgrade->downloadResult == RT_SUCCESS) {
        fp = rt_fopen(RT_CERTIFICATE, RT_FS_READ);
        if (!fp) {
            MSG_PRINTF(LOG_ERR, "open cert file failed!\n");
            goto exit_entry;
        }
        length = get_file_size(RT_CERTIFICATE);
        buf = (uint8_t *) rt_os_malloc(length);
        if (!buf) {
            MSG_PRINTF(LOG_ERR, "malloc failed!\n");
            goto exit_entry;
        }
        rt_fread(buf, 1, length, fp);

        ret = card_manager_event((const uint8_t *)buf, length, MSG_CARD_SETTING_CERTIFICATE);
        if (ret) {
            on_issue_cert_status = ret;
        }

        rt_os_free(buf);
        if (fp) {
            rt_fclose(fp);
        }
        rt_os_unlink(RT_CERTIFICATE);
    }else {
        on_issue_cert_status = upgrade->downloadResult;
    }

exit_entry:

    upload_event_report(upgrade->event, (const char *)upgrade->tranId, on_issue_cert_status, (void *)upgrade); 
    
    /* release upgrade struct memory */
    if (upgrade) {
        rt_os_free((void *)upgrade);
        upgrade = NULL;
    }

    return RT_TRUE;
}

static int32_t upgrade_download_package(const void *in, const char *upload_event)
{
    int32_t ret;
    rt_task id;
    uint8_t update_mode = 1;
    uint8_t force_update = 0;
    const issue_cert_struct_t *param = (const issue_cert_struct_t *)in;
    upgrade_struct_t *upgrade_info = NULL;  

    OTA_CHK_PINTER_NULL(param, UPGRADE_NULL_POINTER_ERROR);
    upgrade_process_create(&upgrade_info);
    OTA_CHK_PINTER_NULL(upgrade_info, UPGRADE_NULL_POINTER_ERROR);

    /* set upgrade information */
    rt_os_memset(upgrade_info, 0, sizeof(upgrade_struct_t));
    SET_UPDATEMODE(upgrade_info, update_mode);
    SET_FORCEUPDATE(upgrade_info, force_update);
    snprintf(upgrade_info->tranId, sizeof(upgrade_info->tranId), "%s", param->tranId);
    snprintf(upgrade_info->fileHash, sizeof(upgrade_info->fileHash), "%s", param->fileHash);
    snprintf(upgrade_info->ticket, sizeof(upgrade_info->ticket), "%s", param->ticket);
    snprintf(upgrade_info->tmpFileName, sizeof(upgrade_info->tmpFileName), "%s", RT_CERTIFICATE);
    snprintf(upgrade_info->targetFileName, sizeof(upgrade_info->targetFileName), "%s", RT_CERTIFICATE);
    snprintf(upgrade_info->event, sizeof(upgrade_info->event), "%s", upload_event);
    upgrade_info->retryAttempts = 2;
    upgrade_info->retryInterval = 10;

     /* set callback functions */
    upgrade_info->check      = NULL;
    upgrade_info->install    = NULL;
    upgrade_info->cleanup    = NULL;
    upgrade_info->on_event   = on_issue_cert_upload_event;

    on_issue_cert_cleanup(upgrade_info);
    ret = upgrade_process_start(upgrade_info);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "Create upgrade start error\n");
        ret = UPGRADE_START_UPGRADE_ERROR;
        goto exit_entry;
    }
    
    ret = 0;
        
exit_entry:
    // todo  handler中释放
    if (param) {
        rt_os_free((void *)param);
        param = NULL;
    }

    return ret;
}

static int32_t downstream_issue_cert_handler(const void *in, const char *event, void **out)
{
    int32_t ret = 0;

    const issue_cert_struct_t *param = (const issue_cert_struct_t *)in;
    ret = upgrade_download_package(param, event);
    return ret;
}

static cJSON *upload_on_issue_cert_packer(void *arg)
{
    return NULL;   
}

DOWNSTREAM_METHOD_OBJ_INIT(ISSUE_CERT, MSG_ID_PERSONLLISE, ON_ISSUE_CERT, downstream_issue_cert_parser, downstream_issue_cert_handler);

UPLOAD_EVENT_OBJ_INIT(ON_ISSUE_CERT, TOPIC_DEVICEID, upload_on_issue_cert_packer);

