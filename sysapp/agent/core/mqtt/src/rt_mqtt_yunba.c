
#include "rt_type.h"
#include "rt_mqtt.h"

#define YUNBA_APPKEY                    "596882bf2a9275716fe3c1e2"  // APPKET of YUNBA server
#define YUNBA_SERVER_IP                 "23.91.101.68"              // the IP address of yunba server
#define YUNBA_URL_PORT                  "1883"                      // yunba mqtt port
#define YUNBA_URL                       "tcp://"YUNBA_SERVER_IP":"YUNBA_URL_PORT
#define YUNBA_SERVER_PORT               8085                        // the port of yunba server
#define YUNBA_SERVER_PORT2              8383
#define YUNBA_AUTHKEY_URL               "abj-redismsg-4.yunba.io"
#define YUNBA_AUTHKEY_PORT              8060

static int32_t retstatus;
static char auth_key[80];

static int32_t get_ret_status(const char *json_data)
{
    int ret = 0;
    cJSON *root = cJSON_Parse(json_data);
    
    if (root) {
        int ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 1) {
            retstatus =  cJSON_GetObjectItem(root, "status")->valueint;
        } else {
            ret = -1;
        }
        cJSON_Delete(root);
    }
    return ret;
}

static int32_t get_authkey_status(const char *json_data)
{
    int ret = 0;
    char buf[500];
    cJSON *root = NULL;

    memset(buf, 0, sizeof(buf));
    memset(auth_key, 0, sizeof(auth_key)),
    memcpy(buf, json_data, strlen(json_data));
    root = cJSON_Parse(buf);
    if (root) {
        int ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 1) {
            retstatus =  cJSON_GetObjectItem(root,"status")->valueint;
            if (retstatus == 0)
                strcpy(auth_key, cJSON_GetObjectItem(root,"authkey")->valuestring);
        } else {
            ret = -1;
        }
        cJSON_Delete(root);
    }
    return ret;
}

/* callback for YUNBA ticket server */
static int32_t mqtt_yunba_ticket_server_cb(const char *json_data) 
{
    int32_t ret = RT_ERROR;
    cJSON *root;
    mqtt_reg_info_t *reg_info  = mqtt_get_reg_info();

    root = cJSON_Parse(json_data);
    if (root) {
        int32_t ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 4) {
            cJSON * client_id   = cJSON_GetObjectItem(root, "c");
            cJSON * username    = cJSON_GetObjectItem(root, "u");
            cJSON * password    = cJSON_GetObjectItem(root, "p");
            cJSON * device_id   = cJSON_GetObjectItem(root, "d");
            
            if (client_id && username && password && device_id) {
                snprintf(reg_info->client_id, sizeof(reg_info->client_id), "%s", client_id->valuestring);
                snprintf(reg_info->username, sizeof(reg_info->username), "%s", username->valuestring);
                snprintf(reg_info->password, sizeof(reg_info->password), "%s", password->valuestring);
                snprintf(reg_info->device_id, sizeof(reg_info->device_id), "%s", device_id->valuestring);
                ret = RT_SUCCESS;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

/* get YUNBA MQTT connect param API */
int32_t MQTTClient_setup_with_appkey_and_deviceid(const char* appkey, const char *deviceid, mqtt_opts_t *opts)
{
    int32_t ret;
    char json_data[1024];
    mqtt_reg_url_t *reg_url    = mqtt_get_reg_url();
    mqtt_reg_info_t *reg_info  = mqtt_get_reg_info();

    if (!appkey) {
        ret = RT_ERROR;
        goto exit_entry;
    }

    if (!deviceid) {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\", \"p\":4}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\", \"p\":4, \"d\": \"%s\"}", appkey, deviceid);
    }
    
    ret = mqtt_http_post_json((const char *)json_data, reg_url->url, reg_url->port,
                "/device/reg/", mqtt_yunba_ticket_server_cb);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "http post json yunba error, ret=%d\r\n", ret);
        ret = RT_ERROR;
        goto exit_entry;
    }

    snprintf(opts->client_id, sizeof(opts->client_id), "%s", reg_info->client_id);
    snprintf(opts->username, sizeof(opts->username), "%s", reg_info->username);
    snprintf(opts->password, sizeof(opts->password), "%s", reg_info->password);
    snprintf(opts->device_id, sizeof(opts->device_id), "%s", reg_info->device_id);

    ret = RT_SUCCESS;
    
exit_entry:

    return ret;
}

int MQTTClient_set_authkey(char *cid, char *appkey, char* authkey, int *ret_status)
{
    int ret = RT_ERROR;
    char json_data[1024];

    snprintf(json_data, sizeof(json_data),
            "{\"cmd\":\"authkey_set\",\"cid\":\"%s\",\"appkey\":\"%s\",\"authkey\":\"%s\"}",
            cid, appkey, authkey);

    ret = mqtt_http_post_json((const char *)json_data, YUNBA_AUTHKEY_URL, YUNBA_AUTHKEY_PORT, "/", get_ret_status);
    if (ret < 0) {
        return RT_ERROR;
    }
    *ret_status = retstatus;
    return RT_SUCCESS;
}

int MQTTClient_get_authkey(char *cid, char *appkey, char* authkey, int *ret_status)
{
    int ret = RT_ERROR;
    char json_data[1024];

    snprintf(json_data, sizeof(json_data), "{\"cmd\":\"authkey_get\",\"cid\":\"%s\",\"appkey\":\"%s\"}", cid, appkey);

    ret = mqtt_http_post_json((const char *)json_data, YUNBA_AUTHKEY_URL, YUNBA_AUTHKEY_PORT, "/", (PCALLBACK)get_authkey_status);
    if (ret < 0) {
        return RT_ERROR;
    }
    *ret_status = retstatus;
    strcpy(authkey, auth_key);
    return RT_SUCCESS;
}

//connect yunba server
rt_bool mqtt_connect_yunba(mqtt_param_t *param, const char *ticket_server)
{
    uint8_t num = 0;
    int32_t ret;
    MQTTClient *c = &param->client;
    mqtt_opts_t *opts = &param->opts;
    const char *alias = param->alias;

    if (ticket_server == NULL) {
        mqtt_set_reg_url((const char *)YUNBA_SERVER_IP, YUNBA_SERVER_PORT2);
    } else {
        int8_t addr[20];
        int32_t port;
        if (mqtt_get_ip_pair(ticket_server, addr, &port) == RT_TRUE) {
            mqtt_set_reg_url((char *)addr, port);
        } else {
            MSG_PRINTF(LOG_WARN, "mqtt_get_ip_pair error ticket_serverL:%s\n", ticket_server);
            return RT_FALSE;
        }
        MSG_PRINTF(LOG_INFO, "ticket_server:%s, yunba addr:%s, port:%d\n", ticket_server, addr, port);
    }

    /* connect yunba mqtt server with max 3 times */
    do {
        if ((ret = MQTTClient_setup_with_appkey_and_deviceid(YUNBA_APPKEY, (char *)opts->device_id, opts)) == 0) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "MQTTClient setup_with_appkey_and deviceid num:%d, error ret:%d\n", num++, ret);
        rt_os_sleep(1);
    } while (num != MAX_CONNECT_SERVER_TIMER);

    if (num == MAX_CONNECT_SERVER_TIMER) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect yunba error\n");
        return RT_FALSE;
    }

    snprintf(opts->channel, sizeof(opts->channel), "%s", "YUNBA");
    snprintf(opts->url, sizeof(opts->url), "%s", YUNBA_URL);

    return RT_TRUE;
}

