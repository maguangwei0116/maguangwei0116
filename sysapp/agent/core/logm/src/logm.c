
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "http_client.h"
#include "config.h"
#include "cJSON.h"

typedef struct LOG_PARAM {
    char            tranId[64];
    char            level[8];
    log_level_e     log_level;
    char            startTime[32];
    char            endTime[32];
} log_param_t;

typedef struct LOG_LEVEL_ITEM {
    log_level_e     level;
    const char *    label;
} log_item_t;

static log_item_t g_log_item_table[] =
{
    {LOG_NONE,      "NONE",},
    {LOG_ERR,       "ERROR",},  // default
    {LOG_WARN,      "WARN",},
    {LOG_DBG,       "DEBUG",},
    {LOG_INFO,      "INFO",},
    {LOG_ALL,       "ALL",},
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

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

static log_level_e downstream_log_get_level(const char *level_str)
{
    log_level_e ret_level = LOG_ALL;
    int32_t i;

    for (i = 0; i < ARRAY_SIZE(g_log_item_table); i++) {
        if (!rt_os_strcmp(g_log_item_table[i].label, level_str)) {
        	ret_level = g_log_item_table[i].level;
        	break;
        }
    }

    return ret_level;
}

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
    rt_os_strncpy(param->tranId, tranId->valuestring, rt_os_strlen(tranId->valuestring));

    cJSON_GET_JSON_DATA(msg, payload);
    OTA_CHK_PINTER_NULL(param, -4);

    payload_json = cJSON_Parse((const char *)payload->valuestring);
    OTA_CHK_PINTER_NULL(payload_json, -5);

    cJSON_GET_JSON_DATA(payload_json, content);
    OTA_CHK_PINTER_NULL(content, -6);

    cJSON_GET_STR_DATA(content, level, param->level, sizeof(param->level), tmp);
    cJSON_GET_STR_DATA(content, startTime, param->startTime, sizeof(param->startTime), tmp);
    cJSON_GET_STR_DATA(content, endTime, param->endTime, sizeof(param->endTime), tmp);

    param->log_level = downstream_log_get_level(param->level);

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

#define MAX_OTI_URL_LEN             128
#define BOUNDARY                    "2128e33d-b5b5-40e8-a5a6-aadd7db1c23f"

#define RT_LOG_HEAD                 "--%s\r\n"\
                                    "Content-Disposition: form-data; name=\"logfile\"; filename=\"%s\"\r\n"\
                                    "Content-Type: application/octet-stream\r\n\r\n"

#define CONTENT_TYPE                "multipart/form-data;boundary=\"2128e33d-b5b5-40e8-a5a6-aadd7db1c23f\""

#define LOG_NAME                    "/data/redtea/rt_log"
#define LOG_CUT_TMP_FILE            "/data/redtea/log_tmp_cut"
#define LOG_CUT_FILE                "/data/redtea/log_cut"
#define LOG_API                     "/api/v2/log"
#define OTI_ENVIRONMENT_PORT        7082
#define MAX_UPLOAD_RETRY_TIMES      3

#define STRUCTURE_OTI_URL(buf, buf_len, addr, port, interface) \
do {                 \
    snprintf((char *)buf, buf_len, "http://%s:%d%s", addr, port, interface);     \
} while(0)

static const char *g_upload_log_eid         = NULL;
static const char *g_upload_log_oti_addr    = NULL;

static int32_t log_file_cut(const char *file_name, log_level_e log_level)
{
    char cmd[100];
    char buf[512];
    int32_t offset;
    int32_t ret;
    const char *tmp_file_name = LOG_CUT_TMP_FILE;

    /* create a tmp log file */
    snprintf((char *)cmd, sizeof(cmd), "cp -rf %s %s && touch %s", LOG_NAME, tmp_file_name, file_name);
    system(cmd);
    MSG_PRINTF(LOG_DBG, "1--> %s, g_upload_log_eid=%s\r\n", cmd, g_upload_log_eid);

    /* add detail information on the header of the upload log file */
    snprintf((char *)buf, sizeof(buf), RT_LOG_HEAD, BOUNDARY, (char *)g_upload_log_eid);
    if (rt_write_data((uint8_t *)file_name, 0, buf, rt_os_strlen(buf)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "write %s error\n",file_name);
        ret = RT_ERROR;
        goto exit_entry;
    }

    /* copy out selective log data */
    MSG_PRINTF(LOG_DBG, "----> log cut start ... log_level = %d\n", log_level);
    log_file_copy_out(tmp_file_name, file_name, log_level);
    MSG_PRINTF(LOG_DBG, "----> log cut end ...\n");

    /* delete tmp log file */
    rt_os_unlink(tmp_file_name);

    /* add boundary information on the tail of the upload log file */
    offset = linux_file_size(file_name);
    if (offset <= 0) {
        MSG_PRINTF(LOG_WARN, "read file size error\n");
        ret = RT_ERROR;
        goto exit_entry;
    }
    snprintf(buf, sizeof(buf), "\r\n--%s--", BOUNDARY);
    if (rt_write_data((uint8_t *)file_name, offset, buf, rt_os_strlen(buf)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "write %s error\n",file_name);
        ret = RT_ERROR;
        goto exit_entry;
    }

    ret = RT_SUCCESS;

 exit_entry:

    return ret;
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
    const char *log_cut_file = LOG_CUT_FILE;

    tranId = param->tranId;
    RT_CHECK_ERR(obj = (http_client_struct_t *)rt_os_malloc(sizeof(http_client_struct_t)), NULL);
    RT_CHECK_NEQ(log_file_cut(log_cut_file, param->log_level), 0);  // log cut
    RT_CHECK_LESE(log_length = linux_file_size(log_cut_file), 0);  // get log file size

    snprintf((char *)log_length_char, sizeof(log_length_char), "%d", log_length);
    obj->manager_type = 0;  // uoload
    obj->http_header.method = 0;  // POST

    STRUCTURE_OTI_URL(log_url, MAX_OTI_URL_LEN + 1, g_upload_log_oti_addr, OTI_ENVIRONMENT_PORT, LOG_API);
    snprintf((char *)log_host, MAX_OTI_URL_LEN + 1, "%s:%d", g_upload_log_oti_addr, OTI_ENVIRONMENT_PORT);

    rt_os_memcpy(obj->http_header.url, log_url, rt_os_strlen(log_url));
    obj->http_header.url[rt_os_strlen(log_url)] = '\0';
    obj->http_header.version = 0;
    obj->http_header.record_size = 0;
    http_set_header_record(obj, "Content-Type", CONTENT_TYPE);
    http_set_header_record(obj, "Content-Length", (const char *)log_length_char);
    http_set_header_record(obj, "HOST", log_host);
    http_set_header_record(obj, "tranId", tranId);
    obj->file_path = log_cut_file;

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
    rt_os_unlink(log_cut_file);
    return;
}

static void downstream_log_start(const log_param_t *param)
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

        downstream_log_start(param);
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
    g_upload_log_oti_addr = (const char *)public_value_list->config_info->oti_addr;

    return 0;
}

