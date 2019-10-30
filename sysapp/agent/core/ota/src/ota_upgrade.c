
#ifdef CFG_AGENT_OTA_ON
#warning AGENT_OTA_ON on ...

#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "downstream.h"
#include "upload.h"
#include "upgrade.h"
#include "hash.h"
#include "http_client.h"
#include "file.h"
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

typedef struct OTA_UPGRADE_TARGET {
    char                    name[64];
    char                    version[16];
    char                    chipModel[16];
    char                    ticket[64];
    uint8_t                 type;
    uint32_t                size;
    char                    fileHash[72];    
} ota_upgrade_target_t;

typedef struct OTA_UPGRADE_POLICY {
    uint8_t                 forced;
    char                    executionType[16];
    uint8_t                 profileType;
    uint16_t                retryAttempts;
    uint16_t                retryInterval;
    char                    mode[16];
    char                    issueType[16];
} ota_upgrade_policy_t;

typedef struct OTA_UPGRADE_PARAM {
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

#define MAX_RESTART_WAIT_TIMEOUT    10
#define PRIVATE_HASH_STR_LEN        64
#define HASH_CHECK_BLOCK            1024    /* block size for HASH check */
#define OTA_UPGRADE_TMP_TASK        "/data/redtea/ota.task.tmp"

static const card_info_t *g_ota_card_info = NULL;
extern const target_versions_t *g_upload_ver_info;

typedef struct OTA_TASK_INFO {
    uint32_t            param_len;
    char                tmp_file[MAX_FILE_NAME_LEN + 1];     // the full path in local file system 
    char                event[64 + 1];
    ota_upgrade_param_t param;
} ota_task_info_t;

static int32_t ota_upgrade_start(const void *in, const char *upload_event, const char *tmp_file);

static int32_t ota_upgrade_task_record(const void *param, uint32_t param_len, const char *tmp_file, const char *event)
{  
    int32_t ret = -1;
    rt_fshandle_t fp = NULL;
    ota_task_info_t task = {0};
    
    fp = rt_fopen(OTA_UPGRADE_TMP_TASK, "w+");
    OTA_CHK_PINTER_NULL(fp, -1);

    task.param_len = param_len;
    rt_os_memcpy(&task.param, param, param_len);
    snprintf(task.tmp_file, sizeof(task.tmp_file), "%s", tmp_file);
    snprintf(task.event, sizeof(task.event), "%s", event);
    rt_fwrite(&task, 1, sizeof(task), fp);
    
    rt_fclose(fp);

    ret = 0;

exit_entry:

    return ret;
}

int32_t ota_upgrade_task_check_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    static int32_t g_task_check = 0;    
    int32_t ret = -1;

    (void)buf;
    (void)len;

    if (MSG_NETWORK_CONNECTED == mode && !g_task_check && rt_file_exist(OTA_UPGRADE_TMP_TASK)) {
        rt_fshandle_t fp = NULL;
        ota_task_info_t task = {0};
        ota_upgrade_param_t *param = NULL;

        g_task_check = 1;
        fp = rt_fopen(OTA_UPGRADE_TMP_TASK, "r"); 
        OTA_CHK_PINTER_NULL(fp, -1);
        rt_fread(&task, 1, sizeof(task), fp);
        rt_fclose(fp);
        OTA_CHK_PINTER_NULL(task.param_len, -2);

        MSG_PRINTF(LOG_INFO, "OTA remain task list          : \r\n");
        MSG_PRINTF(LOG_INFO, "param->tranId                 : %s\r\n", task.param.tranId);
        MSG_PRINTF(LOG_INFO, "param->target.name            : %s\r\n", task.param.target.name);
        MSG_PRINTF(LOG_INFO, "param->target.version         : %s\r\n", task.param.target.version);
        MSG_PRINTF(LOG_INFO, "param->target.chipModel       : %s\r\n", task.param.target.chipModel);
        MSG_PRINTF(LOG_INFO, "param->target.ticket          : %s\r\n", task.param.target.ticket);
        MSG_PRINTF(LOG_INFO, "param->target.fileHash        : %s\r\n", task.param.target.fileHash);
        MSG_PRINTF(LOG_INFO, "param->policy.forced          : %d\r\n", task.param.policy.forced);
        MSG_PRINTF(LOG_INFO, "param->policy.executionType   : %s\r\n", task.param.policy.executionType);
        MSG_PRINTF(LOG_INFO, "param->policy.profileType     : %d\r\n", task.param.policy.profileType);
        MSG_PRINTF(LOG_INFO, "param->policy.retryAttempts   : %d\r\n", task.param.policy.retryAttempts);
        MSG_PRINTF(LOG_INFO, "param->policy.retryInterval   : %d\r\n", task.param.policy.retryInterval);        
        MSG_PRINTF(LOG_WARN, "ota task, len=%d, tmp-file: %s, event: %s\r\n", task.param_len, task.tmp_file, task.event); 

        param = (ota_upgrade_param_t *)rt_os_malloc(sizeof(ota_upgrade_param_t));
        OTA_CHK_PINTER_NULL(param, -3);
        rt_os_memcpy(param, &task.param, sizeof(ota_upgrade_param_t));
        ota_upgrade_start(param, task.event, task.tmp_file);

        ret = 0;
    }

exit_entry:

    return ret;
}

static int32_t ota_upgrade_task_cleanup(void)
{
    rt_os_unlink(OTA_UPGRADE_TMP_TASK);
    return 0;  
}

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
    rt_os_strncpy(param->tranId, tranId->valuestring, rt_os_strlen(tranId->valuestring));

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
    cJSON_GET_INT_DATA(target, type, param->target.type, tmp);
    cJSON_GET_INT_DATA(target, size, param->target.size, tmp);
    cJSON_GET_STR_DATA(target, fileHash, param->target.fileHash, sizeof(param->target.fileHash), tmp);

    cJSON_GET_JSON_DATA(content, policy);
    OTA_CHK_PINTER_NULL(policy, -8);
    cJSON_DEBUG_JSON_STR_DATA(policy);
    cJSON_GET_INT_DATA(policy, forced, param->policy.forced, tmp);
    cJSON_GET_STR_DATA(policy, executionType, param->policy.executionType, sizeof(param->policy.executionType), tmp);
    cJSON_GET_INT_DATA(policy, profileType, param->policy.profileType, tmp);
    cJSON_GET_INT_DATA(policy, retryAttempts, param->policy.retryAttempts, tmp);
    cJSON_GET_INT_DATA(policy, retryInterval, param->policy.retryInterval, tmp);

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

static rt_bool ota_upgrade_get_target_file_name(const ota_upgrade_param_t *param, char *targetFileName, int32_t len)
{
    const char *g_target_files[] = 
    {
        "/usr/bin/rt_agent",
        "/usr/bin/rt_monitor",
        "/usr/lib/libcomm.so",        
    };
    int32_t i = 0;
    int32_t cnt = ARRAY_SIZE(g_target_files);
    char *p;
    char *p0;
    const char *fileName = param->target.name;

    for (i = 0; i < cnt; i++) {
        p = rt_os_strrchr(g_target_files[i], '/');
        if (p) {
            p++;

            /* delete "redtea" prefix */
            p0 = rt_os_strstr(p, "rt_");    
            if (p0) {
                p = p0+3;
            }
            
            if (rt_os_strstr(fileName, p)) {
                snprintf(targetFileName, len, g_target_files[i]);
                MSG_PRINTF(LOG_WARN, "Find target file name: [%s] => [%s]\r\n", fileName, targetFileName);
                return RT_TRUE;
            }
        }
    }

    MSG_PRINTF(LOG_WARN, "Can't find target file name of [%s]\r\n", fileName);
    return RT_FALSE;
}

static rt_bool ota_upgrade_get_tmp_file_name(const ota_upgrade_param_t *param, char *tmpFileName, int32_t len)
{
#define TMP_DOWNLOAD_PATH "/data/redtea/"
    static int32_t tmp_file_index = 0;

    /* Build a complete path to download files */
    snprintf(tmpFileName, len, "%s%s_v%s_%s.%d.tmp", \
                TMP_DOWNLOAD_PATH, param->target.name, param->target.version, param->target.chipModel, ++tmp_file_index);  

    return RT_TRUE;
#undef TMP_DOWNLOAD_PATH
}

static rt_bool ota_policy_compare_version(const char *old_in, const char *new_in)
{
    int32_t i;
    int32_t old_version[4] = {0};
    int32_t new_version[4] = {0};

    //MSG_PRINTF(LOG_INFO, "old version: %s\r\n", old_in); 
    //MSG_PRINTF(LOG_INFO, "new version: %s\r\n", new_in); 
    sscanf(old_in, "%d.%d.%d.%d", &old_version[0], &old_version[1], &old_version[2], &old_version[3]);
    sscanf(new_in, "%d.%d.%d.%d", &new_version[0], &new_version[1], &new_version[2], &new_version[3]);
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

    MSG_PRINTF(LOG_WARN, "unmathed version [%s] => [%s]\r\n", old_in, new_in);
    return RT_FALSE;
}

static int32_t ota_policy_check(const ota_upgrade_param_t *param, upgrade_struct_t *upgrade)
{
    int32_t ret;
    uint8_t policy_forced = 0;

    if (!rt_os_strcmp(param->policy.executionType, "NOW")) {
        upgrade->execute_app_now = RT_TRUE;
    } else if (!rt_os_strcmp(param->policy.executionType, "REBOOT")) {
        upgrade->execute_app_now = RT_FALSE;
    } else {
        MSG_PRINTF(LOG_WARN, "unknow execution type %s\r\n", param->policy.executionType);
        ret = UPGRADE_EXECUTION_TYPE_ERROR;
        goto exit_entry; 
    }

    if (param->policy.profileType == UPGRADE_PRO_TYPE_ANY) {
    } else if (param->policy.profileType == UPGRADE_PRO_TYPE_OPERATIONAL) {
        if (g_ota_card_info->type != PROFILE_TYPE_OPERATIONAL) {
            MSG_PRINTF(LOG_WARN, "unmathed profile type %d/%d\r\n", param->policy.profileType, g_ota_card_info->type);
            ret = UPGRADE_PROFILE_TYPE_ERROR;
            goto exit_entry;  
        }
    } else {
        MSG_PRINTF(LOG_WARN, "unknow profile type %d/%d\r\n", param->policy.profileType, g_ota_card_info->type);
        ret = UPGRADE_PROFILE_TYPE_ERROR;
        goto exit_entry; 
    }

    if (param->target.type >= TARGET_TYPE_MAX) {
        MSG_PRINTF(LOG_WARN, "unknow support target type %d\r\n", param->target.type);
        ret = UPGRADE_TARGET_TYPE_ERROR;
        goto exit_entry;
    }

    if (rt_os_strcmp(param->target.chipModel, g_upload_ver_info->versions[param->target.type].chipModel)) {
        MSG_PRINTF(LOG_WARN, "unmathed chip model [%s] => [%s]\r\n", 
                        g_upload_ver_info->versions[param->target.type].chipModel, param->target.chipModel);
        ret = UPGRADE_CHIP_MODEL_ERROR;
        goto exit_entry;         
    }

    if (param->target.type == TARGET_TYPE_SHARE_PROFILE) {
        /* force to update share profile */
        policy_forced == UPGRADE_MODE_FORCED; 
    } else {
        policy_forced = param->policy.forced;
    }

    if (policy_forced == UPGRADE_MODE_FORCED) {
        MSG_PRINTF(LOG_WARN, "forced to upgrade\r\n");   
    } else {
        if (policy_forced == UPGRADE_MODE_CHK_FILE_NAME) {
            if (rt_os_strcmp(param->target.name, g_upload_ver_info->versions[param->target.type].name)) {
                MSG_PRINTF(LOG_WARN, "unmathed file name [%s] => [%s]\r\n", 
                        g_upload_ver_info->versions[param->target.type].name, param->target.name);
                ret = UPGRADE_FILE_NAME_ERROR;
                goto exit_entry;  
            }
        } else if (policy_forced == UPGRADE_MODE_CHK_VERSION ) {
            if (!ota_policy_compare_version(g_upload_ver_info->versions[param->target.type].version, param->target.version)) {
                MSG_PRINTF(LOG_WARN, "unmathed version name\r\n");
                ret = UPGRADE_CHECK_VERSION_ERROR;
                goto exit_entry; 
            }
        } else if (policy_forced == UPGRADE_MODE_NO_FORCED) {
             if (rt_os_strcmp(param->target.name, g_upload_ver_info->versions[param->target.type].name)) {
                MSG_PRINTF(LOG_WARN, "unmathed file name [%s] => [%s]\r\n", 
                        g_upload_ver_info->versions[param->target.type].name, param->target.name);
                ret = UPGRADE_FILE_NAME_ERROR;
                goto exit_entry; 
             }

             if (!ota_policy_compare_version(g_upload_ver_info->versions[param->target.type].version, param->target.version)) {
                MSG_PRINTF(LOG_WARN, "unmathed version name\r\n");
                ret = UPGRADE_CHECK_VERSION_ERROR;
                goto exit_entry; 
             }
        }
    }

    ret = 0;

exit_entry:

    return ret;
}

#if 0  
/* only SHA256 check */
static rt_bool ota_file_check(const void *arg)
{
    const upgrade_struct_t *d_info = (const upgrade_struct_t *)arg;
    rt_bool ret = RT_FALSE;
    sha256_ctx_t sha_ctx;
    rt_fshandle_t fp = NULL;
    int8_t hash_result[MAX_FILE_HASH_LEN + 1];
    int8_t hash_out[MAX_FILE_HASH_BYTE_LEN + 1];
    int8_t hash_buffer[HASH_CHECK_BLOCK];
    int8_t last_hash_buffer[PRIVATE_HASH_STR_LEN + 1] = {0};
    uint32_t check_size;
    int32_t partlen;
    struct  stat f_info;

    RT_CHECK_ERR(stat((char *)d_info->tmpFileName, &f_info), -1);

    RT_CHECK_ERR(fp = rt_fopen((char *)d_info->tmpFileName, "r") , NULL);

    sha256_init(&sha_ctx);
    f_info.st_size -= PRIVATE_HASH_STR_LEN;
    if (f_info.st_size < HASH_CHECK_BLOCK) {
        rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
        RT_CHECK_ERR(rt_fread(hash_buffer, f_info.st_size, 1, fp), 0);
        sha256_update(&sha_ctx, (uint8_t *)hash_buffer, f_info.st_size);
    } else {
        for (check_size = HASH_CHECK_BLOCK; check_size < f_info.st_size; check_size += HASH_CHECK_BLOCK) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(rt_fread(hash_buffer, HASH_CHECK_BLOCK, 1, fp), 0);
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, HASH_CHECK_BLOCK);
        }

        partlen = f_info.st_size + HASH_CHECK_BLOCK - check_size;
        if (partlen > 0) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(rt_fread(hash_buffer, partlen, 1, fp), 0);
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, partlen);
        }

        RT_CHECK_ERR(rt_fread(last_hash_buffer, PRIVATE_HASH_STR_LEN, 1, fp), 0);
    }

    sha256_final(&sha_ctx, (uint8_t *)hash_out);
    bytestring_to_charstring(hash_out, hash_result, MAX_FILE_HASH_BYTE_LEN);

    MSG_PRINTF(LOG_WARN, "calc hash_result: %s\r\n", hash_result);
    MSG_PRINTF(LOG_WARN, "tail hash_result: %s\r\n", last_hash_buffer);
    RT_CHECK_NEQ(strncpy_case_insensitive(hash_result, last_hash_buffer, MAX_FILE_HASH_LEN), RT_TRUE);
    MSG_PRINTF(LOG_INFO, "private file hash check ok !\r\n");
    ret = RT_TRUE;

end:

    if (fp != NULL) {
        rt_fclose(fp);
    }
    return ret;
}
#else
/* SHA256withECC check */
static rt_bool ota_file_check(const void *arg)
{
    rt_bool ret = RT_FALSE;
    const upgrade_struct_t *d_info = (const upgrade_struct_t *)arg;
    int32_t iret;

    if (d_info->type == TARGET_TYPE_SHARE_PROFILE) {
        /* share profile needn't check signature */
        return RT_TRUE;
    }

    iret = ipc_file_verify_by_monitor((const char *)d_info->tmpFileName);
    ret = !iret ? RT_TRUE : RT_FALSE; 
    return ret;
}
#endif

static rt_bool ota_file_install(const void *arg)
{
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;
    rt_bool ret = RT_FALSE;
    
    /* app replace */
    MSG_PRINTF(LOG_INFO, "tmpFileName=%s, targetFileName=%s\r\n", upgrade->tmpFileName, upgrade->targetFileName);
    if (rt_os_rename(upgrade->tmpFileName, upgrade->targetFileName) != 0) {
        MSG_PRINTF(LOG_WARN, "re-name error\n");
        goto exit_entry;
    }

    /* add app executable mode */
    if (rt_os_chmod(upgrade->targetFileName, RT_S_IRWXU | RT_S_IRWXG | RT_S_IRWXO) != 0) {
        MSG_PRINTF(LOG_WARN, "change mode error\n");
        goto exit_entry;
    }

    /* set upgrade ok flag */
    //SET_UPGRADE_STATUS(upgrade, 1);

    /* sync data to flash */
    rt_os_sync();
    rt_os_sync();

    ret = RT_TRUE;
    
exit_entry:
    
    return ret;
}

static rt_bool ota_file_cleanup(const void *arg)
{
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;

    rt_os_unlink(upgrade->tmpFileName);

    return RT_TRUE;
}

static rt_bool ota_on_upload_event(const void *arg)
{
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;

    upload_event_report(upgrade->event, (const char *)upgrade->tranId, upgrade->downloadResult, (void *)upgrade);

    ota_upgrade_task_cleanup();

    /* restart app right now ? */
    if (UPGRADE_NO_FAILURE == upgrade->downloadResult && upgrade->execute_app_now) {
        if (upgrade->activate) {
            upgrade->activate(NULL);
        }
    }

    /* release upgrade struct memory */
    if (upgrade) {
        rt_os_free((void *)upgrade);
        upgrade = NULL;
    }

    return RT_TRUE;
}

/* make ota downloaded file take active */
static rt_bool ota_file_activate_agent_so(const void *arg)
{
    MSG_PRINTF(LOG_WARN, "sleep %d seconds to restart app !\r\n", MAX_RESTART_WAIT_TIMEOUT);   
    rt_os_sleep(MAX_RESTART_WAIT_TIMEOUT);
    MSG_PRINTF(LOG_WARN, "Current app restart to run new app ...\r\n");        
    rt_os_exit(-1); 

    return RT_TRUE;
}

static rt_bool ota_file_activate_monitor(const void *arg)
{
    MSG_PRINTF(LOG_WARN, "sleep %d seconds to restart monitor !\r\n", MAX_RESTART_WAIT_TIMEOUT);
    ipc_restart_monitor(MAX_RESTART_WAIT_TIMEOUT);
    rt_os_sleep(MAX_RESTART_WAIT_TIMEOUT * 2);
    MSG_PRINTF(LOG_WARN, "Current app restart to run new monitor ...\r\n");

    return RT_TRUE;
}

static file_activate g_file_activate_list[] = 
{
    ota_file_activate_agent_so,
    NULL,
    ota_file_activate_monitor,
    ota_file_activate_agent_so,
};

static int32_t ota_upgrade_start(const void *in, const char *upload_event, const char *tmp_file)
{
    int32_t ret;
    rt_task id;
    uint8_t update_mode = 1;
    uint8_t force_update = 0;
    const ota_upgrade_param_t *param = (const ota_upgrade_param_t *)in;
    upgrade_struct_t *upgrade = NULL;  

    /* create upgrade struct */
    upgrade_process_create(&upgrade);
    OTA_CHK_PINTER_NULL(upgrade, UPGRADE_NULL_POINTER_ERROR);

    /* set upgrade information */
    rt_os_memset(upgrade, 0, sizeof(upgrade_struct_t));
    SET_UPDATEMODE(upgrade, update_mode);
    SET_FORCEUPDATE(upgrade, force_update);
    snprintf(upgrade->tranId, sizeof(upgrade->tranId), "%s", param->tranId);
    snprintf(upgrade->targetName, sizeof(upgrade->targetName), "%s", param->target.name);
    snprintf(upgrade->targetVersion, sizeof(upgrade->targetVersion), "%s", param->target.version);
    snprintf(upgrade->targetChipModel, sizeof(upgrade->targetChipModel), "%s", param->target.chipModel);
    snprintf(upgrade->fileHash, sizeof(upgrade->fileHash), "%s", param->target.fileHash);
    snprintf(upgrade->ticket, sizeof(upgrade->ticket), "%s", param->target.ticket);
    snprintf(upgrade->event, sizeof(upgrade->event), "%s", upload_event);
    upgrade->type           = param->target.type;
    upgrade->size           = param->target.size;
    upgrade->retryAttempts  = param->policy.retryAttempts;
    upgrade->retryInterval  = param->policy.retryInterval;    
    if (!tmp_file) {
        ota_upgrade_get_tmp_file_name(param, upgrade->tmpFileName, sizeof(upgrade->tmpFileName));
    } else {
        snprintf(upgrade->tmpFileName, sizeof(upgrade->tmpFileName), "%s", tmp_file);
    }
    if (ota_upgrade_get_target_file_name(param, upgrade->targetFileName, sizeof(upgrade->targetFileName)) != RT_TRUE) {
        ret = UPGRADE_FILE_NAME_ERROR;
        goto exit_entry;
    }

    /* set callback functions */
    upgrade->check      = ota_file_check;
    upgrade->install    = ota_file_install;
    upgrade->cleanup    = ota_file_cleanup;
    upgrade->on_event   = ota_on_upload_event;
    upgrade->activate   = g_file_activate_list[upgrade->type];

    ota_upgrade_task_record(param, sizeof(ota_upgrade_param_t), upgrade->tmpFileName, upload_event);

    /* check private uprade policy */
    ret = ota_policy_check(param, upgrade);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "ota policy check error\n");
        goto exit_entry;
    }

    /* start upgrade process */
    ret = upgrade_process_start(upgrade);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "Create upgrade start error\n");
        ret = UPGRADE_START_UPGRADE_ERROR;
        goto exit_entry;
    }
    
    ret = 0;
        
exit_entry:

    /* need to upload event here when start upgrade fail */
    if (ret) {
        upgrade->downloadResult = ret;
        ota_on_upload_event((const void *)upgrade);
    }

    return ret;
}

static int32_t ota_upgrade_handler(const void *in, const char *event, void **out)
{
    int32_t ret = -1;
    const ota_upgrade_param_t *param = (const ota_upgrade_param_t *)in;

    (void)out;
    if (param) {
        MSG_PRINTF(LOG_INFO, "upload_event                  : %s\r\n", event);
        MSG_PRINTF(LOG_INFO, "param->tranId                 : %s\r\n", param->tranId);
        MSG_PRINTF(LOG_INFO, "param->target.name            : %s\r\n", param->target.name);
        MSG_PRINTF(LOG_INFO, "param->target.version         : %s\r\n", param->target.version);
        MSG_PRINTF(LOG_INFO, "param->target.chipModel       : %s\r\n", param->target.chipModel);
        MSG_PRINTF(LOG_INFO, "param->target.ticket          : %s\r\n", param->target.ticket);
        MSG_PRINTF(LOG_INFO, "param->target.type            : %d\r\n", param->target.type);
        MSG_PRINTF(LOG_INFO, "param->target.size            : %d\r\n", param->target.size);
        MSG_PRINTF(LOG_INFO, "param->target.fileHash        : %s\r\n", param->target.fileHash);
        MSG_PRINTF(LOG_INFO, "param->policy.forced          : %d\r\n", param->policy.forced);
        MSG_PRINTF(LOG_INFO, "param->policy.executionType   : %s\r\n", param->policy.executionType);
        MSG_PRINTF(LOG_INFO, "param->policy.profileType     : %d\r\n", param->policy.profileType);
        MSG_PRINTF(LOG_INFO, "param->policy.retryAttempts   : %d\r\n", param->policy.retryAttempts);
        MSG_PRINTF(LOG_INFO, "param->policy.retryInterval   : %d\r\n", param->policy.retryInterval);

        ret = ota_upgrade_start(param, event, NULL);

        /* release input param memory */
        if (param) {
            rt_os_free((void *)param);
            param = NULL;
        }
    }

exit_entry:

    return ret;
}

static cJSON *ota_upgrade_packer(void *arg)
{
    int32_t ret = 0;
    cJSON *app_version = NULL;
    const upgrade_struct_t *upgrade = (const upgrade_struct_t *)arg;
    const char *name = "";
    const char *version = "";
    const char *chipModel = "";
    int32_t type;

    if (upgrade) {
        name = upgrade->targetName;
        version = upgrade->targetVersion;
        chipModel = upgrade->targetChipModel;
        type = upgrade->type;
    } else {
        MSG_PRINTF(LOG_WARN, "error param input !\n");
    }

    app_version = cJSON_CreateObject();
    if (!app_version) {
        MSG_PRINTF(LOG_WARN, "The app_version is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(app_version, name);
    CJSON_ADD_NEW_STR_OBJ(app_version, version);
    CJSON_ADD_NEW_STR_OBJ(app_version, chipModel);
    CJSON_ADD_NEW_INT_OBJ(app_version, type);
    
    ret = 0;
    
exit_entry:

    return !ret ? app_version : NULL;   
}

int32_t ota_upgrade_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t status = 0;
    downstream_msg_t *downstream_msg = (downstream_msg_t *)buf;

    (void)mode;
    MSG_PRINTF(LOG_INFO, "msg: %s (%d bytes) ==> method: %s ==> event: %s\n", 
        downstream_msg->msg, downstream_msg->msg_len, downstream_msg->method, downstream_msg->event);
    
    downstream_msg->parser(downstream_msg->msg, downstream_msg->tranId, &downstream_msg->private_arg);
    if (downstream_msg->msg) {
        rt_os_free(downstream_msg->msg);
        downstream_msg->msg = NULL;
    }
    //MSG_PRINTF(LOG_WARN, "tranId: %s, %p\n", downstream_msg->tranId, downstream_msg->tranId);

    status = downstream_msg->handler(downstream_msg->private_arg, downstream_msg->event, &downstream_msg->out_arg);

    return 0;
}

int32_t init_ota(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    g_ota_card_info = (const card_info_t *)public_value_list->card_info->info;

    MSG_PRINTF(LOG_INFO, "profile type : %p, %d\n", &g_ota_card_info->type, g_ota_card_info->type);

    return 0;
}

DOWNSTREAM_METHOD_OBJ_INIT(UPGRADE, MSG_ID_OTA_UPGRADE, ON_UPGRADE, ota_upgrade_parser, ota_upgrade_handler);

UPLOAD_EVENT_OBJ_INIT(ON_UPGRADE, TOPIC_DEVICEID_OR_EID, ota_upgrade_packer);

#endif

