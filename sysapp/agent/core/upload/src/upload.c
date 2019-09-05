
#include <time.h>

#include "md5.h"
#include "http.h"
#include "agent_queue.h"
#include "agent_main.h"
#include "config.h"
#include "random.h"
#include "upload.h"

#include "cJSON.h"

#define MAX_OTI_URL_LEN             100

#define STRUCTURE_OTI_URL(buf, buf_len, addr, port, interface) \
do{                 \
   snprintf((char *)buf, buf_len, "http://%s:%d%s", addr, port, interface);     \
}while(0)

#define HTTP_POST "POST %s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
    "Content-Type:application/json;charset=UTF-8\r\n"\
    "md5sum:%s\r\n"\
    "Content-Length: %d\r\n\r\n%s"
    
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"

static const char *g_upload_eid     = NULL;
static const char *g_upload_imei    = NULL;
const char *g_push_channel   = NULL;
static uint8_t g_current_mcc[8]     = "460";

int32_t upload_http_post(const char *host_addr, int32_t port, socket_call_back cb, void *buffer, int32_t len)
{
    int32_t socket_fd = -1;
    int8_t  *recv_buf = NULL;
    int32_t offset = 0;
    http_result_e ret = HTTP_SUCCESS;

    do {
        socket_fd = http_tcpclient_create(host_addr, port);       // connect network
        MSG_PRINTF(LOG_INFO, "http_tcpclient_create (%s:%d) fd=%d\n", host_addr, port, socket_fd);
        if (socket_fd < 0) {
            ret = HTTP_SOCKET_CONNECT_ERROR;
            MSG_PRINTF(LOG_WARN, "http_tcpclient_create failed\n");
            break;
        }

        //MSG_HEXDUMP("http-send", buffer, len);
        MSG_PRINTF(LOG_INFO, "http post send (%d bytes): %s\r\n", len, buffer);
        if (http_tcpclient_send(socket_fd, buffer, len) < 0) {      // send data
            ret = HTTP_SOCKET_SEND_ERROR;
            MSG_PRINTF(LOG_WARN, "http_tcpclient_send failed..\n");
            break;
        }
        
        recv_buf = (int8_t *)rt_os_malloc(BUFFER_SIZE * 4);
        if (!recv_buf) {
            ret = HTTP_SYSTEM_CALL_ERROR;
            MSG_PRINTF(LOG_WARN, "lpbuf memory alloc error\n");
            break;
        }

        /*it's time to recv from server*/
        rt_os_memset(recv_buf, 0, BUFFER_SIZE * 4);
        if (http_tcpclient_recv(socket_fd, recv_buf, BUFFER_SIZE * 4) <= 0) {     // recv data
            ret = HTTP_SOCKET_RECV_ERROR;
            MSG_PRINTF(LOG_WARN, "http_tcpclient_recv failed\n");
            break;
        } else {
            //MSG_PRINTF(LOG_INFO, "recv_buf: %s\n", recv_buf);
            offset = http_parse_result(recv_buf);
            MSG_PRINTF(LOG_WARN, "%s\n", recv_buf + offset);
            if (cb(recv_buf + offset) != 0) {
            ret = HTTP_RESPOND_ERROR;
            }
        }
    } while (0);

    if (recv_buf) {
        rt_os_free(recv_buf);
    }

    http_tcpclient_close(socket_fd);
    
    return ret;     
}

static int32_t upload_deal_rsp_msg(int8_t *msg)
{
    int32_t state = RT_ERROR;
    cJSON *rsp_msg = NULL;
    cJSON *back_state = NULL;

    rsp_msg = cJSON_Parse((char *)msg);
    if (!rsp_msg) {
        MSG_PRINTF(LOG_WARN, "rsp_msg error\n");
        return RT_ERROR;
    }
    
    back_state = cJSON_GetObjectItem(rsp_msg, "status");
    if (back_state == NULL) {
        MSG_PRINTF(LOG_WARN, "back_state error\n");
    } else {
        if (back_state->valueint == 0) {
            state = RT_SUCCESS;
        } else {
            MSG_PRINTF(LOG_WARN, "state error string: %s\r\n", (const char*)msg);  
        }
    }
    if (rsp_msg) {
        cJSON_Delete(rsp_msg);
    }
    return state;
}

static int32_t upload_send_request(const char *out)
{
    int32_t ret = RT_ERROR;
    char md5_out[MD5_STRING_LENGTH + 1];
    char upload_url[MAX_OTI_URL_LEN + 1];
    char file[100] = {0};
    char host_addr[HOST_ADDRESS_LEN];
    int32_t port = 0;
    uint8_t lpbuf[BUFFER_SIZE * 4] = {0};
    int32_t send_len;

    if (!out) {
        MSG_PRINTF(LOG_WARN, "out buffer error\n");
        ret = HTTP_PARAMETER_ERROR;
        return ret;
    }

    MSG_PRINTF(LOG_WARN, "len=%d, Upload:%s\r\n", strlen((const char *)out), (const char *)out);
    
    get_md5_string((int8_t *)out, md5_out);
    md5_out[MD5_STRING_LENGTH] = '\0';

    //send report by http
    STRUCTURE_OTI_URL(upload_url, MAX_OTI_URL_LEN + 1, OTI_ENVIRONMENT_ADDR, 7082, "/api/v2/report");
    MSG_PRINTF(LOG_INFO, "OTI_ENVIRONMENT_ADDR=%s, upload_url=%s\r\n", OTI_ENVIRONMENT_ADDR, upload_url);
    if (http_parse_url(upload_url, host_addr, file, &port)) {
        MSG_PRINTF(LOG_WARN, "http_parse_url failed!\n");
        ret = HTTP_PARAMETER_ERROR;
        goto exit_entry;
    }

    snprintf(lpbuf, BUFFER_SIZE * 4, HTTP_POST, file, host_addr, port, md5_out, rt_os_strlen(out), out);
    
    send_len = rt_os_strlen(lpbuf);
    ret = msg_send_upload_queue(host_addr, port, upload_deal_rsp_msg, lpbuf, send_len);
    MSG_PRINTF(LOG_INFO, "send queue %d bytes, ret=%d\r\n", send_len, ret);
    
exit_entry:
    
    return ret;
}

static void upload_get_random_tran_id(char *tran_id, uint16_t size)
{
    int32_t i,flag;
       
    for(i = 0; i < size; i ++) {
        flag = rt_get_random_num() % 3;
        switch(flag)
        {
            case 0:
                tran_id[i] = rt_get_random_num() % 26 + 'a'; 
                break;

            case 1:
                tran_id[i] = rt_get_random_num() % 26 + 'A'; 
                break;
            
            case 2:
                tran_id[i] = rt_get_random_num() % 10 + '0'; 
                break;
        }
    }
}

#if 0
{
    "tranId": "6c90fcc5-a30d-444f-9ba4-5bc4338956c7",
    "version": 0,
    "timestamp": 1566284086,
    "topic": "89086001202200101018000001017002",
    "payload": \"" {
        "event": "INFO",
        "status": 0,
        "content": {}
     } \""
}
#endif

static int32_t upload_packet_header_info(cJSON *upload, const char *tran_id)
{
    char random_tran_id[NORMAL_TRAN_ID_LEN + 1] = {0};
    const char *tranId = tran_id;
    const char *topic = g_upload_eid ? g_upload_eid : "";
    int32_t version = 0;
    time_t timestamp = time(NULL);
    
    if (!tranId || !rt_os_strlen(tranId)) {        
        upload_get_random_tran_id(random_tran_id, sizeof(random_tran_id) - 1);
        tranId = (const char *)random_tran_id;
    }

    CJSON_ADD_NEW_STR_OBJ(upload, tranId);
    CJSON_ADD_NEW_INT_OBJ(upload, version);
    CJSON_ADD_NEW_INT_OBJ(upload, timestamp);
    CJSON_ADD_NEW_STR_OBJ(upload, topic);

    return 0;
}

static int32_t upload_packet_payload(cJSON *upload, const char *event, int32_t status, const cJSON *content)
{
    int32_t ret;
    cJSON *payload_json = NULL;
    char *payload = NULL;
     
    payload_json = cJSON_CreateObject();
    if (!payload_json) {
        MSG_PRINTF(LOG_WARN, "The payload_json is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(payload_json, event);
    CJSON_ADD_NEW_INT_OBJ(payload_json, status);
    CJSON_ADD_STR_OBJ(payload_json, content);

    payload = (char *)cJSON_PrintUnformatted(payload_json);
    
    CJSON_ADD_NEW_STR_OBJ(upload, payload);

    ret = 0;

exit_entry:

    if (payload_json) {
        cJSON_Delete(payload_json);
        payload_json = NULL;
    }

    if (payload) {
        cJSON_free(payload);
        payload = NULL;
    }

    return ret;
}

static cJSON *upload_packet_all(const char *tran_id, const char *event, int32_t status, const cJSON *content)
{
    int32_t ret;
    cJSON *upload = NULL;
    
    upload = cJSON_CreateObject();
    if (!upload) {
        MSG_PRINTF(LOG_WARN, "The upload is error\n");
        ret = -1;
        goto exit_entry;
    }

    upload_packet_header_info(upload, tran_id);
    upload_packet_payload(upload, event, status, content);

    ret = 0;
    
exit_entry:

    return !ret ? upload : NULL;
}

static int32_t init_upload_obj(void)
{
    const upload_event_t *obj = NULL;
    cJSON *ret;
    
    MSG_PRINTF(LOG_WARN, "event_START ~ event_END : %p ~ %p\r\n", g_upload_event_START, g_upload_event_END);
    for (obj = g_upload_event_START; obj <= g_upload_event_END; obj++) {
        MSG_PRINTF(LOG_WARN, "upload %p, %s ...\r\n", obj, obj->event);
        ret = obj->packer(NULL);
    }

    return 0;
}

int32_t upload_event_report(const char *event, const char *tran_id, int32_t status, void *private_arg)
{
    const upload_event_t *obj = NULL;

    for (obj = g_upload_event_START; obj <= g_upload_event_END; obj++) {
        //MSG_PRINTF(LOG_WARN, "upload %p, %s ...\r\n", obj, obj->event);
        if (!rt_os_strcmp(obj->event, event)) {
            char *upload_json_pag = NULL;
            cJSON *upload = NULL;
            cJSON *content = NULL;
            int32_t status = 0; 
            int32_t ret;
            
            content = obj->packer(private_arg);
            MSG_PRINTF(LOG_WARN, "content [%p] tran_id: %s, status: %d !!!\r\n", content, tran_id, status);
            upload = upload_packet_all(tran_id, event, status, content);
            MSG_PRINTF(LOG_WARN, "upload [%p] !!!\r\n", upload);
            upload_json_pag = (char *)cJSON_PrintUnformatted(upload); 
            MSG_PRINTF(LOG_WARN, "upload_json_pag [%p] !!!\r\n", upload_json_pag);
            ret = upload_send_request((const char *)upload_json_pag);
MSG_PRINTF(LOG_WARN, "upload_json_pag [%p] !!!\r\n", upload_json_pag);
            if (upload) {
                cJSON_Delete(upload);
            }
MSG_PRINTF(LOG_WARN, "upload_json_pag [%p] !!!\r\n", upload_json_pag);
            if (upload_json_pag) {
                cJSON_free(upload_json_pag);
            }
            MSG_PRINTF(LOG_WARN, "upload_json_pag [%p] !!!\r\n", upload_json_pag);
            return ret;
        }
    }

    MSG_PRINTF(LOG_WARN, "Unknow upload event [%s] !!!\r\n", event);
    return 0;  
}

int32_t init_upload(void *arg)
{
    rt_bool report_all_info;
    public_value_list_t *public_value_list = (public_value_list_t *)arg;
    
    MSG_PRINTF(LOG_WARN, "eid : %p, %s\n", public_value_list->eid, public_value_list->eid);
    MSG_PRINTF(LOG_WARN, "imei: %p, %s\n", public_value_list->imei, public_value_list->imei);
    g_upload_eid = (const char *)public_value_list->eid;
    g_upload_imei = (const char *)public_value_list->imei;
    g_push_channel = (const char *)public_value_list->push_channel;

    return 0;

    rt_os_sleep(10);

    init_upload_obj();
    
    upload_event_report("REGISTERED", NULL, 0, NULL);
    upload_event_report("BOOT", NULL, 0, NULL);
    report_all_info = RT_FALSE;
    upload_event_report("INFO", NULL, 0, &report_all_info);
    report_all_info = RT_TRUE;
    upload_event_report("INFO", NULL, 0, &report_all_info);
    
    return 0;
}

