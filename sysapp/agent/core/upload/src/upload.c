
#include <time.h>

#include "md5.h"
#include "http.h"
#include "agent_queue.h"
#include "config.h"
#include "random.h"
#include "upload.h"
#include "cJSON.h"
#include "https.h"

#define MAX_UPLOAD_TIMES                    3
#define MAX_BOOT_INFO_INTERVAL              5       // unit: second
#define MAX_FILE_NAME_LEN                   128
#ifndef MAX_URL_LEN
#define MAX_URL_LEN                         128
#endif
#define MQTT_UPLOAD_SWITCH_STATE            0       // turn off now !!!

#define HTTP_POST "POST %s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
    "Content-Type:application/json;charset=UTF-8\r\n"\
    "md5sum:%s\r\n"\
    "Content-Length: %d\r\n\r\n%s"

static const char  g_temp_eid[MAX_EID_LEN + 1]  = {0};
static const char *g_upload_eid                 = NULL;
static const char *g_upload_deviceid            = NULL;
static const char *g_upload_addr                = NULL;
static uint32_t g_upload_port                   = 0;
static const char *g_upload_api                 = "/api/v2/report";
static const profile_type_e *g_last_card_type   = NULL;
const char *g_push_channel                      = NULL;
const devicde_info_t *g_upload_device_info      = NULL;
card_info_t *g_upload_card_info                 = NULL;
const target_versions_t *g_upload_ver_info      = NULL;
static rt_bool g_upload_network                 = RT_FALSE;
static rt_bool g_upload_mqtt                    = RT_FALSE;
static profile_type_e *g_upload_card_type       = NULL;


static int32_t upload_http_post_single(const char *host_addr, int32_t port, socket_call_back cb, void *buffer, int32_t len)
{
    MSG_PRINTF(LOG_DBG, "http post send (%d bytes, buf: %p): %s\r\n", len, buffer, (const char *)buffer);
#if (CFG_UPLOAD_HTTPS_ENABLE)
    return https_post_raw(g_upload_addr, g_upload_api, buffer, NULL, NULL, cb);
#else
    return http_post_raw(host_addr, port, buffer, len, cb);
#endif
}

static int32_t upload_deal_rsp_msg(const char *msg)
{
    int32_t state = RT_ERROR;
    cJSON *rsp_msg = NULL;
    cJSON *back_state = NULL;

    rsp_msg = cJSON_Parse((char *) msg);
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
            MSG_PRINTF(LOG_WARN, "state error string: %s\r\n", (const char *)msg);
        }
    }
    if (rsp_msg) {
        cJSON_Delete(rsp_msg);
    }
    return state;
}

static int32_t parse_upload_event(const char *data, int32_t data_len, uint8_t *eid)
{
    int32_t ret = RT_ERROR;
    cJSON *agent_msg = NULL;
    cJSON *tran_id = NULL;
    cJSON *topic = NULL;
    cJSON *payload = NULL;
    cJSON *event = NULL;
    cJSON *content = NULL;
    uint8_t *buf = NULL;    
    int32_t len = 0;

    do {
        agent_msg = cJSON_Parse(data);
        if (!agent_msg) {
            MSG_PRINTF(LOG_ERR, "message parse failed!!\n");
            break;
        }
        tran_id = cJSON_GetObjectItem(agent_msg, "tranId");
        if (!tran_id) {
            MSG_PRINTF(LOG_ERR, "Prase tranId failed!!\n");
            break;
        }
        topic = cJSON_GetObjectItem(agent_msg, "topic");
        if (!topic) {
            MSG_PRINTF(LOG_ERR, "Prase topic failed!!\n");
            break;
        }
        payload = cJSON_GetObjectItem(agent_msg, "payload");
        if (!payload) {
            MSG_PRINTF(LOG_ERR, "Prase payload failed!!\n");
            break;
        }
        len = rt_os_strlen(payload->valuestring);
        buf = (uint8_t *)rt_os_malloc(len + 1);
        if (!buf) {
            MSG_PRINTF(LOG_ERR, "Malloc buf failed!!\n");
            break;
        }
        rt_os_memcpy(buf, payload->valuestring, len);
        buf[len] = '\0';

        payload = cJSON_Parse((uint8_t *)buf);
        if (!payload) {
            MSG_PRINTF(LOG_ERR, "Parse payload failed!!\n");
            break;
        }
        event = cJSON_GetObjectItem(payload, "event");
        if (!event) {
            MSG_PRINTF(LOG_ERR, "Parse event failed!!\n");
            break;
        } 
        len = rt_os_strlen(event->valuestring);
        if (rt_os_strcmp(event->valuestring, "INIT")) {
            break;
        }
        len = rt_os_strlen(topic->valuestring);
        rt_os_memcpy(eid, topic->valuestring, len);
        eid[len] = '\0';
        ret = RT_SUCCESS;
    } while(0);
    
    if (agent_msg != NULL) {
        cJSON_Delete(agent_msg);
    }
    if (buf != NULL) {
        rt_os_free((void *)buf);
    }
    return ret;
}

static int32_t upload_send_http_request(const char *data, int32_t data_len)
{
    int32_t ret = RT_ERROR;
    char md5_out[MD5_STRING_LENGTH + 1];
    char upload_url[MAX_URL_LEN];
    char file[MAX_FILE_NAME_LEN] = {0};
    char host_addr[HOST_ADDRESS_LEN];
    uint8_t lpbuf[BUFFER_SIZE * 4] = {0};
    uint8_t eid[MAX_EID_LEN + 1] = {0};
    int32_t send_len;
    int32_t port = 0;
    int32_t is_init = RT_ERROR;

    if (!data) {
        // MSG_PRINTF(LOG_WARN, "out buffer error\n");
        ret = HTTP_PARAMETER_ERROR;
        return ret;
    }
    is_init = parse_upload_event(data, data_len, eid);

    //MSG_PRINTF(LOG_WARN, "len=%d, Upload:%s\r\n", rt_os_strlen((const char *)out), (const char *)out);

    get_md5_string((int8_t *) data, md5_out);
    md5_out[MD5_STRING_LENGTH] = '\0';

    //send report by http
    snprintf(upload_url, sizeof(upload_url), "http://%s:%d%s", g_upload_addr, g_upload_port, g_upload_api);
    MSG_PRINTF(LOG_TRACE, "upload_url: %s\r\n", upload_url);

    if (http_parse_url(upload_url, host_addr, file, &port)) {
        MSG_PRINTF(LOG_WARN, "http_parse_url failed!\n");
        ret = HTTP_PARAMETER_ERROR;
        goto exit_entry;
    }

    snprintf(lpbuf, BUFFER_SIZE * 4, HTTP_POST, file, host_addr, port, md5_out, data_len, data);
    send_len = rt_os_strlen(lpbuf);

#if (CFG_UPLOAD_HTTPS_ENABLE)
    ret = upload_http_post_single(host_addr, port, upload_deal_rsp_msg, data, rt_os_strlen(data));
#else
    ret = upload_http_post_single(host_addr, port, upload_deal_rsp_msg, lpbuf, send_len);
    // MSG_PRINTF(LOG_INFO, "send queue %d bytes, ret=%d\r\n", send_len, ret);
#endif
    if (is_init == RT_SUCCESS) {
        if (ret == 0) {
            card_update_last_eid((const char *)eid);
            MSG_PRINTF(LOG_INFO, "EID updated : %s\n", eid);
        } else {
            MSG_PRINTF(LOG_TRACE, "upload INIT failed!\n");
        }
    }
exit_entry:

    return ret;
}

static int32_t upload_wait_network_connected(void)
{
    while (1) {
        if (g_upload_network == RT_TRUE) {
            break;
        }
        rt_os_msleep(100);
    }

    return RT_SUCCESS;
}

static int32_t upload_send_mqtt_request(const char *data, int32_t data_len)
{
    return mqtt_pulish_msg((const char *)data, data_len);
}

static int32_t upload_send_final_request(const char *data, int32_t data_len)
{
    int32_t ret = RT_ERROR;

#if (MQTT_UPLOAD_SWITCH_STATE)
    ret = upload_send_mqtt_request(data, data_len);
#endif

    /* send with http when MQTT upload fail, or MQTT disconnected */
    if (ret != RT_SUCCESS) {
        ret = upload_send_http_request(data, data_len);
    }

    return ret;
}

int32_t upload_event_final_report(void *buffer, int32_t len)
{
    int32_t ret = RT_ERROR;
    int32_t i = 0;

    for (i = 0; i < MAX_UPLOAD_TIMES; i++) {
        upload_wait_network_connected();
        MSG_PRINTF(LOG_DBG, "begin to send final report ...\r\n");
        ret = upload_send_final_request(buffer, len);

        if (HTTP_SOCKET_CONNECT_ERROR == ret || HTTP_SOCKET_SEND_ERROR == ret || HTTP_SOCKET_RECV_ERROR == ret) {
            MSG_PRINTF(LOG_WARN, "upload event final report fail, ret=%d, retry i=%d\r\n", ret, i);
            rt_os_sleep(3);
            continue;
        }
        MSG_PRINTF(LOG_DBG, "end to send final report ...\r\n");
        break;
    }
    return ret;
}

static int32_t upload_send_request(const void *data, int32_t data_len)
{
    int32_t ret = RT_ERROR;

    ret = msg_send_upload_queue((void *)data, data_len);

    return ret;
}

/**
 * Create random UUID
 *
 * @param uuid - buffer to be filled with the uuid string
 * @param len  - length of uuid buffer
 */
static char *random_uuid(char *uuid, int32_t len)
{
    const char *magic = "89ab";
    char *p = uuid;
    uint16_t n;
    uint16_t b;

    for( n = 0; n < 16; ++n ) {
        b = rt_get_random_num()%255;

        switch( n ) {
            case 6:
                sprintf(p, "4%x", b%15);
                break;

            case 8:
                sprintf(p, "%c%x", magic[rt_get_random_num() % rt_os_strlen( magic )], b%15 );
                break;

            default:
                sprintf(p, "%02x", b);
                break;
        }

        p += 2;
        switch( n ) {
            case 3:
            case 5:
            case 7:
            case 9:
                *p++ = '-';
                break;
        }
    }

    *p = 0;

    return uuid;
}

static void upload_get_random_tran_id(char *tran_id, uint16_t size)
{
    random_uuid(tran_id, size);
}

static rt_bool upload_check_memory(const void *buf, int32_t len, int32_t value)
{
    int32_t i = 0;
    const uint8_t *p = (const uint8_t *)buf;

    for (i = 0; i < len; i++) {
        if (p[i] != value) {
            return RT_FALSE;
        }
    }
    return RT_TRUE;
}

rt_bool upload_check_eid_empty(void)
{
    return (upload_check_memory(g_upload_eid, MAX_EID_LEN, 'F') || \
                upload_check_memory(g_upload_eid, MAX_EID_LEN, '0')) ? RT_TRUE : RT_FALSE;
}

static const char *upload_get_topic_name(upload_topic_e upload_topic)
{
    if (TOPIC_DEVICEID == upload_topic) {
        return g_upload_deviceid;
    } else {
        if (g_upload_eid) {
            if (upload_check_eid_empty()) {
                return g_upload_deviceid;
            } else {
                return g_upload_eid;
            }
        } else {
            return g_upload_deviceid;
        }
    }
}

static int32_t upload_packet_header_info(cJSON *upload, const char *tran_id, upload_topic_e upload_topic)
{
    char random_tran_id[MAX_UUID_LEN] = {0};
    const char *tranId = tran_id;
    const char *topic = upload_get_topic_name(upload_topic);
    int32_t version = 0;
    time_t timestamp = time(NULL);

    //MSG_PRINTF(LOG_INFO, "The upload g_upload_eid: %s\n", g_upload_eid);
    //MSG_PRINTF(LOG_INFO, "The upload device_id: %s\n", g_upload_device_info->device_id);
    //MSG_PRINTF(LOG_INFO, "The upload topic: %s\n", topic);
    if (!tranId || !rt_os_strlen(tranId)) {
        upload_get_random_tran_id(random_tran_id, sizeof(random_tran_id));
        tranId = (const char *) random_tran_id;
    }

    CJSON_ADD_NEW_STR_OBJ(upload, tranId);
    CJSON_ADD_NEW_INT_OBJ(upload, version);
    CJSON_ADD_NEW_INT_OBJ(upload, timestamp);
    CJSON_ADD_NEW_STR_OBJ(upload, topic);

    return RT_SUCCESS;
}

static int32_t upload_packet_payload(cJSON *upload, const char *event, int32_t status, const cJSON *content)
{
    int32_t ret = RT_ERROR;
    cJSON *payload_json = NULL;
    char *payload = NULL;

    payload_json = cJSON_CreateObject();
    if (!payload_json) {
        MSG_PRINTF(LOG_WARN, "The payload_json is error\n");
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(payload_json, event);
    CJSON_ADD_NEW_INT_OBJ(payload_json, status);
    CJSON_ADD_STR_OBJ(payload_json, content);

    payload = (char *)cJSON_PrintUnformatted(payload_json);
    if (payload != NULL) {
        CJSON_ADD_NEW_STR_OBJ(upload, payload);
    }

    ret = RT_SUCCESS;

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

static cJSON *upload_packet_all(const char *tran_id, const char *event, int32_t status,
                                        upload_topic_e topic, const cJSON *content)
{
    int32_t ret = RT_ERROR;
    cJSON *upload = NULL;

    upload = cJSON_CreateObject();
    if (!upload) {
        MSG_PRINTF(LOG_WARN, "The upload is error\n");
        goto exit_entry;
    }

    upload_packet_header_info(upload, tran_id, topic);
    upload_packet_payload(upload, event, status, content);

    ret = RT_SUCCESS;

    exit_entry:

    return !ret ? upload : NULL;
}

int32_t get_upload_event_result(const char *event, const char *tran_id, int32_t status, void *private_arg)
{
    const upload_event_t *obj = NULL;
    int32_t count = 1;

    for (obj = g_upload_event_START; obj <= g_upload_event_END; obj++) {
        if (!rt_os_strcmp(obj->event, event)) {
            char *upload_json_pag = NULL;
            cJSON *upload = NULL;
            cJSON *content = NULL;
            int32_t ret = RT_ERROR;

            MSG_PRINTF(LOG_INFO, "------->%s\n", event);

            content = obj->packer(private_arg);
            upload = upload_packet_all(tran_id, event, status, obj->topic, content);
            upload_json_pag = (char *)cJSON_PrintUnformatted(upload);
            if (upload_json_pag != NULL) {
                while (ret != RT_SUCCESS) {
                    ret = upload_send_request((const void *)upload_json_pag, rt_os_strlen(upload_json_pag));
                    if ((ret == RT_SUCCESS) || (ret == HTTP_RESPOND_ERROR)) {
                        break;
                    }
                    rt_os_sleep(3 * count);
                    count ++;
                }
            }

            if (upload) {
                cJSON_Delete(upload);
            }

            if (upload_json_pag) {
                cJSON_free(upload_json_pag);
            }

            return ret;
        }
    }

    MSG_PRINTF(LOG_WARN, "Unknow upload event [%s] !!!\r\n", event);
    return RT_ERROR;
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
            int32_t ret = RT_ERROR;

            MSG_PRINTF(LOG_INFO, "------->%s\n", event);

            content = obj->packer(private_arg);
            //MSG_PRINTF(LOG_WARN, "content [%p] tran_id: %s, status: %d !!!\r\n", content, tran_id, status);
            upload = upload_packet_all(tran_id, event, status, obj->topic, content);
            //MSG_PRINTF(LOG_WARN, "upload [%p] !!!\r\n", upload);
            upload_json_pag = (char *)cJSON_PrintUnformatted(upload);
            if (upload_json_pag != NULL) {
                //MSG_PRINTF(LOG_WARN, "upload_json_pag [%p] !!!\r\n", upload_json_pag);
                ret = upload_send_request((const void *)upload_json_pag, rt_os_strlen(upload_json_pag));
            }

            if (upload) {
                cJSON_Delete(upload);
            }

            if (upload_json_pag) {
                cJSON_free(upload_json_pag);
            }

            return ret;
        }
    }

    MSG_PRINTF(LOG_WARN, "Unknow upload event [%s] !!!\r\n", event);
    return RT_SUCCESS;
}

int32_t init_upload(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    g_upload_device_info    = (const devicde_info_t *)public_value_list->device_info;
    g_push_channel          = (const char *)public_value_list->push_channel;
    g_upload_eid            = (const char *)public_value_list->card_info->eid;
    g_upload_card_info      = (card_info_t *)public_value_list->card_info->info;
    g_upload_deviceid       = (const char *)g_upload_device_info->device_id;
    g_upload_addr           = (const char *)public_value_list->config_info->oti_addr;
    g_upload_port           = public_value_list->config_info->oti_port;
    g_upload_ver_info       = (const target_versions_t *)public_value_list->version_info;
    g_last_card_type        = (const profile_type_e *)&(public_value_list->card_info->last_type);
    g_upload_card_type      = (profile_type_e *)&(((public_value_list_t *)arg)->card_info->type);

    //MSG_PRINTF(LOG_INFO, "imei: %p, %s\n", g_upload_device_info->imei, g_upload_device_info->imei);
    //MSG_PRINTF(LOG_INFO, "eid : %p, %s\n", g_upload_eid, g_upload_eid);

    return RT_SUCCESS;
}

static int32_t upload_boot_info_event(void)
{
    static rt_bool g_report_boot_event = RT_FALSE;

    if (g_report_boot_event == RT_FALSE) {
        upload_event_report("BOOT", NULL, 0, NULL);
        g_report_boot_event = RT_TRUE;
    } else {
        rt_bool report_all_info = RT_FALSE;
        upload_event_report("INFO", NULL, 0, &report_all_info);
    }

    return RT_SUCCESS;
}

static void upload_boot_task(void)
{
    while (1) {
        if (g_upload_mqtt == RT_TRUE) {
            upload_boot_info_event();
            break;
        } else {
            rt_os_sleep(1);
        }
    }

    rt_exit_task(NULL);
}

static int32_t upload_boot_info_event_handle(void)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;

    ret = rt_create_task(&task_id, (void *)upload_boot_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }

    return ret;
}

int32_t upload_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    (void)buf;
    (void)len;

    if (MSG_NETWORK_CONNECTED == mode) {
        MSG_PRINTF(LOG_DBG, "upload module recv network connected\r\n");
        g_upload_network = RT_TRUE;
        upload_boot_info_event_handle();
    } else if (MSG_NETWORK_DISCONNECTED == mode) {
        MSG_PRINTF(LOG_DBG, "upload module recv network disconnected\r\n");
        g_upload_network = RT_FALSE;
        g_upload_mqtt = RT_FALSE;
    } else if (MSG_MQTT_CONNECTED == mode) {
        MSG_PRINTF(LOG_DBG, "upload module recv mqtt connected\r\n");
        g_upload_mqtt = RT_TRUE;
    } else if (MSG_MQTT_DISCONNECTED == mode) {
        MSG_PRINTF(LOG_DBG, "upload module recv mqtt disconnected\r\n");
        g_upload_mqtt = RT_FALSE;
    }

    return RT_SUCCESS;
}

