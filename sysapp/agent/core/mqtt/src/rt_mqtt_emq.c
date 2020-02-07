#include "rt_type.h"
#include "cJSON.h"
#include "rt_mqtt.h"

#define EMQ_APPKEY                      "12345" // APPKET of EMQ server
#define EMQ_SERVER_PORT                 8085    // the port of EMQ server

/* callback for EMQ ticket server */
static int32_t mqtt_emq_ticket_server_cb(const char *json_data) 
{
    int32_t ret = RT_ERROR;
    char *str = NULL;
    cJSON *root;
    mqtt_reg_info_t *reg_info  = mqtt_get_reg_info();

    MSG_PRINTF(LOG_INFO, "buf:%s\n", json_data);    
    root = cJSON_Parse(json_data);
    if(!root){
        return RT_ERROR;
    }
    
    str = cJSON_Print(root);
    if (str) {
        MSG_PRINTF(LOG_INFO, "%s\n", str);
        cJSON_free(str);
    }
    
    if (root) {
        cJSON *obj = cJSON_GetObjectItem(root, "obj");
        if(obj){
            cJSON * username = cJSON_GetObjectItem(obj, "username");
            cJSON * password = cJSON_GetObjectItem(obj, "password");
            
            if(username && password){
                snprintf(reg_info->username, sizeof(reg_info->username), "%s", username->valuestring);
                snprintf(reg_info->password, sizeof(reg_info->password), "%s", password->valuestring);
                ret = RT_SUCCESS;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

/* get EMQ MQTT connect param API */
int32_t MQTTClient_setup_with_appkey(const char* appkey, mqtt_opts_t *opts)
{
    int32_t ret;
    char json_data[1024];
    mqtt_reg_url_t *reg_url    = mqtt_get_reg_url();
    mqtt_reg_info_t *reg_info  = mqtt_get_reg_info();

    if (!appkey) {
        return RT_ERROR;
    }

    snprintf(json_data, sizeof(json_data), "{\"appKey\":\"%s\"}", appkey);

    ret = mqtt_http_post_json((const char *)json_data, reg_url->url, reg_url->port, \
                            "/clientService/getEmqUser", mqtt_emq_ticket_server_cb);
    if (ret < 0) {
        return RT_ERROR;
    }

    snprintf(opts->client_id, sizeof(opts->client_id), "%s", reg_info->client_id);
    snprintf(opts->username, sizeof(opts->username), "%s", reg_info->username);
    snprintf(opts->password, sizeof(opts->password), "%s", reg_info->password);
    
    return RT_SUCCESS;
}

//connect EMQ server
rt_bool mqtt_connect_emq(mqtt_param_t *param, const char *emq_addr, const char *ticket_server)
{
    uint8_t num = 0;
    MQTTClient *c = &param->client;
    mqtt_opts_t *opts = &param->opts;
    const char *alias = param->alias;

    if (ticket_server == NULL) {
        mqtt_set_reg_url(emq_addr, EMQ_SERVER_PORT);
    } else {
        int8_t addr[20];
        int32_t port;
        if (mqtt_get_ip_pair(ticket_server, addr, &port) == RT_TRUE) {
            mqtt_set_reg_url((char *)addr, port);
        } else {
            MSG_PRINTF(LOG_WARN, "mqtt_get_ip_pair error ticket_serverL:%s\n", ticket_server);
            return RT_FALSE;
        }
        MSG_PRINTF(LOG_WARN, "ticket_server:%s, EMQ addr:%s, port:%d\n", ticket_server, addr, port);
    }

    /* connect EMQ mqtt server with max 3 times */
    do {
        if ((MQTTClient_setup_with_appkey(EMQ_APPKEY, opts) == 0) &&
           (MQTTClient_get_host((const char *)opts->node_name, (char *)opts->url, EMQ_APPKEY) == 0)) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "MQTTClient_setup_with appkey num:%d\n", num++);
        rt_os_sleep(1);
    } while (num != MAX_CONNECT_SERVER_TIMER);

    if (num == MAX_CONNECT_SERVER_TIMER) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect _emq error\n");
        return RT_FALSE;
    }

    snprintf(opts->channel, sizeof(opts->channel), "%s", "EMQ");
    snprintf(opts->client_id, sizeof(opts->client_id), "%s", alias);

    return RT_TRUE;
}

