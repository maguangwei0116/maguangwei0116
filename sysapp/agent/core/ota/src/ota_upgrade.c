

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

extern const card_info_t *g_ota_card_info;

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

static rt_bool ota_policy_compare_version(const char *old, const char *new)
{
    int32_t i;
    int32_t old_version[4] = {0};
    int32_t new_version[4] = {0};

    MSG_PRINTF(LOG_INFO, "old version: %s\r\n", old); 
    MSG_PRINTF(LOG_INFO, "new version: %s\r\n", new); 
    sscanf(old, "%d.%d.%d.%d", &old_version[0], &old_version[1], &old_version[2], &old_version[3]);
    sscanf(new, "%d.%d.%d.%d", &new_version[0], &new_version[1], &new_version[2], &new_version[3]);
    MSG_PRINTF(LOG_INFO, "--old version: %d.%d.%d.%d\r\n", old_version[0], old_version[1], old_version[2], old_version[3]); 
    MSG_PRINTF(LOG_INFO, "--new version: %d.%d.%d.%d\r\n", new_version[0], new_version[1], new_version[2], new_version[3]); 

    for (i = 0; i < 4; i++) {
        //MSG_PRINTF(LOG_INFO, "%d -- %d\r\n", new_version[i], old_version[i]);
        if (new_version[i] < old_version[i]) {
            return RT_FALSE;
        } else if (new_version[i] > old_version[i]) {
            return RT_TRUE;
        } else {  // ==
            continue;
        }
    }

    MSG_PRINTF(LOG_WARN, "unmsthed version [%s] = [%s]\r\n", old, new);
    return RT_FALSE;
}

static int32_t ota_policy_check(const ota_upgrade_param_t *param, upgrade_struct_t *upgrade_info)
{
    int32_t ret;

    if (!rt_os_strcmp(param->policy.executionType, "NOW")) {
        upgrade_info->excute_app_now = RT_TRUE;
    }

    if (param->policy.profileType == 1 && g_ota_card_info->type != PROFILE_TYPE_OPERATIONAL) {
        MSG_PRINTF(LOG_WARN, "unmathed profile type !\r\n");
        ret = UPGRADE_PROFILE_TYPE_ERROR;
        goto exit_entry;         
    }

    if (rt_os_strcmp(param->target.chipModel, AGENT_LOCAL_PLATFORM_TYPE)) {
        MSG_PRINTF(LOG_WARN, "unmathed platform type !\r\n");
        ret = UPGRADE_FILE_NAME_ERROR;
        goto exit_entry;         
    }

    if (param->policy.forced == UPGRADE_MODE_FORCED) {
        MSG_PRINTF(LOG_WARN, "forced to upgrade !\r\n");
        ret = 0;
        goto exit_entry;   
    } else {
        if (param->policy.forced == UPGRADE_MODE_CHK_FILE_NAME && rt_os_strcmp(param->target.name, AGENT_LOCAL_NAME)) {
            MSG_PRINTF(LOG_WARN, "unmathed file name!\r\n");
            ret = UPGRADE_FILE_NAME_ERROR;
            goto exit_entry;  
        } else if (param->policy.forced == UPGRADE_MODE_CHK_VERSION && \
                        !ota_policy_compare_version(AGENT_LOCAL_VERSION, param->target.version)) {
            MSG_PRINTF(LOG_WARN, "forced to upgrade !\r\n");
            ret = UPGRADE_CHECK_VERSION_ERROR;
            goto exit_entry;   
        } else if (param->policy.forced == UPGRADE_MODE_NO_FORCED) {
             if (rt_os_strcmp(param->target.name, AGENT_LOCAL_NAME)) {
                MSG_PRINTF(LOG_WARN, "unmathed file name!\r\n");
                ret = UPGRADE_FILE_NAME_ERROR;
                goto exit_entry; 
             }

             if (!ota_policy_compare_version(AGENT_LOCAL_VERSION, param->target.version)) {
                MSG_PRINTF(LOG_WARN, "unmathed version name !\r\n");
                ret = UPGRADE_CHECK_VERSION_ERROR;
                goto exit_entry; 
             }
        }
    }

    ret = 0;

exit_entry:

    return ret;
}

static int32_t ota_upgrade_start(const void *in)
{
    int32_t ret;
    rt_task id;
    uint8_t update_mode = 1;
    uint8_t force_update = 0;
    const ota_upgrade_param_t *param = (const ota_upgrade_param_t *)in;
    upgrade_struct_t *upgrade_info = NULL;  

    OTA_CHK_PINTER_NULL(param, UPGRADE_NULL_POINTER_ERROR);
    upgrade_process_create(&upgrade_info);
    OTA_CHK_PINTER_NULL(upgrade_info, UPGRADE_NULL_POINTER_ERROR);

    /* set upgrade information */
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
        ret = UPGRADE_FILE_NAME_ERROR;
        goto exit_entry;
    }

    ret = ota_policy_check(param, upgrade_info);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "ota policy check error\n");
        goto exit_entry;
    }
    
    ret = upgrade_process_start(upgrade_info);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "Create upgrade start error\n");
        ret = UPGRADE_START_UPGRADE_ERROR;
        goto exit_entry;
    }
    
    ret = 0;
        
exit_entry:

    if (param) {
        rt_os_free((void *)param);
        param = NULL;
    }

    upgrade_process_wating(upgrade_info, MAX_DOWNLOAD_TIMEOUTS);
    if (!ret) {
        GET_DOWNLOAD_RET(upgrade_info, ret);
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

    MSG_PRINTF(LOG_WARN, "rer=%d\n", ret);
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

