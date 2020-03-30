
#include "rt_type.h"
#include "cJSON.h"
#include "rt_mqtt.h"

#define ADAPTER_APPKEY                  "D358134D21684E8FA23CC25740506A82"

/* callback for REDTEA ticket server */
static int32_t mqtt_redtea_ticket_server_cb(const char *json_data)
{
    int32_t ret = RT_ERROR;
    cJSON *data;
    cJSON *root;
    mqtt_reg_info_t *reg_info  = mqtt_get_reg_info();

    data = cJSON_Parse(json_data);
    if (!data) {
        MSG_PRINTF(LOG_ERR, "json data parse fail !\r\n"); 
        return ret;
    }
    
    root = cJSON_GetObjectItem(data, "data");
    if (root) {
        int32_t ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 4) {
            cJSON * channel     = cJSON_GetObjectItem(root, "s");
            cJSON * username    = cJSON_GetObjectItem(root, "u");
            cJSON * password    = cJSON_GetObjectItem(root, "p");
            cJSON * client_id   = cJSON_GetObjectItem(root, "c");
            cJSON * host        = cJSON_GetObjectItem(root, "h");
            cJSON * port        = cJSON_GetObjectItem(root, "o");
            cJSON * ticket_url  = cJSON_GetObjectItem(root, "r");
            
            if (username && password && channel && ticket_url && host && port ) {
                snprintf(reg_info->username, sizeof(reg_info->username), "%s", username->valuestring);
                snprintf(reg_info->password, sizeof(reg_info->password), password->valuestring);
                snprintf(reg_info->channel, sizeof(reg_info->channel), channel->valuestring);
                snprintf(reg_info->ticket_server, sizeof(reg_info->ticket_server), ticket_url->valuestring);
#if (CFG_EMQ_MQTTS_ENABLE)
                snprintf(reg_info->url, sizeof(reg_info->url), "ssl://%s:%d", host->valuestring, port->valueint);
#else
                snprintf(reg_info->url, sizeof(reg_info->url), "tcp://%s:%d", host->valuestring, port->valueint);
#endif
                ret = 0;
            }

            if(!rt_os_strncmp(reg_info->channel, "YUNBA", 5) && client_id){
                snprintf(reg_info->client_id, sizeof(reg_info->client_id), client_id->valuestring);
            }
        }

        cJSON_Delete(data);
    }
    
    return ret;
}

/* get connect param with REDTEA adapter API */
int32_t mqtt_adapter_setup_with_appkey(const char *appkey, mqtt_opts_t *opts, const char *eid)
{
    char json_data[1024];
    int32_t ret;
    mqtt_reg_url_t *reg_url    = mqtt_get_reg_url();
    mqtt_reg_info_t *reg_info  = mqtt_get_reg_info();

    if (!appkey){
        MSG_PRINTF(LOG_ERR, "appkey is NULL\n");
        return RT_ERROR;
    }

#if (CFG_EMQ_MQTTS_ENABLE)
    if (!opts->device_id) {
        snprintf(json_data, sizeof(json_data), "{\"a\":\"%s\",\"ssl_enabled\":true}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), \
            "{\"a\":\"%s\",\"d\":\"%s\",\"c\":\"%s\",\"s\":\"%d\",\"ssl_enabled\":true}", \
            appkey, opts->device_id, eid, opts->last_connect_status);
    }
#else
    if (!opts->device_id) {
        snprintf(json_data, sizeof(json_data), "{\"a\":\"%s\"}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"a\":\"%s\",\"d\":\"%s\",\"c\":\"%s\",\"s\":\"%d\"}", \
            appkey, opts->device_id, eid, opts->last_connect_status);
    }
#endif

    ret = mqtt_http_post_json((const char *)json_data, reg_url->url, reg_url->port, \
                            "/api/v1/ticket", mqtt_redtea_ticket_server_cb);
    if (ret < 0){
        MSG_PRINTF(LOG_ERR, "http post json error, ret=%d\r\n", ret);
        return RT_ERROR;
    }
    snprintf(opts->client_id, sizeof(opts->client_id), "%s", reg_info->client_id);
    snprintf(opts->username, sizeof(opts->username), "%s", reg_info->username);
    snprintf(opts->password, sizeof(opts->password), "%s", reg_info->password);
    snprintf(opts->channel, sizeof(opts->channel), "%s", reg_info->channel);
    snprintf(opts->ticket_server, sizeof(opts->ticket_server), "%s", reg_info->ticket_server);
    snprintf(opts->url, sizeof(opts->url), "%s", reg_info->url);

#if 0
    MSG_PRINTF(LOG_DBG, "client_id     : %s\r\n", reg_info->client_id);
    MSG_PRINTF(LOG_DBG, "username      : %s\r\n", reg_info->username);
    MSG_PRINTF(LOG_DBG, "password      : %s\r\n", reg_info->password);
    MSG_PRINTF(LOG_DBG, "channel       : %s\r\n", reg_info->channel);
    MSG_PRINTF(LOG_DBG, "ticket_server : %s\r\n", reg_info->ticket_server);
    MSG_PRINTF(LOG_DBG, "url           : %s\r\n", reg_info->url);
#endif
    
    return RT_SUCCESS;
}

rt_bool mqtt_connect_adapter(mqtt_param_t *param, const char *oti_addr, int32_t oti_port, const char *eid)
{
    uint8_t num = 0;
    MQTTClient *c = &param->client;
    mqtt_opts_t *opts = &param->opts;
    const char *alias = param->alias;

    mqtt_set_reg_url(oti_addr, oti_port);
    MSG_PRINTF(LOG_INFO, "OTI server addr:%s, port:%d\r\n", oti_addr, oti_port);

    /* connect redtea adpater server with max 3 times to get ticket server addr and port */
    do {
        if (mqtt_adapter_setup_with_appkey(ADAPTER_APPKEY, opts, eid) == 0) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "rt_mqtt_setup_with appkey num:%d\n", num++);
        rt_os_sleep(1);
    } while (num != MAX_CONNECT_SERVER_TIMER);

    if (num == MAX_CONNECT_SERVER_TIMER) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect adapter error\n");
        return RT_FALSE;
    }

    if (!rt_os_strncmp(opts->channel, "EMQ", 3)) {
        MSG_PRINTF(LOG_INFO, "mqtt alias: %s\r\n", alias);
        snprintf(opts->client_id, sizeof(opts->client_id), "%s", alias);
    }

    /* save ticket server into cache file */
    mqtt_save_ticket_server((const mqtt_opts_t *)opts);

    return RT_TRUE;
}


