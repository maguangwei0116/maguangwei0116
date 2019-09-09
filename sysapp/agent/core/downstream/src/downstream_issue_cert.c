
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"

#include "cJSON.h"

static int32_t downstream_issue_cert_parser(const void *in, char *tranId, void **out)
{
    int32_t ret;
    cJSON *msg = NULL;
    cJSON *tran_id = NULL;

    MSG_PRINTF(LOG_WARN, "\n");

    msg =  cJSON_Parse((const char *)in);
    if (!msg) {
        MSG_PRINTF(LOG_WARN, "msg error\n");
        ret = -1;
        goto exit_entry;
    }

    tran_id = cJSON_GetObjectItem(msg, "tranId");
    if (!tran_id) {
        MSG_PRINTF(LOG_WARN, "tran_id error\n");
        ret = -2;
        goto exit_entry;
    }

    rt_os_strcpy(tranId, tran_id->valuestring);
    MSG_PRINTF(LOG_WARN, "tranId: %s, %p, stelen=%d\n", tranId, tranId, rt_os_strlen(tran_id->valuestring));

    ret = 0;

    exit_entry:

    if (msg != NULL) {
        cJSON_Delete(msg);
        msg = NULL;
    }

    return ret;
}

rt_bool upgrade_download_package(issue_cert_struct_t *d_info)
{
    rt_bool ret = RT_FALSE;
    http_client_struct_t dw_struct;
    int8_t file_path[MAX_FILE_PATH_LEN + 1];
    cJSON  *post_info = NULL;
    int8_t *out;
    int8_t  buf[100];
    uint8_t down_try = 0;
    int8_t timer = 0;  // used to count the number of download

    g_download_flag = 1;  // set download flag
    dw_struct.if_continue = 1;
    dw_struct.buf = NULL;

    /* ???????????Http????body */
    snprintf((char *)file_path, MAX_FILE_PATH_LEN + 1, "%s%s", BACKUP_PATH, "cert");  // Build a complete path to download files
    dw_struct.file_path = (const char *)file_path;
    dw_struct.manager_type = 1;
    dw_struct.http_header.method = 0;  // POST
    STRUCTURE_OTI_URL(buf, 100, OTI_ENVIRONMENT_ADDR, DEFAULT_OTI_ENVIRONMENT_PORT, "/api/v1/download");  // Build the OTI address
    rt_os_memcpy(dw_struct.http_header.url, buf, rt_os_strlen(buf));
    dw_struct.http_header.url[rt_os_strlen(buf)] = '\0';
    dw_struct.http_header.version = 0;
    dw_struct.http_header.record_size = 0;
    post_info = cJSON_CreateObject();
    cJSON_AddItemToObject(post_info,"ticket",cJSON_CreateString((char *)d_info->ticket));
    out = (int8_t *)cJSON_PrintUnformatted(post_info);
    rt_os_memcpy(dw_struct.http_header.buf, out, rt_os_strlen(out));
    dw_struct.http_header.buf[rt_os_strlen(out)] = '\0';

    snprintf((char *)buf, 100, "%s:%d",OTI_ENVIRONMENT_ADDR, DEFAULT_OTI_ENVIRONMENT_PORT);
    http_set_header_record(&dw_struct, "HOST", buf);

    http_set_header_record(&dw_struct, "Content-Type", "application/json");
    http_set_header_record(&dw_struct, "Accept", "application/octet-stream");

    snprintf((char *)buf, 100, "%d", rt_os_strlen(dw_struct.http_header.buf));
    http_set_header_record(&dw_struct, "Content-Length", (const char *)buf);

    /* ????body??md5��???? */
    get_md5_string((int8_t *)dw_struct.http_header.buf, buf);
    buf[MD5_STRING_LENGTH] = '\0';
    http_set_header_record(&dw_struct, "md5sum", (const char *)buf);

    do {
#if 0
        /* wait network connecting*/
        while(get_network_state() != NETWORK_CONNECTING && get_network_state() != NETWORK_USING) {
            rt_os_sleep(1);
        };
#endif

        /*There is file need to download in system*/
        if (rt_os_access((const int8_t *)file_path, F_OK) == RT_SUCCESS){
            snprintf(buf, 100, "%d", get_file_size(file_path));
            http_set_header_record(&dw_struct, "Range", (const char *)buf);
        }

        if (http_client_file_download(&dw_struct) == 0) {
            g_download_flag = 0;  // download success reset flag
            ret = RT_TRUE;
            break;
        }
        timer++;
        rt_os_sleep(1);
    } while(timer < MAX_DOWNLOAD_TIMER);
    cJSON_free(out);
    cJSON_Delete(post_info);
    return ret;
}

static int32_t downstream_issue_cert_handler(const void *in, void **out)
{
    int32_t ret = 0;

    const char *msg = (const char *)in;

    MSG_PRINTF(LOG_WARN, "\n");

    exit_entry:

    return ret;
}

DOWNSTREAM_METHOD_OBJ_INIT(INSPECT, MSG_ID_PERSONLLISE, INFO, downstream_issue_cert_parser, downstream_issue_cert_handler);


