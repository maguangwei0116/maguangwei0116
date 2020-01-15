/*
 * yunba_common.c
 *
 *  Created on: Nov 16, 2015
 *      Author: yunba
 */

#define _GNU_SOURCE /* for pthread_mutexattr_settype */
#include <stdlib.h>
#if !defined(WIN32) && !defined(WIN64)
    #include <sys/time.h>
#endif

#include "MQTTClient.h"
#if !defined(NO_PERSISTENCE)
#include "MQTTPersistence.h"
#endif

#include "utf-8.h"
#include "MQTTProtocol.h"
#include "MQTTProtocolOut.h"
#include "Thread.h"
#include "SocketBuffer.h"
#include "StackTrace.h"
#include "Heap.h"
#include "cJSON.h"
#include "rt_type.h"
#include "md5.h"
#include "rt_os.h"
#include "log.h"
#include "rt_mqtt_common.h"

#if defined(OPENSSL)
#include <openssl/ssl.h>
#endif

typedef int32_t (*YUNBA_CALLBACK)(char *p);

static mqtt_reg_info_t g_mqtt_reg_info;
static char g_mqtt_reg_url[40];
static int32_t g_mqtt_reg_port = 8383;
static char g_mqtt_url_host[200];
static char g_mqtt_url_port[8];

/* callback for EMQ ticket server */
static int32_t mqtt_emq_ticket_server_cb(const char *json_data) 
{
    int32_t ret = RT_ERROR;
    char *str = NULL;
    cJSON *root;

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
                snprintf(g_mqtt_reg_info.username, sizeof(g_mqtt_reg_info.username), "%s", username->valuestring);
                snprintf(g_mqtt_reg_info.password, sizeof(g_mqtt_reg_info.password), "%s", password->valuestring);
                ret = RT_SUCCESS;
            }
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

    root = cJSON_Parse(json_data);
    if (root) {
        int32_t ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 4) {
            cJSON * client_id   = cJSON_GetObjectItem(root, "c");
            cJSON * username    = cJSON_GetObjectItem(root, "u");
            cJSON * password    = cJSON_GetObjectItem(root, "p");
            cJSON * device_id   = cJSON_GetObjectItem(root, "d");
            
            if (client_id && username && password && device_id) {
                snprintf(g_mqtt_reg_info.client_id, sizeof(g_mqtt_reg_info.client_id), "%s", client_id->valuestring);
                snprintf(g_mqtt_reg_info.username, sizeof(g_mqtt_reg_info.username), "%s", username->valuestring);
                snprintf(g_mqtt_reg_info.password, sizeof(g_mqtt_reg_info.password), "%s", password->valuestring);
                snprintf(g_mqtt_reg_info.device_id, sizeof(g_mqtt_reg_info.device_id), "%s", device_id->valuestring);
                ret = 0;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

/* callback for REDTEA ticket server */
static int32_t mqtt_redtea_ticket_server_cb(const char *json_data) 
{
    int32_t ret = RT_ERROR;
    cJSON *data;
    cJSON *root;

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
                snprintf(g_mqtt_reg_info.username, sizeof(g_mqtt_reg_info.username), "%s", username->valuestring);
                snprintf(g_mqtt_reg_info.password, sizeof(g_mqtt_reg_info.password), password->valuestring);
                snprintf(g_mqtt_reg_info.channel, sizeof(g_mqtt_reg_info.channel), channel->valuestring);
                snprintf(g_mqtt_reg_info.ticket_server, sizeof(g_mqtt_reg_info.ticket_server), ticket_url->valuestring);
                snprintf(g_mqtt_reg_info.url, sizeof(g_mqtt_reg_info.url), "%s:%d", host->valuestring, port->valueint);
                ret = 0;
            }

            if(!rt_os_strncmp(g_mqtt_reg_info.channel, "YUNBA", 5) && client_id){
                snprintf(g_mqtt_reg_info.client_id, sizeof(g_mqtt_reg_info.client_id), client_id->valuestring);
            }
        }

        cJSON_Delete(data);
    }
    
    return ret;
}

/* get YUNBA MQTT connect param API */
int32_t MQTTClient_setup_with_appkey_and_deviceid(const char* appkey, const char *deviceid, mqtt_opts_t *opts)
{
    int32_t ret;
    char json_data[1024];

    if (!appkey) {
        ret = RT_ERROR;
        goto exit_entry;
    }

    if (!deviceid) {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\", \"p\":4}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\", \"p\":4, \"d\": \"%s\"}", appkey, deviceid);
    }
    
    ret = http_post_json((const char *)json_data, g_mqtt_reg_url, g_mqtt_reg_port, "/device/reg/", (PCALLBACK)mqtt_yunba_ticket_server_cb);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "http post json yunba error, %s:%d, ret=%d\r\n", g_mqtt_reg_url, g_mqtt_reg_port, ret);
        ret = RT_ERROR;
        goto exit_entry;
    }

    snprintf(opts->client_id, sizeof(opts->client_id), "%s", g_mqtt_reg_info.client_id);
    snprintf(opts->username, sizeof(opts->username), "%s", g_mqtt_reg_info.username);
    snprintf(opts->password, sizeof(opts->password), "%s", g_mqtt_reg_info.password);
    snprintf(opts->device_id, sizeof(opts->device_id), "%s", g_mqtt_reg_info.device_id);

    ret = RT_SUCCESS;
    
exit_entry:

    return ret;
}

/* get EMQ MQTT connect param API */
int32_t MQTTClient_setup_with_appkey(const char* appkey, mqtt_opts_t *opts)
{
    int32_t ret;
    char json_data[1024];

    if (!appkey) {
        return RT_ERROR;
    }

    snprintf(json_data, sizeof(json_data), "{\"appKey\":\"%s\"}", appkey);

    ret = http_post_json((const char *)json_data, g_mqtt_reg_url, g_mqtt_reg_port, \
                            "/clientService/getEmqUser", (PCALLBACK)mqtt_emq_ticket_server_cb);
    if (ret < 0) {
        return RT_ERROR;
    }

    snprintf(opts->client_id, sizeof(opts->client_id), "%s", g_mqtt_reg_info.client_id);
    snprintf(opts->username, sizeof(opts->username), "%s", g_mqtt_reg_info.username);
    snprintf(opts->password, sizeof(opts->password), "%s", g_mqtt_reg_info.password);
    
    return RT_SUCCESS;
}

/* get connect param with REDTEA adapter API */
int32_t mqtt_adapter_setup_with_appkey(const char *appkey, mqtt_opts_t *opts, const char *eid)
{
    char json_data[1024];
    int32_t ret;

    if (!appkey){
        MSG_PRINTF(LOG_ERR, "appkey is NULL\n");
        return RT_ERROR;
    }

    if (!opts->device_id) {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\"}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\",\"d\": \"%s\",\"c\":\"%s\",\"s\":\"%d\"}", \
            appkey, opts->device_id, eid, opts->last_connect_status);
    }

    MSG_PRINTF(LOG_INFO, "reg_url:%s, reg_port:%d\r\n", g_mqtt_reg_url, g_mqtt_reg_port);
    ret = http_post_json((const char *)json_data, g_mqtt_reg_url, g_mqtt_reg_port, \
                            "/api/v1/ticket", (PCALLBACK)mqtt_redtea_ticket_server_cb);
    if (ret < 0){
        MSG_PRINTF(LOG_ERR, "http post json error, ret=%d\r\n", ret);
        return RT_ERROR;
    }
    snprintf(opts->client_id, sizeof(opts->client_id), "%s", g_mqtt_reg_info.client_id);
    snprintf(opts->username, sizeof(opts->username), "%s", g_mqtt_reg_info.username);
    snprintf(opts->password, sizeof(opts->password), "%s", g_mqtt_reg_info.password);
    snprintf(opts->channel, sizeof(opts->channel), "%s", g_mqtt_reg_info.channel);
    snprintf(opts->ticket_server, sizeof(opts->ticket_server), "%s", g_mqtt_reg_info.ticket_server);
    snprintf(opts->url, sizeof(opts->url), "%s", g_mqtt_reg_info.url);

#if 0
    MSG_PRINTF(LOG_DBG, "client_id     : %s\r\n", info->client_id);
    MSG_PRINTF(LOG_DBG, "username      : %s\r\n", info->username);
    MSG_PRINTF(LOG_DBG, "password      : %s\r\n", info->password);
    MSG_PRINTF(LOG_DBG, "channel       : %s\r\n", info->channel);
    MSG_PRINTF(LOG_DBG, "ticket_server : %s\r\n", info->ticket_server);
    MSG_PRINTF(LOG_DBG, "url           : %s\r\n", info->url);
#endif
    
    return RT_SUCCESS;
}

static size_t mqtt_get_broker_cb(const char *json_data)
{
    int32_t ret = RT_ERROR;
    cJSON *root;
   
    MSG_PRINTF(LOG_INFO, "json_data: %s\n", json_data);
    root = cJSON_Parse(json_data);  
    
#if 0  // only for test
    {
        char *str = NULL;
        str = cJSON_Print(root);
        MSG_PRINTF(LOG_INFO, "%s\n",str);
        cJSON_free(str);
    }
#endif

    if (root) {
        cJSON *obj = cJSON_GetObjectItem(root, "obj");
        if(obj) {
            cJSON *url = cJSON_GetObjectItem(obj, "ip");
            cJSON *port = cJSON_GetObjectItem(obj, "port");
            if ((url != NULL) && (port != NULL)) {
                snprintf(g_mqtt_url_host, sizeof(g_mqtt_url_host), "%s", url->valuestring);
                snprintf(g_mqtt_url_port, sizeof(g_mqtt_url_port), "%s", port->valuestring);
                ret = 0;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

void mqtt_set_reg_url(const char url[20], int32_t port)
{
    snprintf(g_mqtt_reg_url, sizeof(g_mqtt_reg_url), "%s", url);
    g_mqtt_reg_port = port;
}

int32_t MQTTClient_get_host(const char *node_name, char *url, const char *appkey)
{
    int32_t ret = RT_ERROR;
    char json_data[1024];
    
    if(!node_name){
        snprintf(json_data, sizeof(json_data), "{\"appKey\":\"%s\"}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"nodeName\":\"%s\",\"appKey\":\"%s\"}", node_name, appkey);
    }

    MSG_PRINTF(LOG_INFO, "json_data: %s\n", json_data);
    ret = http_post_json((const char *)json_data, g_mqtt_reg_url, g_mqtt_reg_port, \
                            "/clientService/getNodes", (PCALLBACK)mqtt_get_broker_cb);
    if (ret < 0) {
        return RT_ERROR;
    }
    
    sprintf(url, "%s:%s", g_mqtt_url_host, g_mqtt_url_port);
    return RT_SUCCESS;
}

