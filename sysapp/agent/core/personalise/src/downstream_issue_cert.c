
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "http_client.h"
#include "config.h"
#include "cJSON.h"
#include "md5.h"
#include "upgrade.h"

typedef struct _issue_cert_struct_t {
    char                    tranId[64];
    char                    ticket[64];
    char                    fileHash[72];
} issue_cert_struct_t;

int8_t g_downstream_flag = 0;

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

static int32_t downstream_issue_cert_parser(const void *in, char *tran_id, void **out)
{
    int32_t ret;
    cJSON *msg = NULL;
    cJSON *tranId = NULL;
    cJSON *payload = NULL;
    char *payload_str = NULL;
    cJSON *payload_json = NULL;
    cJSON *content = NULL;
    cJSON *target = NULL;
    cJSON *tmp = NULL;
    issue_cert_struct_t *param = NULL;

    msg =  cJSON_Parse((const char *)in);

    cJSON_GET_JSON_DATA(msg, tranId);
    
    rt_os_strcpy(tran_id, tranId->valuestring);

    param = (issue_cert_struct_t *)rt_os_malloc(sizeof(issue_cert_struct_t));
    rt_os_memset(param, 0, sizeof(issue_cert_struct_t));
    strncpy(param->tranId, tranId->valuestring, rt_os_strlen(tranId->valuestring));

    cJSON_GET_JSON_DATA(msg, payload);

    payload_json = cJSON_Parse((const char *)payload->valuestring);

    cJSON_GET_JSON_DATA(payload_json, content);

    cJSON_GET_STR_DATA(content, fileHash, param->fileHash, sizeof(param->fileHash), tmp);
    cJSON_GET_STR_DATA(content, ticket, param->ticket, sizeof(param->ticket), tmp);

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

rt_bool upgrade_download_package(const void *in)
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
    snprintf(upgrade_info->fileName, sizeof(upgrade_info->fileName), "%s", "cert");
    snprintf(upgrade_info->fileHash, sizeof(upgrade_info->fileHash), "%s", param->fileHash);
    snprintf(upgrade_info->ticket, sizeof(upgrade_info->ticket), "%s", param->ticket);
    snprintf(upgrade_info->targetFileName, sizeof(upgrade_info->targetFileName), "%s", "/data/cert");
    
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

static int32_t downstream_issue_cert_handler(const void *in, void **out)
{
    int32_t ret = 0;

    const issue_cert_struct_t *param = (const issue_cert_struct_t *)in;
    upgrade_download_package(param);

    exit_entry:

    return ret;
}

DOWNSTREAM_METHOD_OBJ_INIT(ISSUE_CERT, MSG_ID_PERSONLLISE, ON_ISSUE_CERT, downstream_issue_cert_parser, downstream_issue_cert_handler);


