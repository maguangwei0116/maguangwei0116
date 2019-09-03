

#include <time.h>

#include "md5.h"
#include "http.h"
#include "agent_queue.h"
#include "config.h"
#include "random.h"

#include "cJSON.h"

#define MAX_OTI_URL_LEN             100
#define MAX_TRAN_ID_LEN             32

#define STRUCTURE_OTI_URL(buf, buf_len, addr, port, interface) \
do{                 \
   snprintf((char *)buf, buf_len, "http://%s:%d%s", addr, port, interface);     \
}while(0)

#define HTTP_POST "POST %s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
    "Content-Type:application/json;charset=UTF-8\r\n"\
    "md5sum:%s\r\n"\
    "Content-Length: %d\r\n\r\n%s"
    
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"

static uint8_t g_eid[64] = "89086657727465610100000000000171";;
static uint8_t g_imei[64] = "1234567890ABCDEF";
static uint8_t g_current_mcc[8] = "460";
static uint8_t g_push_channel[8] = "EMQ";

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

        MSG_PRINTF(LOG_INFO, "http post send (%d bytes): %s\r\n", len, buffer);
        if (http_tcpclient_send(socket_fd, buffer, len) < 0) {      // send data
            ret = HTTP_SOCKET_SEND_ERROR;
            MSG_PRINTF(LOG_WARN, "http_tcpclient_send failed..\n");
            break;
        }
        
        recv_buf = (int8_t *)rt_os_malloc(BUFFER_SIZE * 4);
        if (!recv_buf) {
            ret = HTTP_SYSTEM_CALL_ERROR;
            MSG_PRINTF(LOG_WARN, "lpbuf malloc error\n");
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
    char file[100];
    char host_addr[HOST_ADDRESS_LEN];
    int32_t port = 0;
    uint8_t lpbuf[4096] = {0};
    int32_t send_len;

    if (!out) {
        MSG_PRINTF(LOG_WARN, "out buffer error\n");
        ret = HTTP_PARAMETER_ERROR;
        return ret;
    }

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


#define CJSON_ADD_STR_OBJ(father_item, sub_item)        cJSON_AddItemToObject(father_item, #sub_item, (cJSON *)sub_item)
#define CJSON_ADD_NEW_STR_OBJ(father_item, str_item)    cJSON_AddItemToObject(father_item, #str_item, cJSON_CreateString(str_item));
#define CJSON_ADD_NEW_INT_OBJ(father_item, int_item)    cJSON_AddItemToObject(father_item, #int_item, cJSON_CreateNumber(int_item));

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
    "payload": {
        "event": "INFO",
        "status": 0,
        "content": {}
     }
}
#endif

static int32_t upload_packet_header_info(cJSON *upload, const char *tran_id)
{
    char random_tran_id[MAX_TRAN_ID_LEN + 1] = {0};
    const char *tranId = tran_id;
    const char *topic = g_eid;
    int32_t version = 0;
    time_t timestamp = time(NULL);
    
    if (!tranId) {        
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
    cJSON *payload = NULL;
    
    payload = cJSON_CreateObject();
    if (!payload) {
        MSG_PRINTF(LOG_WARN, "The payload is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(payload, event);
    CJSON_ADD_NEW_INT_OBJ(payload, status);
    CJSON_ADD_STR_OBJ(payload, content);
    CJSON_ADD_STR_OBJ(upload, payload);

    ret = 0;

exit_entry:

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

int32_t upload_cmd_registered(void)
{
    int32_t ret;
    int32_t status = 0;
    cJSON *upload = NULL;
    cJSON *content = NULL;    
    char *upload_json_pag = NULL;
    const char *event = "REGISTERED";
    const char *pushChannel = (const char *)g_push_channel;

    MSG_PRINTF(LOG_WARN, "\n----------------->%s\n", event);
    
    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(content, pushChannel);
    MSG_PRINTF(LOG_WARN, "1111111111111\r\n");
    upload = upload_packet_all(NULL, event, status, content);MSG_PRINTF(LOG_WARN, "1111111111111\r\n");
    upload_json_pag = (char *)cJSON_PrintUnformatted(upload);MSG_PRINTF(LOG_WARN, "1111111111111\r\n");
    MSG_PRINTF(LOG_WARN, "1111111111111upload_json_pag=%p, upload=%p\r\n", upload_json_pag, upload);
    MSG_PRINTF(LOG_WARN, "len=%d, Upload:%s\r\n", strlen((const char *)upload_json_pag), (const char *)upload_json_pag);
    
    ret = upload_send_request((const char *)upload_json_pag);

exit_entry:

    if (upload != NULL) {
        cJSON_Delete(upload);
    }

    if (upload_json_pag) {
        rt_os_free(upload_json_pag);
    }

    return ret;
}

static int32_t upload_cmd_boot_info(const char *str_cmd, rt_bool only_profile_network)
{
    int32_t ret = 0;
    int32_t status = 0;
    cJSON *upload = NULL;
    cJSON *content = NULL;
    cJSON *deviceInfo = NULL;
    cJSON *profiles = NULL;
    cJSON *network = NULL;
    cJSON *software = NULL;
    char *upload_json_pag = NULL;
    const char *event = str_cmd;

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
    }

    if (!only_profile_network) {
        deviceInfo = cJSON_CreateObject();
        if (!deviceInfo) {
            MSG_PRINTF(LOG_WARN, "The deviceInfo is error\n");
        }
    }

    profiles = cJSON_CreateObject();
    if (!profiles) {
        MSG_PRINTF(LOG_WARN, "The profiles is error\n");
    }

    network = cJSON_CreateObject();
    if (!network) {
        MSG_PRINTF(LOG_WARN, "The network is error\n");
    }

    if (!only_profile_network) {
        software = cJSON_CreateObject();
        if (!software) {
            MSG_PRINTF(LOG_WARN, "The software is error\n");
        }
    }

    MSG_PRINTF(LOG_WARN, "\n----------------->%s\n", event);
    
    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        ret = -1;
        goto exit_entry;
    }

    if (!only_profile_network) {
        CJSON_ADD_STR_OBJ(content, deviceInfo);
    }
    CJSON_ADD_STR_OBJ(content, profiles);
    CJSON_ADD_STR_OBJ(content, network);
    if (!only_profile_network) {
        CJSON_ADD_STR_OBJ(content, software);
    }

    upload = upload_packet_all(NULL, event, status, content);
    upload_json_pag = (char *)cJSON_PrintUnformatted(upload);
    MSG_PRINTF(LOG_WARN, "len=%d, Upload:%s\r\n", strlen((const char *)upload_json_pag), (const char *)upload_json_pag);
    
    ret = upload_send_request((const char *)upload_json_pag);

exit_entry:

    if (upload != NULL) {
        cJSON_Delete(upload);
    }

    if (upload_json_pag) {
        rt_os_free(upload_json_pag);
    }

    return ret;
}

int32_t upload_cmd_boot(void)
{
    return upload_cmd_boot_info("BOOT", RT_FALSE);
}

int32_t upload_cmd_info(void)
{
    return upload_cmd_boot_info("INFO", RT_TRUE);
}

int32_t init_upload(void *arg)
{
    upload_cmd_registered(); 
    rt_os_sleep(1);
    upload_cmd_boot();
    rt_os_sleep(1);
    upload_cmd_info();
    rt_os_sleep(1);

    return 0;
}

