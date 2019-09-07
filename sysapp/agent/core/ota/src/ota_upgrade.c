

#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "downstream.h"
#include "upload.h"
#include "upgrade.h"

#include "cJSON.h"

#if 0
{
   "target":{
       "name": "linux_agent",
       "version": "v2.4.1",
       "chipModel": "9x07",
       "ticket": "3Ubf-Helb-FE5h-zfgB",
       "fileHash": ¡°88363cdd81481d2f902b04b034ac3463aaf5ade2dd54ce226d41519ca5fe520e¡±
   },
   "policy":{
       "forced": 0,
       "executionType": "NOW",
       "profileType": 0,
       "retryAttempts": 3,
       "retryInterval": 90
   }
}
#endif

typedef struct _ota_upgrade_target_t {
    char                    name[64];
    char                    version[16];
    char                    chipModel[16];
    char                    ticket[64];
    char                    fileHash[72];
} ota_upgrade_target_t;

typedef struct _ota_upgrade_policy_t {
    uint8_t                 forced;
    char                    executionType[16];
    uint8_t                 profileType;
    uint16_t                retryAttempts;
    uint16_t                retryInterval;
    char                    mode[16];
    char                    issueType[16];
} ota_upgrade_policy_t;

typedef struct _ota_upgrade_param_t {
    char                    tranId[64];
    ota_upgrade_target_t    target;
    ota_upgrade_policy_t    policy;
} ota_upgrade_param_t;

#define cJSON_GET_INT_DATA(json, item, item_int_out, tmp)\
    do {\
        tmp = cJSON_GetObjectItem(json, #item);\
        if (tmp) {\
            item_int_out = tmp->valueint;\
        }\
    } while(0)

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

#define cJSON_PRINT_JSON_STR_DATA(json, item_str_out)\
    do {\
        item_str_out = cJSON_PrintUnformatted(json);\
    } while(0)

#define cJSON_DEBUG_JSON_STR_DATA(json)\
    do {\
        const char *tmp_str_out = cJSON_PrintUnformatted(json);\
        if (tmp_str_out) {\
            MSG_PRINTF(LOG_INFO, #json": %s\r\n", tmp_str_out);\
            cJSON_free((void *)tmp_str_out);\
        }\
    } while(0)

#define OTA_CHK_PINTER_NULL(p, ret_value)\
    do {\
        if (!p) {\
            MSG_PRINTF(LOG_WARN, #p" error\n");\
            ret = ret_value;\
            goto exit_entry;\
        }\
    } while(0)

static int32_t ota_upgrade_parser(const void *in, char *tran_id, void **out)
{
    int32_t ret;
    cJSON *msg = NULL;
    cJSON *tranId = NULL;
    cJSON *payload = NULL;
    char *payload_str = NULL;
    cJSON *payload_json = NULL;
    cJSON *content = NULL;
    cJSON *target = NULL;
    cJSON *policy = NULL;
    cJSON *tmp = NULL;
    ota_upgrade_param_t *param = NULL;

    msg =  cJSON_Parse((const char *)in);
    OTA_CHK_PINTER_NULL(msg, -1);

    cJSON_GET_JSON_DATA(msg, tranId);
    OTA_CHK_PINTER_NULL(tranId, -2);
    
    rt_os_strcpy(tran_id, tranId->valuestring);

    param = (ota_upgrade_param_t *)rt_os_malloc(sizeof(ota_upgrade_param_t));
    OTA_CHK_PINTER_NULL(param, -3);
    rt_os_memset(param, 0, sizeof(ota_upgrade_param_t));
    strncpy(param->tranId, tranId->valuestring, rt_os_strlen(tranId->valuestring));

    cJSON_GET_JSON_DATA(msg, payload);
    OTA_CHK_PINTER_NULL(param, -4);

    payload_json = cJSON_Parse((const char *)payload->valuestring);
    OTA_CHK_PINTER_NULL(payload_json, -5);
    cJSON_DEBUG_JSON_STR_DATA(payload_json);

    cJSON_GET_JSON_DATA(payload_json, content);
    OTA_CHK_PINTER_NULL(content, -6);

    cJSON_GET_JSON_DATA(content, target);
    OTA_CHK_PINTER_NULL(target, -7);
    cJSON_DEBUG_JSON_STR_DATA(target);
    cJSON_GET_STR_DATA(target, name, param->target.name, sizeof(param->target.name), tmp);
    cJSON_GET_STR_DATA(target, version, param->target.version, sizeof(param->target.version), tmp);
    cJSON_GET_STR_DATA(target, chipModel, param->target.chipModel, sizeof(param->target.chipModel), tmp);
    cJSON_GET_STR_DATA(target, ticket, param->target.ticket, sizeof(param->target.ticket), tmp);
    cJSON_GET_STR_DATA(target, fileHash, param->target.fileHash, sizeof(param->target.fileHash), tmp);

    cJSON_GET_JSON_DATA(content, policy);
    OTA_CHK_PINTER_NULL(policy, -8);
    cJSON_DEBUG_JSON_STR_DATA(policy);
    cJSON_GET_INT_DATA(policy, forced, param->policy.forced, tmp);
    cJSON_GET_STR_DATA(policy, executionType, param->policy.executionType, sizeof(param->policy.executionType), tmp);
    cJSON_GET_INT_DATA(policy, profileType, param->policy.forced, tmp);
    cJSON_GET_INT_DATA(policy, retryAttempts, param->policy.forced, tmp);
    cJSON_GET_INT_DATA(policy, retryInterval, param->policy.forced, tmp);

    *out = param;
    ret = 0;

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

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           (sizeof((a)) / sizeof((a)[0]))
#endif

static rt_bool ota_upgrade_get_target_file_name(const char *fileName, char *targetFileName)
{
    const char *g_target_files[] = 
    {
        "/usr/bin/agent",
        "/usr/bin/monitor",
        "/usr/lib/libcomm.so",        
    };
    int32_t i = 0;
    int32_t cnt = ARRAY_SIZE(g_target_files);
    char *p;

    for (i = 0; i < cnt; i++) {
        p = rt_os_strrchr(g_target_files[i], '/');
        if (p) {
            p++;
            if (rt_os_strstr(fileName, p)) {
                rt_os_strcpy(targetFileName, g_target_files[i]);
                MSG_PRINTF(LOG_WARN, "Find target file name: [%s] => [%s]\r\n", fileName, targetFileName);
                return RT_TRUE;
            }
        }
    }

    MSG_PRINTF(LOG_WARN, "Can't find target file name of [%s]\r\n", fileName);
    return RT_FALSE;
}

static int32_t ota_upgrade_start(const void *in)
{
    int32_t ret;
    rt_task id;
    uint8_t update_mode = 1;
    uint8_t force_update = 0;
    const ota_upgrade_param_t *param = (const ota_upgrade_param_t *)in;
    upgrade_struct_t *upgrade_info = (upgrade_struct_t *)rt_os_malloc(sizeof(upgrade_struct_t));  

    OTA_CHK_PINTER_NULL(param, -1);
    OTA_CHK_PINTER_NULL(upgrade_info, -2);
    rt_os_memset(upgrade_info, 0, sizeof(upgrade_struct_t));

    SET_UPDATEMODE(upgrade_info, update_mode);
    SET_FORCEUPDATE(upgrade_info, force_update);

    snprintf(upgrade_info->tranId, sizeof(upgrade_info->tranId), "%s", param->tranId);
    snprintf(upgrade_info->chipModel, sizeof(upgrade_info->chipModel), "%s", param->target.chipModel);
    snprintf(upgrade_info->fileName, sizeof(upgrade_info->fileName), "%s", param->target.name);
    snprintf(upgrade_info->versionName, sizeof(upgrade_info->versionName), "%s", param->target.version);
    snprintf(upgrade_info->fileHash, sizeof(upgrade_info->fileHash), "%s", param->target.fileHash);
    snprintf(upgrade_info->ticket, sizeof(upgrade_info->ticket), "%s", param->target.ticket);
    upgrade_info->retryAttempts = param->policy.retryAttempts;
    upgrade_info->retryInterval = param->policy.retryInterval;
    if (ota_upgrade_get_target_file_name(upgrade_info->fileName, upgrade_info->targetFileName) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "Unknow target file name: %s\r\n", upgrade_info->fileName);
        ret = -3;
        goto exit_entry;
    }
    
    if (rt_create_task(&id, check_upgrade_process, (void *)upgrade_info) != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "Create upgrade thread flase\n");
        ret = -4;
        goto exit_entry;
    }
    
    ret = 0;
        
exit_entry:

    if (param) {
        rt_os_free((void *)param);
        param = NULL;
    }

    if (ret && upgrade_info) {
        rt_os_free((void *)upgrade_info);
        upgrade_info = NULL;
    }

    return ret;
}

/* 
return RT_FALSE: needn't download
return RT_TRUE : download right now
*/
static rt_bool ota_upgrade_start_check(const ota_upgrade_param_t *param)
{
    rt_bool ret;   
}

static int32_t ota_upgrade_handler(const void *in, void **out)
{
    int32_t ret = -1;
    const ota_upgrade_param_t *param = (const ota_upgrade_param_t *)in;
    
    if (param) {
        MSG_PRINTF(LOG_INFO, "param->tranId                 : %s\r\n", param->tranId);
        MSG_PRINTF(LOG_INFO, "param->target.name            : %s\r\n", param->target.name);
        MSG_PRINTF(LOG_INFO, "param->target.version         : %s\r\n", param->target.version);
        MSG_PRINTF(LOG_INFO, "param->target.chipModel       : %s\r\n", param->target.chipModel);
        MSG_PRINTF(LOG_INFO, "param->target.ticket          : %s\r\n", param->target.ticket);
        MSG_PRINTF(LOG_INFO, "param->target.fileHash        : %s\r\n", param->target.fileHash);
        MSG_PRINTF(LOG_INFO, "param->policy.forced          : %d\r\n", param->policy.forced);
        MSG_PRINTF(LOG_INFO, "param->policy.executionType   : %s\r\n", param->policy.executionType);
        MSG_PRINTF(LOG_INFO, "param->policy.profileType     : %d\r\n", param->policy.profileType);
        MSG_PRINTF(LOG_INFO, "param->policy.retryAttempts   : %d\r\n", param->policy.retryAttempts);
        MSG_PRINTF(LOG_INFO, "param->policy.retryInterval   : %d\r\n", param->policy.retryInterval);

        ret = ota_upgrade_start(param);
    }

exit_entry:

    return ret;
}

static cJSON *ota_upgrade_packer(void *arg)
{
    int32_t ret = 0;
    cJSON *app_version = NULL;
    const char *name = AGENT_LOCAL_NAME;
    const char *version = AGENT_LOCAL_VERSION;
    const char *chipModel = AGENT_LOCAL_PLATFORM_TYPE;

    app_version = cJSON_CreateObject();
    if (!app_version) {
        MSG_PRINTF(LOG_WARN, "The app_version is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(app_version, name);
    CJSON_ADD_NEW_STR_OBJ(app_version, version);
    CJSON_ADD_NEW_STR_OBJ(app_version, chipModel);
    
    ret = 0;
    
exit_entry:

    return !ret ? app_version : NULL;   
}

DOWNSTREAM_METHOD_OBJ_INIT(UPGRADE, MSG_ID_OTA_UPGRADE, ON_UPGRADE, ota_upgrade_parser, ota_upgrade_handler);

UPLOAD_EVENT_OBJ_INIT(ON_UPGRADE, ota_upgrade_packer);

