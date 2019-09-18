
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "http_client.h"
#include "config.h"
#include "cJSON.h"

typedef struct _log_param_t {
    char tranId[64];
    char level[8];
    char startTime[32];
    char endTime[32];
} log_param_t;

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

static int32_t downstream_log_parser(const void *in, char *tran_id, void **out)
{
    int32_t ret;
    cJSON *msg = NULL;
    cJSON *tranId = NULL;
    cJSON *payload = NULL;
    char *payload_str = NULL;
    cJSON *payload_json = NULL;
    cJSON *content = NULL;
    cJSON *tmp = NULL;
    log_param_t *param = NULL;

    msg =  cJSON_Parse((const char *)in);
    OTA_CHK_PINTER_NULL(msg, -1);

    cJSON_GET_JSON_DATA(msg, tranId);
    OTA_CHK_PINTER_NULL(tranId, -2);
    
    rt_os_strcpy(tran_id, tranId->valuestring);

    param = (log_param_t *)rt_os_malloc(sizeof(log_param_t));
    OTA_CHK_PINTER_NULL(param, -3);
    rt_os_memset(param, 0, sizeof(log_param_t));
    strncpy(param->tranId, tranId->valuestring, rt_os_strlen(tranId->valuestring));

    cJSON_GET_JSON_DATA(msg, payload);
    OTA_CHK_PINTER_NULL(param, -4);

    payload_json = cJSON_Parse((const char *)payload->valuestring);
    OTA_CHK_PINTER_NULL(payload_json, -5);

    cJSON_GET_JSON_DATA(payload_json, content);
    OTA_CHK_PINTER_NULL(content, -6);

    cJSON_GET_STR_DATA(content, level, param->level, sizeof(param->level), tmp);
    cJSON_GET_STR_DATA(content, startTime, param->startTime, sizeof(param->startTime), tmp);
    cJSON_GET_STR_DATA(content, endTime, param->endTime, sizeof(param->endTime), tmp);

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

#define MAX_OTI_URL_LEN             100
#define LOG_NAME                    "/data/redtea/rt_log"
#define BOUNDARY                    "2128e33d-b5b5-40e8-a5a6-aadd7db1c23f"

#define RT_LOG_HEAD                 "--%s\r\n"\
    "Content-Disposition: form-data; name=\"logfile\"; filename=\"%s\"\r\n"\
    "Content-Type: application/octet-stream\r\n\r\n"
    
#define CONTENT_TYPE                "multipart/form-data;boundary=\"2128e33d-b5b5-40e8-a5a6-aadd7db1c23f\""

#define LOG_NAME                    "/data/redtea/rt_log"
#define LOG_CUT_FILE                "/data/redtea/log_cut"
#define LOG_CUT_LEVEL               INFORMATION
#define LOG_API                     "/api/v2/log" 
#define OTI_ENVIRONMENT_PORT        7082
#define MAX_UPLOAD_RETRY_TIMES      3

typedef enum {
    ERROR = 0,
    INFORMATION
} _log_level_e;

#define STRUCTURE_OTI_URL(buf, buf_len, addr, port, interface) \
do {                 \
    snprintf((char *)buf, buf_len, "http://%s:%d%s", addr, port, interface);     \
} while(0)

static const char *g_upload_log_eid = NULL;

static int32_t log_cut_file(const int8_t *file_name, log_level_e log_level)
{
    char cmd[100];
    char buf[512];
    int32_t offset;

    snprintf((char *)cmd, sizeof(cmd), "touch %s", file_name);
    system(cmd);
    MSG_PRINTF(LOG_DBG, "1--> %s, g_upload_log_eid=%s\r\n", cmd, g_upload_log_eid);

    snprintf((char *)buf, sizeof(buf), RT_LOG_HEAD, BOUNDARY, (char *)g_upload_log_eid);
    if (rt_write_data((uint8_t *)file_name, 0, buf, rt_os_strlen(buf)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "write %s error\n",file_name);
        return RT_ERROR;
    }

    snprintf((char *)cmd, sizeof(cmd), "cat %s >> %s", LOG_NAME, file_name);
    system(cmd);
    MSG_PRINTF(LOG_DBG, "2--> %s\n", cmd);

    snprintf(buf, sizeof(buf), "\r\n--%s--", BOUNDARY);

    offset = get_file_size(file_name);
    if (offset <= 0) {
        MSG_PRINTF(LOG_WARN, "read file size error\n");
        return RT_ERROR;
    }
    MSG_PRINTF(LOG_DBG, "3--> offset=%d\n", offset);
    
    return rt_write_data((uint8_t *)file_name, offset, buf, rt_os_strlen(buf));
}

static void log_file_upload(void *arg)
{
    http_client_struct_t *obj;
    int32_t log_length;
    int32_t up_try_count = 0;
    char log_length_char[10];    
    char log_url[MAX_OTI_URL_LEN + 1];
    char log_host[MAX_OTI_URL_LEN + 1];
    const log_param_t *param = (log_param_t *)arg;
    const char *tranId;

    tranId = param->tranId;
    RT_CHECK_ERR(obj = (http_client_struct_t *)rt_os_malloc(sizeof(http_client_struct_t)), NULL);
    RT_CHECK_NEQ(log_cut_file((const int8_t *)LOG_CUT_FILE, LOG_CUT_LEVEL), 0);  // log cut
    RT_CHECK_LESE(log_length = get_file_size(LOG_CUT_FILE), 0);  // get log file size

    snprintf((char *)log_length_char, sizeof(log_length_char), "%d", log_length);
    obj->manager_type = 0;  // uoload
    obj->http_header.method = 0;  // POST

    STRUCTURE_OTI_URL(log_url, MAX_OTI_URL_LEN + 1, OTI_ENVIRONMENT_ADDR, OTI_ENVIRONMENT_PORT, LOG_API);
    snprintf((char *)log_host, MAX_OTI_URL_LEN + 1, "%s:%d", OTI_ENVIRONMENT_ADDR, OTI_ENVIRONMENT_PORT);

    rt_os_memcpy(obj->http_header.url, log_url, rt_os_strlen(log_url));
    obj->http_header.url[rt_os_strlen(log_url)] = '\0';
    obj->http_header.version = 0;
    obj->http_header.record_size = 0;
    http_set_header_record(obj, "Content-Type", CONTENT_TYPE);
    http_set_header_record(obj, "Content-Length", (const char *)log_length_char);
    http_set_header_record(obj, "HOST", log_host);
    http_set_header_record(obj, "tranId", tranId);
    obj->file_path = LOG_CUT_FILE;

    while (1) {
        if (http_client_file_upload(obj) != 0) {
            rt_os_sleep(3);
            MSG_PRINTF(LOG_WARN, "http_client_file_upload error, up_try_count=%d\r\n", up_try_count);
            if (++up_try_count > MAX_UPLOAD_RETRY_TIMES) {
                break;
            }
        } else {
            break;
        }
    }

end:
    rt_os_free(obj);
    rt_os_free((void *)param);
    rt_os_unlink(LOG_CUT_FILE);
    return;
}

static void msg_deal_with_log(const log_param_t *param)
{
    rt_task task_id;
    if (rt_create_task(&task_id , (void *)log_file_upload, (void *)param) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_create_task log_file_upload error !!!\n");
    }
}

static int32_t downstream_log_handler(const void *in, const char *event, void **out)
{
    int32_t ret = 0;
    const log_param_t *param = (log_param_t *)in;

    (void)event;
    if (param) {
        MSG_PRINTF(LOG_INFO, "upload_event      : %s\r\n", event);
        MSG_PRINTF(LOG_INFO, "param->tranId     : %s\r\n", param->tranId);
        MSG_PRINTF(LOG_INFO, "param->level      : %s\r\n", param->level);
        MSG_PRINTF(LOG_INFO, "param->startTime  : %s\r\n", param->startTime);
        MSG_PRINTF(LOG_INFO, "param->endTime    : %s\r\n", param->endTime);

        msg_deal_with_log(param);
    }
 
    *out = NULL;

exit_entry:

    return ret;
}

DOWNSTREAM_METHOD_OBJ_INIT(LOG, MSG_ID_IDLE, NULL, downstream_log_parser, downstream_log_handler);

int32_t init_logm(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;
    g_upload_log_eid = (const char *)public_value_list->card_info->eid;

    return 0;
}

