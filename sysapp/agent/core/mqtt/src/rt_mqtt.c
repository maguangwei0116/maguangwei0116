
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "rt_type.h"
#include "config.h"
#include "cJSON.h"
#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
#include "downstream.h"
#include "personalise.h"

#define ADAPTER_APPKEY                  "D358134D21684E8FA23CC25740506A82"
#define ADAPTER_PORT                    7082

#define YUNBA_APPKEY                    "596882bf2a9275716fe3c1e2"  // APPKET of YUNBA server
#define YUNBA_SERVER_IP                 "23.91.101.68"  // the IP address of yunba server
#define YUNBA_URL                       "tcp://23.91.101.68:1883"
#define YUNBA_SERVER_PORT               8085  // the port of yunba server
#define YUNBA_SERVER_PORT2              8383

#define EMQ_APPKEY                      "12345"  // APPKET of EMQ server
#define EMQ_SERVER_PORT                 8085  // the port of EMQ server

#define MQTT_CONNECT_YUNBA_ERROR        -1
#define MQTT_CONNECT_EMQ_ERROR          1
#define MQTT_CONNECT_SUCCESS            0

#define TICKET_SERVER_CACHE             "rt_ticket_server"
#define MAX_CONNECT_SERVER_TIMER        3

#define USE_ADAPTER_SERVER              1  // use mqtt ticket adapter proxy server ?

#define AGENT_ALIAS                     "agent"

#define MQTT_PUBLISH_TOPIC              "client_report"

#define MQTT_PUBLISH_TIMEOUT            10000L

#define MQTT_SUBCRIBE_TOPIC_MAX_CNT     10

#define MQTT_KEEP_ALIVE_INTERVAL        300

#define MQTT_RECONNECT_MAX_CNT          5

#define MQTT_SUBSCRIBE_MAX_CNT          3

#define MQTT_ALIAS_MAX_LEN              40

#define MQTT_NETWORK_STATE_TIMEOUT      60  // seconds

#define GET_EID_FLAG(flag)              (((flag) >> 0) & 0x01)
#define GET_AGENT_FLAG(flag)            (((flag) >> 1) & 0x01)
#define GET_DEVICE_ID_FLAG(flag)        (((flag) >> 2) & 0x01)
#define GET_IMEI_FLAG(flag)             (((flag) >> 3) & 0x01)
#define SET_EID_FLAG(flag)              ((flag) |= (0x01 << 0))
#define SET_AGENT_FLAG(flag)            ((flag) |= (0x01 << 1))
#define SET_DEVICE_ID_FLAG(flag)        ((flag) |= (0x01 << 2))
#define SET_IMEI_FLAG(flag)             ((flag) |= (0x01 << 3))
#define CLR_EID_FLAG(flag)              ((flag) &= ~(0x01 << 0))
#define CLR_AGENT_FLAG(flag)            ((flag) &= ~(0x01 << 1))
#define CLR_DEVICE_ID_FLAG(flag)        ((flag) &= ~(0x01 << 2))
#define CLR_IMEI_FLAG(flag)             ((flag) &= ~(0x01 << 3))
#define CLR_SUBSCRIBE_FLAG(flag)        ((flag) = 0x00)

typedef enum MQTT_QOS {
    MQTT_QOS_0 = 0, 
    MQTT_QOS_1 = 1,
    MQTT_QOS_2 = 2,
} mqtt_qos_e;

typedef enum NETWORK_STATE {
    NETWORK_IDLE = 0,           // network idle state
    NETWORK_CONNECTED,          // network is connected
    NETWORK_DISCONNECTED,       // network is disconnected
} network_state_info_e;

typedef enum MQTT_STATE {
    MQTT_IDLE = 0,              // mqtt idle state, it's a instable state !
    MQTT_CHECK_NETWORK,         // mqtt client check network connected or disconnected, it's a instable state !
    MQTT_GET_SERVER_ADDR,       // mqtt client get mqtt server addr, it's a instable state !
    MQTT_CONNECTING,            // mqtt client connectiong broker server, it's a instable state !
    MQTT_CONNECTED,             // mqtt client connecte broker server ok, it's a instable state !
    MQTT_SET_ALIAS,             // mqtt client set alias, it's a instable state !
    MQTT_SUBSCRIBED,            // mqtt client can subscribe topic, it's a instable state !
    MQTT_WAIT_EVENT,            // mqtt client wait event (network connected/disconnected/subscribe eid ...), it's a stable state !
    MQTT_FAST_RECONNECT,        // mqtt client fast reconnect when the last connect lost, it's a stable state !
    MQTT_DISCONNECTED,          // mqtt client disconnected, it's a instable state !
} mqtt_state_info_e;

typedef enum MQTT_EVENT {
    MQTT_IDLE_EVENT = 0,        // mqtt idle
    MQTT_NETWORK_CONNECTED,     // mqtt network connected
    MQTT_NETWORK_DISCONNECTED,  // mqtt network disconnected
    MQTT_CONNECTED_LOST,        // mqtt connected lost
    MQTT_SUBSCRIBE_EID,         // mqtt subscribe topic (eid)
} mqtt_event_e;

typedef struct MQTT_PARAM {
    MQTTClient_connectOptions   conn_opts;
    mqtt_opts_t                 opts;
    MQTTClient                  client;
    network_state_info_e        network_state;
    mqtt_state_info_e           mqtt_state;
    char                        alias[MQTT_ALIAS_MAX_LEN];
    rt_bool                     mqtt_get_addr;
    rt_bool                     mqtt_conn_state;    // mqtt connect state flag
    rt_bool                     mqtt_flag;          // mqtt connect ok flag
    rt_bool                     lost_flag;          // mqtt connect lost flag
    rt_bool                     alias_rc;           // need set alias flag
    rt_bool                     subscribe_eid;      // need to subscribe eid
    uint8_t                     subscribe_flag;     // subscribe ok flag
} mqtt_param_t;

typedef struct MQTT_SUBCRIBE_INFO {
    int32_t                     cnt;
    const char *                topic[MQTT_SUBCRIBE_TOPIC_MAX_CNT];
} mqtt_subcribe_info_t;

typedef struct MQTT_INFO {
    const char *                eid;
    const char *                imei;
    const char *                device_id;
    const char *                emq_addr;
    const char *                oti_addr;
    mqtt_subcribe_info_t        sub_info;
} mqtt_info_t;

static mqtt_param_t g_mqtt_param = {MQTTClient_connectOptions_initializer, 0};
static mqtt_info_t g_mqtt_info;

static rt_bool mqtt_eid_check_memory(const void *buf, int32_t len, int32_t value)
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

/* save cache ticket server which got from adapter into cache file */
static rt_bool mqtt_save_ticket_server(const mqtt_opts_t *opts)
{
    cJSON   *obj = NULL;
    uint8_t  *save_info = NULL;
    uint8_t  data_len[2] = {0}; // frist 2 byte store length !!!
    int16_t length = 0;
    rt_bool ret = RT_FALSE;

    if ((obj = cJSON_CreateObject()) == NULL) {
        MSG_PRINTF(LOG_WARN, "cJSON_CreateObject error\n");
        goto exit_entry;
    }

    cJSON_AddItemToObject(obj, "channel", cJSON_CreateString(opts->channel));
    cJSON_AddItemToObject(obj, "ticketServer", cJSON_CreateString(opts->ticket_server));
    save_info = (uint8_t *)cJSON_PrintUnformatted(obj);
    if (!save_info) {
        MSG_PRINTF(LOG_WARN, "save_info is NULL\n");
        goto exit_entry;
    }

    length = rt_os_strlen(save_info);
    data_len[0] = (length >> 8) & 0xff;
    data_len[1] = length & 0xff;

    if (rt_create_file(TICKET_SERVER_CACHE) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt create_file  error\n");
        goto exit_entry;
    }

    if (rt_write_data(TICKET_SERVER_CACHE, 0, data_len, sizeof(data_len)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt write_data data_len error\n");
        goto exit_entry;
    }

    if (rt_write_data(TICKET_SERVER_CACHE, sizeof(data_len), save_info, rt_os_strlen(save_info)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt write_data TICKET_SERVER_CACHE error\n");
        goto exit_entry;
    }

    ret = RT_TRUE;

exit_entry:

    if (save_info) {
        cJSON_free(save_info);
    }
    if (obj) {
        cJSON_Delete(obj);
    }

    return ret;
}

/* get cache ticket server which got from adapter from cache file */
static rt_bool mqtt_get_ticket_server(mqtt_opts_t *opts)
{
    uint8_t  *save_info = NULL;
    uint8_t  data_len[2] = {0};  // frist 2 byte store length !!!
    int16_t length = 0;
    cJSON   *obj = NULL;
    cJSON   *channel = NULL;
    cJSON   *ticket_server = NULL;
    rt_bool ret = RT_FALSE;

    if (rt_read_data(TICKET_SERVER_CACHE, 0, data_len, sizeof(data_len)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt read_data data_len error\n");
        goto exit_entry;
    }

    length = (data_len[0] << 8) | data_len[1];

    if ((save_info = (uint8_t *)rt_os_malloc(length)) == NULL) {
        MSG_PRINTF(LOG_ERR, "rt_os_malloc error\n");
        goto exit_entry;
    }

    if (rt_read_data(TICKET_SERVER_CACHE, sizeof(data_len), save_info, length) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt read_data save_info error\n");
        goto exit_entry;
    }

    if ((obj = cJSON_Parse(save_info)) == NULL) {
        MSG_PRINTF(LOG_WARN, "cJSON_Parse error\n");
        goto exit_entry;
    }
    channel = cJSON_GetObjectItem(obj, "channel");
    ticket_server = cJSON_GetObjectItem(obj, "ticketServer");

    if (!channel || !ticket_server) {
        MSG_PRINTF(LOG_WARN, "channel or ticket_server is NULL\n");
        goto exit_entry;
    }

    snprintf((char *)opts->channel, sizeof(opts->channel), "%s", channel->valuestring);
    snprintf((char *)opts->ticket_server, sizeof(opts->ticket_server), "%s", ticket_server->valuestring);

    ret = RT_TRUE;

exit_entry:

    if (save_info) {
        rt_os_free(save_info);
    }
    if (obj) {
        cJSON_Delete(obj);
    }

    return ret;
}

static rt_bool mqtt_get_ip_pair(const char *url, int8_t *addr, int32_t *port)
{
    char *p = (int8_t *)strstr((char *)url, "http://");
    if (p) {
        p += 7;
        char *q = (int8_t *)strstr((char *)p, ":");
        if (q) {
            int32_t len = rt_os_strlen(p) - rt_os_strlen(q);
            if (len > 0) {
                sprintf((char *)addr, "%.*s", len, p);
                *port = atoi((char *)(q + 1));
                return RT_TRUE;
            }
        }
    }
    return RT_FALSE;
}

static rt_bool mqtt_connect_adapter(mqtt_param_t *param)
{
    uint8_t num = 0;
    MQTTClient *c = &param->client;
    mqtt_opts_t *opts = &param->opts;
    const char *alias = param->alias;
    const char *eid = "";  // OTI server required: Request with [eid=""] when eid isn't exist !!!

    if (!mqtt_eid_check_memory(g_mqtt_info.eid, MAX_EID_LEN, 'F') && !mqtt_eid_check_memory(g_mqtt_info.eid, MAX_EID_LEN, '0')) {
        eid = g_mqtt_info.eid;
    }

    mqtt_set_reg_url(g_mqtt_info.oti_addr, ADAPTER_PORT);
    MSG_PRINTF(LOG_INFO, "OTI server addr:%s, port:%d\r\n", g_mqtt_info.oti_addr, ADAPTER_PORT);

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

//connect yunba server
static rt_bool mqtt_connect_yunba(mqtt_param_t *param, const char *ticket_server)
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
        MSG_PRINTF(LOG_WARN, "ticket_server:%s, yunba addr:%s, port:%d\n", ticket_server, addr, port);
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

//connect EMQ server
static rt_bool mqtt_connect_emq(mqtt_param_t *param, const char *ticket_server)
{
    uint8_t num = 0;
    MQTTClient *c = &param->client;
    mqtt_opts_t *opts = &param->opts;
    const char *alias = param->alias;

    if (ticket_server == NULL) {
        mqtt_set_reg_url(g_mqtt_info.emq_addr, EMQ_SERVER_PORT);
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

#if 1
#define LABEL_FORCE_TO_ADAPTER          1
#define LABEL_FORCE_TO_EMQ              0
#define LABEL_FORCE_TO_YUNBA            0

#define MQTT_PASSAGEWAY_DEFINITION(x)\
do {\
    if (LABEL_##x) {\
        MSG_PRINTF(LOG_DBG, "MQTT %s ...\r\n", #x);\
        goto x;\
    }\
} while(0)
#else
#define MQTT_PASSAGEWAY_DEFINITION(x)   do {} while(0)
#endif

//used to get mqtt server addr
static rt_bool mqtt_get_server_addr(mqtt_param_t *param)
{
    //attemp to connect adapter
    do{
        MQTT_PASSAGEWAY_DEFINITION(FORCE_TO_ADAPTER);
        MQTT_PASSAGEWAY_DEFINITION(FORCE_TO_EMQ);
        MQTT_PASSAGEWAY_DEFINITION(FORCE_TO_YUNBA);

FORCE_TO_ADAPTER:
        if (USE_ADAPTER_SERVER){
            if (mqtt_connect_adapter(param) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "connect adapter server to get mqtt server address EMQ or YUNBA successfully\n");
                goto ok_exit_entry;
            }

            if (mqtt_get_ticket_server(&param->opts) == RT_TRUE) {
                /* If connect yunba ticket server before, and then try this */
                if (!rt_os_strncmp(param->opts.channel, "YUNBA", 5) &&
                      (mqtt_connect_yunba(param, param->opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "get YUNBA mqtt server connect param successfully\n");
                    goto ok_exit_entry;
                }

                /* If connect EMQ ticket server before, and then try this */
                if (!rt_os_strncmp(param->opts.channel, "EMQ", 3) &&
                      (mqtt_connect_emq(param, param->opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "get EMQ mqtt server connect param successfully\n");
                    goto ok_exit_entry;
                }
            }
        }

        /* If connect adapter and ticket server all fail, and then try dead yunba server or EMQ server */
        if (!rt_os_strncmp(param->opts.channel, "YUNBA", 5)) {
FORCE_TO_EMQ:
            if (mqtt_connect_emq(param, NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "get EMQ mqtt server connect param successfully\n");
                goto ok_exit_entry;
            }
        } else if (!rt_os_strncmp(param->opts.channel, "EMQ", 3)) {
FORCE_TO_YUNBA:
            if (mqtt_connect_yunba(param, NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "get yunba mqtt server connect param successfully\n");
                goto ok_exit_entry;
            }
        }
    }while(0);

fail_exit_entry:
    
    MSG_PRINTF(LOG_WARN, "connet adapter/EMQ server/YUNBA server all fail !\n");
    return RT_FALSE;

ok_exit_entry:
    
    g_mqtt_param.mqtt_get_addr = RT_TRUE;
    return RT_TRUE;
}

static int32_t mqtt_subcribe(MQTTClient handle, const char* topic, int32_t qos)
{
    return MQTTClient_subscribe(handle, topic, qos);
}

static int32_t mqtt_subcribe_many_with_default_qos(MQTTClient handle, int32_t count, const char** topic)
{
    return MQTTClient_subscribe_many(handle, count, topic);
}

static int32_t mqtt_client_pulish_msg(MQTTClient handle, int32_t qos, const char* topic, const void* data, int32_t data_len)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token = 0;
    int32_t rc1;
    int32_t rc2;
    
    pubmsg.payload      = (void *)data;
    pubmsg.payloadlen   = data_len;
    pubmsg.qos          = qos;
    pubmsg.retained     = 0;

    #if 1
    do {
        rc1 = MQTTClient_publishMessage(handle, topic, &pubmsg, &token);
        MSG_PRINTF(LOG_INFO, "Waiting for up to %dS to publish (%d bytes): \r\n%s\r\n", 
                (int32_t)(MQTT_PUBLISH_TIMEOUT/1000), data_len, (const char *)data);
        MSG_PRINTF(LOG_INFO, "on topic [%s] ClientID: [%s] rc1=%d, token=%lld\r\n", topic, g_mqtt_info.device_id, rc1, token);
        rc2 = MQTTClient_waitForCompletion(handle, token, MQTT_PUBLISH_TIMEOUT);
        MSG_PRINTF(LOG_INFO, "Message with delivery token %lld delivered, rc2=%d\n", token, rc2);
        if (rc1 != 0 || rc2 != 0) {
            MSG_PRINTF(LOG_WARN, "MQTT publish msg fail !\r\n");
            rc1 = RT_ERROR;
            goto exit_entry;
        }
        MSG_PRINTF(LOG_INFO, "MQTT publish msg ok !\r\n");
    } while(0);
    #else    
    deliveredtoken = 0;
    rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MSG_PRINTF(LOG_INFO, "Waiting for up to %d seconds for publication of %s\n"
                "on topic %s for client with ClientID: %s, rc=%d\n",
                (int)(TIMEOUT/1000), (const char *)data, topic, (const char *)client_id, rc);
    while(deliveredtoken != token);
    #endif

    rc1 = RT_SUCCESS;

exit_entry:

    return rc1;
}

static int32_t mqtt_pulish(const char* topic, const void* data, int32_t data_len)
{
    int32_t ret;

    /* never use yunba to publish message */
    if (!rt_os_strncmp(g_mqtt_param.opts.channel, "YUNBA", 5)) {
        return MQTT_PUBLISH_NO_YUNBA;
    }

    /* check mqtt connected state */
    if (g_mqtt_param.mqtt_flag != RT_TRUE) {
        return MQTT_PUBLISH_NO_CONNECTED;
    }

    ret = mqtt_client_pulish_msg(g_mqtt_param.client, g_mqtt_param.opts.qos, topic, data, data_len);
    //MSG_PRINTF(LOG_WARN, "mqtt pulish (%d bytes): %s, ret=%d\r\n", data_len, (const char*)data, ret);

    return ret;
}

int32_t mqtt_pulish_msg(const void* data, int32_t data_len)
{
    return mqtt_pulish(MQTT_PUBLISH_TOPIC, data, data_len);
}

static int32_t mqtt_subscribe_all(void)
{
    int32_t ret;
    int32_t offset = 0;
    int32_t cnt = 0;
    rt_bool need_subcribe = RT_FALSE;
    char topic_list[256] = {0};

    g_mqtt_info.sub_info.cnt = 0;  // clear counter
    
    /* subscribe [cid/eid] */
    if ((GET_EID_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE) && \
        (rt_os_strlen(g_mqtt_param.alias)) && \
        (rt_os_strcmp((const char *)g_mqtt_param.alias, g_mqtt_info.device_id))) {
        g_mqtt_info.sub_info.topic[g_mqtt_info.sub_info.cnt] = (const char *)g_mqtt_param.alias;  
        offset = offset + cnt;
        cnt = snprintf(&topic_list[offset], sizeof(topic_list) - offset, "[%s] ", g_mqtt_info.sub_info.topic[g_mqtt_info.sub_info.cnt]);
        g_mqtt_info.sub_info.cnt++;
        need_subcribe = RT_TRUE;
    }

    /* subscribe device-id */
    if ((GET_DEVICE_ID_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
        g_mqtt_info.sub_info.topic[g_mqtt_info.sub_info.cnt] = (const char *)g_mqtt_info.device_id;
        offset = offset + cnt;
        cnt = snprintf(&topic_list[offset], sizeof(topic_list) - offset, "[%s] ", g_mqtt_info.sub_info.topic[g_mqtt_info.sub_info.cnt]);
        g_mqtt_info.sub_info.cnt++;
        need_subcribe = RT_TRUE;
    }

    /* subscribe imei */
    if ((GET_IMEI_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
        g_mqtt_info.sub_info.topic[g_mqtt_info.sub_info.cnt] = (const char *)g_mqtt_info.imei;
        offset = offset + cnt;
        cnt = snprintf(&topic_list[offset], sizeof(topic_list) - offset, "[%s] ", g_mqtt_info.sub_info.topic[g_mqtt_info.sub_info.cnt]);
        g_mqtt_info.sub_info.cnt++; 
        need_subcribe = RT_TRUE;
    }

    /* subscribe agent  */
    if ((GET_AGENT_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
        g_mqtt_info.sub_info.topic[g_mqtt_info.sub_info.cnt] = (const char *)AGENT_ALIAS;
        offset = offset + cnt;
        cnt = snprintf(&topic_list[offset], sizeof(topic_list) - offset, "[%s] ", g_mqtt_info.sub_info.topic[g_mqtt_info.sub_info.cnt]);
        g_mqtt_info.sub_info.cnt++;
        need_subcribe = RT_TRUE;
    }

    if (!need_subcribe) {
        return RT_SUCCESS;
    }

    ret = mqtt_subcribe_many_with_default_qos(g_mqtt_param.client, g_mqtt_info.sub_info.cnt, g_mqtt_info.sub_info.topic);
    if (ret == RT_SUCCESS) {
        SET_EID_FLAG(g_mqtt_param.subscribe_flag);
        SET_DEVICE_ID_FLAG(g_mqtt_param.subscribe_flag);
        SET_IMEI_FLAG(g_mqtt_param.subscribe_flag);
        SET_AGENT_FLAG(g_mqtt_param.subscribe_flag);
        MSG_PRINTF(LOG_WARN, "mqtt subcribe %s ok !\r\n", topic_list);
    } else {
        CLR_EID_FLAG(g_mqtt_param.subscribe_flag);
        CLR_DEVICE_ID_FLAG(g_mqtt_param.subscribe_flag);
        CLR_IMEI_FLAG(g_mqtt_param.subscribe_flag);
        CLR_AGENT_FLAG(g_mqtt_param.subscribe_flag);
        g_mqtt_info.sub_info.cnt = 0;  // clear counter
        MSG_PRINTF(LOG_WARN, "mqtt subcribe %s fail, ret=%d !\r\n", topic_list, ret);
        if (ret == MQTTCLIENT_DISCONNECTED) {
            MSG_PRINTF(LOG_WARN, "mqtt disconnected now !\r\n");
        } else {
            ret = RT_ERROR;
        }
    }

    return ret;
}

static rt_bool mqtt_check_topic(const char *topic)
{
    int32_t i;

#if 0
    MSG_PRINTF(LOG_INFO, "g_mqtt_info.sub_info.cnt=%d\r\n", g_mqtt_info.sub_info.cnt);
    for (i = 0; i < g_mqtt_info.sub_info.cnt; i++) {
        MSG_PRINTF(LOG_INFO, "%d: %s\r\n", i, g_mqtt_info.sub_info.topic[i]);
    }
#endif

    /* never deal with EID topic */
    if (!rt_os_strcmp(g_mqtt_info.eid, topic)) {
        return RT_TRUE;
    }

    for (i = 0; i < g_mqtt_info.sub_info.cnt; i++) {
        if (!rt_os_strcmp(g_mqtt_info.sub_info.topic[i], topic)) {
            return RT_TRUE;
        }
    }
    return RT_FALSE;
}

//Registe callback of message arrive
static int32_t mqtt_msg_arrived(void* context, char* topic_name, int32_t topic_len, MQTTClient_message* md)
{
    char *msg = (char *)md->payload;
    int32_t len = md->payloadlen;
    char *msg_buf = NULL;

    if (len > 0) {   
        msg_buf = (char *)rt_os_malloc(len + 1);
        if (msg_buf) {
            rt_os_memcpy(msg_buf, msg, len);
            msg_buf[len] = '\0';
            MSG_PRINTF(LOG_INFO, "msg arrived, topic:%s, len: %d bytes: \r\n%s\r\n", topic_name, len, msg_buf);
            
            if (!mqtt_check_topic(topic_name)) {
                MSG_PRINTF(LOG_WARN, "mqtt unexpected topic [%s] arrived, ignore !\r\n", topic_name);
            } else {
                downstream_msg_handle((const char *)msg_buf, len);
            }
            rt_os_free(msg_buf);
        }
    }
    
    MQTTClient_freeMessage(&md);
    MQTTClient_free(topic_name);

    return 1;
}

//Registe callback of extra message arrive
static int32_t mqtt_ext_msg_arrived(void *context, EXTED_CMD cmd, int32_t status, int32_t ret_string_len, int8_t *ret_string)
{
    MSG_PRINTF(LOG_INFO, "%s, cmd:%d, status:%d, payload: %.*s\n", __func__, cmd, status, ret_string_len, ret_string);
    return 0;
}

static rt_bool mqtt_connect(MQTTClient* client, MQTTClient_connectOptions* opts)
{
    int32_t c = 0;

    //MSG_PRINTF(LOG_WARN, "Connect mqtt broker [%s] !\n", opts->serverURIs);
    if ((c = MQTTClient_connect(*client, opts)) == 0) {
        g_mqtt_param.mqtt_conn_state    = RT_TRUE;
        g_mqtt_param.mqtt_flag          = RT_TRUE;
        g_mqtt_param.lost_flag          = RT_FALSE;
        MSG_PRINTF(LOG_DBG, "Connect mqtt ok !\r\n");
        return RT_TRUE;
    } else {
        if (!rt_os_strncmp(g_mqtt_param.opts.channel, "EMQ", 3)) {
            g_mqtt_param.opts.last_connect_status = MQTT_CONNECT_EMQ_ERROR;
        } else {
            g_mqtt_param.opts.last_connect_status = MQTT_CONNECT_YUNBA_ERROR;
        }
        g_mqtt_param.mqtt_conn_state    = RT_FALSE;
        MSG_PRINTF(LOG_WARN, "Connect mqtt fail, error:%d\r\n", c);
        return RT_FALSE;
    }
}

static rt_bool mqtt_disconnect(MQTTClient* client, int32_t *wait_cnt)
{
    MSG_PRINTF(LOG_DBG, "MQTTClient disconnect\n");
    MQTTClient_disconnect(*client, 0);                
    g_mqtt_param.mqtt_flag      = RT_FALSE;
    g_mqtt_param.mqtt_conn_state= RT_FALSE;
    g_mqtt_param.lost_flag      = RT_FALSE;
    g_mqtt_param.subscribe_flag = 0;  // reset subscribe flag
    if (wait_cnt) {
        *wait_cnt               = 0;  // reset wait counter
    }
    msg_send_agent_queue(MSG_ID_MQTT, MSG_MQTT_DISCONNECTED, NULL, 0); 
    MSG_PRINTF(LOG_INFO, "MQTTClient disconnect msg throw out !\n");

    return RT_TRUE;
}

static void mqtt_connection_lost(void *context, char *cause)
{
    MSG_PRINTF(LOG_WARN, "connection lost: %s, %s\r\n",(char *)context, cause);
    g_mqtt_param.lost_flag = RT_TRUE;
}

static rt_bool mqtt_eid_check_upload(void)
{
    if (mqtt_eid_check_memory(g_mqtt_info.eid, MAX_EID_LEN, '0')) {
        personalise_upload_no_cert(NULL);
        return RT_TRUE;
    }  

    return RT_FALSE;
}

//connect mqtt server
static rt_bool mqtt_connect_server(mqtt_param_t *param)
{
    int32_t ret = 0;
    MQTTClient *c = &param->client;    
    MQTTClient_connectOptions *pconn_opts = &param->conn_opts;
    mqtt_opts_t *opts = &param->opts;
    static char last_channel[16] = {0};

    /* force to connect EMQ mqtt server with [client_id = g_mqtt_info.device_id] */
    if (!rt_os_strncmp(opts->channel, "EMQ", 3)) {
        snprintf(opts->client_id, sizeof(opts->client_id), "%s", g_mqtt_info.device_id);
    }

    MSG_PRINTF(LOG_DBG, "MQTT broker: addr [%s] id [%s] user [%s] passwd [%s]\n",
                    (const char *)opts->url,
                    (const char *)opts->client_id,
                    (const char *)opts->username,
                    (const char *)opts->password);

    if (MQTTClient_create(c, (const char *)opts->url, \
        (const char *)opts->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL) != 0) {
        MSG_PRINTF(LOG_WARN, "MQTTClient_create error\n");
        return RT_FALSE;
    }

    ret = MQTTClient_setCallbacks(*c, NULL,(MQTTClient_connectionLost *)mqtt_connection_lost,
                                 (MQTTClient_messageArrived *)mqtt_msg_arrived, NULL,
                                 (MQTTClient_extendedCmdArrive *)mqtt_ext_msg_arrived);
    MSG_PRINTF(LOG_INFO, "MQTTClient_setCallbacks %d\n", ret);

    pconn_opts->username = (const char *)opts->username;
    pconn_opts->password = (const char *)opts->password;
    if (!rt_os_strncmp(opts->channel, "YUNBA", 5)) {
        MSG_PRINTF(LOG_INFO, "connecting yunba mqtt server ...\n");
        pconn_opts->MQTTVersion = MQTTVERSION_YUNBA_3_1;
    } else if (!rt_os_strncmp(opts->channel, "EMQ", 3)) {
        MSG_PRINTF(LOG_INFO, "connecting emq mqtt server ...\n");
        pconn_opts->MQTTVersion = MQTTVERSION_3_1;
    }
    pconn_opts->keepAliveInterval   = MQTT_KEEP_ALIVE_INTERVAL;
    pconn_opts->reliable            = 0;
    pconn_opts->cleansession        = 0;
    
    //MSG_PRINTF(LOG_DBG, "pconn_opts->struct_version=%d\n", pconn_opts->struct_version);
    if (mqtt_connect(c, pconn_opts) == RT_FALSE) {
        MSG_PRINTF(LOG_WARN, "connecting %s mqtt server fail\r\n", opts->channel);
        if (++opts->try_connect_timer > MAX_TRY_CONNECT_TIME) {
            if (!rt_os_strncmp(opts->channel, "EMQ", 3)) {
                opts->last_connect_status = MQTT_CONNECT_EMQ_ERROR;
            } else {
                opts->last_connect_status = MQTT_CONNECT_YUNBA_ERROR;
            }
        } else {
            /* connect same mqtt channel for MAX_TRY_CONNECT_TIME times */
            opts->last_connect_status = MQTT_CONNECT_SUCCESS;
        }
        return RT_FALSE;
    }

    opts->last_connect_status = MQTT_CONNECT_SUCCESS;
    opts->try_connect_timer = 0;
    param->alias_rc = 1;

    MSG_PRINTF(LOG_WARN, "Connect mqtt server ok !\r\n");
    
    if (rt_os_strcmp(last_channel, opts->channel)) {
        MSG_PRINTF(LOG_DBG, "last mqtt channel changed: [%s] ==> [%s]\r\n", last_channel, opts->channel);
        snprintf(last_channel, sizeof(last_channel), "%s", opts->channel);
        upload_event_report("REGISTERED", NULL, 0, NULL);
    }
    
    mqtt_eid_check_upload();

    return RT_TRUE;
}

static void mqtt_config_global_alias(const char *eid, const char *device_id, rt_bool init_flag)
{
    const char *alias = NULL;

    if (mqtt_eid_check_memory(g_mqtt_info.eid, MAX_EID_LEN, 'F') || mqtt_eid_check_memory(g_mqtt_info.eid, MAX_EID_LEN, '0')) {
        alias = device_id;
    } else {
        alias = eid;
    }

    snprintf(g_mqtt_param.alias, sizeof(g_mqtt_param.alias), "%s", alias);
    snprintf(g_mqtt_param.opts.device_id, sizeof(g_mqtt_param.opts.device_id), "%s", device_id);
    
    if (init_flag) {
        rt_os_memset(g_mqtt_param.opts.client_id, 0, sizeof(g_mqtt_param.opts.client_id));
        rt_os_memcpy(g_mqtt_param.opts.channel, "EMQ", 3);  // default for EMQ
        g_mqtt_param.alias_rc = 1;
    }
}

static int32_t mqtt_state_get_server_addr(void)
{
    int32_t ret = RT_ERROR;
           
    /* If cache mqtt server addr ok, and then needn't to conenct ticket server to get mqtt server */
    if (g_mqtt_param.mqtt_get_addr == RT_FALSE) {
        /* re-subcribe when re-connect mqtt server */
        if (mqtt_get_server_addr(&g_mqtt_param) == RT_FALSE) {
            goto exit_entry;
        }
    }

    /* config mqtt global alias */
    mqtt_config_global_alias(g_mqtt_info.eid, g_mqtt_info.device_id, RT_FALSE);

    ret = RT_SUCCESS;

exit_entry:

    return ret;  
}

static int32_t mqtt_state_connecting(void)
{
    int32_t ret = RT_ERROR;      

    /* conenct mqtt server which has been got from cache ticket server or from adapter server */
    if (mqtt_connect_server(&g_mqtt_param) == RT_TRUE) {
        msg_send_agent_queue(MSG_ID_MQTT, MSG_MQTT_CONNECTED, NULL, 0);
        ret = RT_SUCCESS;
    } else {
        /* If cache mqtt server and hardcode mqtt server all fail, and then connect ticket server to get mqtt server */
        g_mqtt_param.mqtt_get_addr = RT_FALSE;
        ret = RT_ERROR;
    }

    return ret;
}

static int32_t mqtt_state_set_alias(void)
{
    int32_t ret = RT_SUCCESS;
    
    //MSG_PRINTF(LOG_DBG, "alias:%s, channel:%s\n", g_mqtt_param.alias, g_mqtt_param.opts.channel);
    if (!rt_os_strncmp(g_mqtt_param.opts.channel, "YUNBA", 5)) {
        if (rt_os_strlen(g_mqtt_param.alias) && (g_mqtt_param.alias_rc == RT_TRUE)) {
            g_mqtt_param.alias_rc = MQTTClient_set_alias(g_mqtt_param.client, (char *)g_mqtt_param.alias);
            MSG_PRINTF(LOG_DBG, "MQTTClient set alias : %s\n", g_mqtt_param.alias);
            if (g_mqtt_param.alias_rc != 0) {
                MSG_PRINTF(LOG_WARN, "MQTTSetAlias error, g_mqtt_param.alias_rc=%d\r\n", g_mqtt_param.alias_rc);
            }
        }
    }

    return ret;
}

static int32_t mqtt_state_subscribe_topics(void)
{
    int32_t ret;
    int32_t i;

    for (i = 0; i < MQTT_SUBSCRIBE_MAX_CNT; i++) {
        ret = mqtt_subscribe_all();
        if (ret == RT_SUCCESS || ret == MQTTCLIENT_DISCONNECTED) {
            ret = !ret ? RT_SUCCESS : RT_ERROR;
            break;
        }
        rt_os_sleep(1);
    }

    return ret;
}

static mqtt_event_e mqtt_state_wait_events(void)
{
    mqtt_event_e event = MQTT_IDLE_EVENT;
    network_state_info_e state = g_mqtt_param.network_state;
    
    while (1) {
        if (g_mqtt_param.network_state != state) {
            event = (g_mqtt_param.network_state == NETWORK_CONNECTED) ? MQTT_NETWORK_CONNECTED : MQTT_NETWORK_DISCONNECTED;
            break;
        }

        if (g_mqtt_param.lost_flag == RT_TRUE) {
            g_mqtt_param.lost_flag = RT_FALSE;
            event = MQTT_CONNECTED_LOST;
            break;
        }

        if (g_mqtt_param.subscribe_eid == RT_TRUE) {
            g_mqtt_param.subscribe_eid = RT_FALSE;
            event = MQTT_SUBSCRIBE_EID;
            break;
        }

        rt_os_sleep(1);
    }

    return event;
}

#define mqtt_client_state_changed(new_state)\
    do {\
        MSG_PRINTF(LOG_INFO, "MQTT STATE: %d ==> %d (%s)\r\n", g_mqtt_param.mqtt_state, new_state, #new_state);\
        g_mqtt_param.mqtt_state = new_state;\
    } while (0)

static void mqtt_client_state_mechine(void)
{
    int32_t delay_s = 1;
    int32_t reconnect_cnt = 0;
    mqtt_event_e event;
    
    while (1) {
        switch (g_mqtt_param.mqtt_state) {
            case MQTT_IDLE:
                mqtt_client_state_changed(MQTT_CHECK_NETWORK);
                delay_s = 0;
                break;

            case MQTT_CHECK_NETWORK:
                if (g_mqtt_param.network_state == NETWORK_CONNECTED) {
                    mqtt_client_state_changed(MQTT_GET_SERVER_ADDR);
                    delay_s = 0;
                } else {
                    delay_s = 1;
                }
                break;

            case MQTT_GET_SERVER_ADDR:
                if (mqtt_state_get_server_addr() == RT_SUCCESS) {
                    mqtt_client_state_changed(MQTT_CONNECTING);
                    delay_s = 0;
                    reconnect_cnt = 0;  // clear counter
                } else {                    
                    if (++reconnect_cnt >= MQTT_RECONNECT_MAX_CNT) {
                        mqtt_client_state_changed(MQTT_DISCONNECTED);
                        reconnect_cnt = 0;
                        network_force_down();
                    }
                    delay_s = 3;
                }
                break;

            case MQTT_FAST_RECONNECT:
                /* do reconnect only once after disconnect the last connection */
                mqtt_disconnect(&g_mqtt_param.client, NULL);
                if (mqtt_state_connecting() == RT_SUCCESS) {
                    /* jump to wait-event-state without those middle steps */
                    mqtt_client_state_changed(MQTT_WAIT_EVENT);
                    delay_s = 0;
                } else {                    
                    mqtt_client_state_changed(MQTT_DISCONNECTED);
                    delay_s = 3;
                }
                break;

            case MQTT_CONNECTING:
                if (mqtt_state_connecting() == RT_SUCCESS) {
                    mqtt_client_state_changed(MQTT_CONNECTED);
                    delay_s = 0;
                    reconnect_cnt = 0;  // clear counter
                } else {                    
                    if (++reconnect_cnt >= MQTT_RECONNECT_MAX_CNT) {
                        mqtt_client_state_changed(MQTT_DISCONNECTED);
                        reconnect_cnt = 0;
                        network_force_down();
                    }
                    delay_s = 3;
                }
                break;

            case MQTT_CONNECTED:
                mqtt_client_state_changed(MQTT_SET_ALIAS);
                delay_s = 0;
                break;

            case MQTT_SET_ALIAS:
                if (mqtt_state_set_alias() == RT_SUCCESS) {
                    mqtt_client_state_changed(MQTT_SUBSCRIBED);
                    delay_s = 0;
                } else {
                    mqtt_client_state_changed(MQTT_DISCONNECTED);
                    delay_s = 3;  
                }
                break;

            case MQTT_SUBSCRIBED:
                if (mqtt_state_subscribe_topics() == RT_SUCCESS) {
                    mqtt_client_state_changed(MQTT_WAIT_EVENT);
                    delay_s = 0;
                } else {
                    mqtt_client_state_changed(MQTT_DISCONNECTED);
                    delay_s = 3; 
                }
                break;

            case MQTT_WAIT_EVENT:
                event = mqtt_state_wait_events();
                if (event == MQTT_NETWORK_CONNECTED) {
                    ;
                } else if (event == MQTT_NETWORK_DISCONNECTED) {
                    mqtt_client_state_changed(MQTT_DISCONNECTED);
                } else if (event == MQTT_CONNECTED_LOST) {
                    mqtt_client_state_changed(MQTT_FAST_RECONNECT);
                } else if (event == MQTT_SUBSCRIBE_EID) {
                    mqtt_client_state_changed(MQTT_SUBSCRIBED);
                }                
                delay_s = 0;
                break;
                
            case MQTT_DISCONNECTED:
                mqtt_disconnect(&g_mqtt_param.client, NULL);
                mqtt_client_state_changed(MQTT_IDLE);
                delay_s = 1;
                break;

            default:
                MSG_PRINTF(LOG_ERR, "unexpected mqtt state %d !!!", g_mqtt_param.mqtt_state);
                mqtt_client_state_changed(MQTT_IDLE);
                delay_s = 1;
                break;
        }

        if (delay_s > 0) {
            rt_os_sleep(delay_s);
        }
    }

    MSG_PRINTF(LOG_DBG, "exit mqtt task\n");
    MQTTClient_destroy(&g_mqtt_param.client);
    rt_exit_task(NULL);
}

static int32_t mqtt_create_task(void)
{
    int32_t ret = RT_ERROR;
    rt_task id_connect;

    ret = rt_create_task(&id_connect, (void *)mqtt_client_state_mechine, NULL);
    if (ret == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "create mqtt pthread error, err(%d)=%s\r\n", errno, strerror(errno));
    }
    return ret;
}

//init parameter
static void mqtt_init_param(void)
{
#ifdef CFG_PLATFORM_ANDROID
    g_mqtt_param.network_state              = NETWORK_CONNECTED;
#else
    g_mqtt_param.network_state              = NETWORK_IDLE;
#endif
    g_mqtt_param.mqtt_state                 = MQTT_IDLE;
    g_mqtt_param.subscribe_eid              = RT_FALSE;
    g_mqtt_param.mqtt_flag                  = RT_FALSE;
    g_mqtt_param.mqtt_conn_state            = RT_FALSE;
    g_mqtt_param.lost_flag                  = RT_FALSE;
    g_mqtt_param.opts.nodelimiter           = 0;
    g_mqtt_param.opts.qos                   = MQTT_QOS_1;
    g_mqtt_param.opts.port                  = 0;
    g_mqtt_param.opts.showtopics            = 0;
    g_mqtt_param.opts.node_name             = NULL;
    g_mqtt_param.opts.try_connect_timer     = 0;  // Initialize the connect timer
    g_mqtt_param.opts.last_connect_status   = MQTT_CONNECT_SUCCESS;  // Initialize the last link push the state of the system
    mqtt_config_global_alias(g_mqtt_info.eid, g_mqtt_info.device_id, RT_TRUE);
    MQTTClient_init();
}

int32_t mqtt_connect_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    (void)buf;
    (void)len;

    //MSG_PRINTF(LOG_INFO, "mqtt connect event, mode: %d\r\n", mode);
    if (MSG_NETWORK_CONNECTED == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv network connected\r\n");
        g_mqtt_param.network_state = NETWORK_CONNECTED;
    } else if (MSG_NETWORK_DISCONNECTED == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv network disconnected\r\n");
        g_mqtt_param.network_state = NETWORK_DISCONNECTED;
    } else if (MSG_MQTT_SUBSCRIBE_EID == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv subcsribe eid request\r\n");
        /* set mqtt global alias */
        mqtt_config_global_alias(g_mqtt_info.eid, g_mqtt_info.device_id, RT_FALSE);
        CLR_SUBSCRIBE_FLAG(g_mqtt_param.subscribe_flag);
        g_mqtt_param.subscribe_eid = RT_TRUE;
    }

    return RT_SUCCESS;
}

int32_t init_mqtt(void *arg)
{
    int32_t ret = RT_ERROR;
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    rt_os_memset(&g_mqtt_info, 0, sizeof(g_mqtt_info));
    public_value_list->push_channel = (const char *)g_mqtt_param.opts.channel;
    g_mqtt_info.eid                 = (const char *)public_value_list->card_info->eid;
    g_mqtt_info.device_id           = (const char *)public_value_list->device_info->device_id;
    g_mqtt_info.imei                = (const char *)public_value_list->device_info->imei;
    g_mqtt_info.emq_addr            = (const char *)public_value_list->config_info->emq_addr;
    g_mqtt_info.oti_addr            = (const char *)public_value_list->config_info->oti_addr;

    mqtt_init_param();

    ret = mqtt_create_task();

    return ret;
}

