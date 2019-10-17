
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "rt_type.h"
#include "agent_main.h"
#include "config.h"
#include "cJSON.h"
#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
#include "downstream.h"

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

#define TICKET_SERVER_CACHE             "/data/redtea/ticket_server"
#define MAX_CONNECT_SERVER_TIMER        3

#define USE_ADAPTER_SERVER              1  // use mqtt ticket adapter proxy server ?

#define AGENT_ALIAS                     "agent"

#define MQTT_SUBCRIBE_QOS               MQTT_QOS_1

#define MQTT_PUBLISH_QOS                MQTT_QOS_1

#define MQTT_PUBLISH_TOPIC              "client_report"

#define MQTT_PUBLISH_TIMEOUT            3000L

#define MQTT_SUBCRIBE_TOPIC_MAX_CNT     10

#define MQTT_KEEP_ALIVE_INTERVAL        300

#define MQTT_RECONNECT_MAX_CNT          10

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

typedef enum MQTT_QOS {
    MQTT_QOS_0 = 0, 
    MQTT_QOS_1 = 1,
    MQTT_QOS_2 = 2,
} mqtt_qos_e;

typedef enum NETWORK_STATE {
    NETWORK_STATE_INIT = 0,
    NETWORK_CONNECTING,
    NETWORK_DIS_CONNECTED,
    NETWORK_USING
} network_state_info_e;

typedef struct MQTT_PARAM {
    MQTTClient_connectOptions   conn_opts;
    mqtt_info_t                 opts;
    MQTTClient                  client;
    network_state_info_e        state;
    char                        alias[MQTT_ALIAS_MAX_LEN];
    rt_bool                     mqtt_get_addr;
    rt_bool                     mqtt_flag;          // mqtt connect ok flag
    rt_bool                     lost_flag;          // mqtt connect lost flag
    rt_bool                     alias_rc;           // need set alias flag
    uint8_t                     subscribe_flag;     // subscribe ok flag
} mqtt_param_t;

typedef struct MQTT_SUBCRIBE_INFO {
    int32_t                     cnt;
    const char *                topic[MQTT_SUBCRIBE_TOPIC_MAX_CNT];
} mqtt_subcribe_info_t;

static mqtt_param_t g_mqtt_param            = {MQTTClient_connectOptions_initializer, 0};
static const char *g_mqtt_eid               = NULL;
static const char *g_mqtt_device_id         = NULL;
static const char *g_mqtt_imei              = NULL;
static const char *g_mqtt_emq_addr          = NULL;
static const char *g_mqtt_oti_addr          = NULL;
static mqtt_subcribe_info_t g_mqtt_sub_info = {0};

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
static rt_bool mqtt_save_ticket_server(const mqtt_info_t *opts)
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
    save_info = (int8_t *)cJSON_PrintUnformatted(obj);
    if (!save_info) {
        MSG_PRINTF(LOG_WARN, "save_info is NULL\n");
        goto exit_entry;
    }

    length = rt_os_strlen(save_info);
    data_len[0] = (length >> 8) & 0xff;
    data_len[1] = length & 0xff;

    if (rt_create_file(TICKET_SERVER_CACHE) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_create_file  error\n");
        goto exit_entry;
    }

    if (rt_write_data(TICKET_SERVER_CACHE, 0, data_len, sizeof(data_len)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_write_data data_len error\n");
        goto exit_entry;
    }

    if (rt_write_data(TICKET_SERVER_CACHE, sizeof(data_len), save_info, rt_os_strlen(save_info)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_write_data TICKET_SERVER_CACHE error\n");
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
static rt_bool mqtt_get_ticket_server(mqtt_info_t *opts)
{
    uint8_t  *save_info = NULL;
    uint8_t  data_len[2] = {0};  // frist 2 byte store length !!!
    int16_t length = 0;
    cJSON   *obj = NULL;
    cJSON   *channel = NULL;
    cJSON   *ticket_server = NULL;
    rt_bool ret = RT_FALSE;

    if (rt_read_data(TICKET_SERVER_CACHE, 0, data_len, sizeof(data_len)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_read_data data_len error\n");
        goto exit_entry;
    }

    length = (data_len[0] << 8) | data_len[1];

    if ((save_info = (int8_t *)rt_os_malloc(length)) == NULL) {
        MSG_PRINTF(LOG_ERR, "rt_os_malloc error\n");
        goto exit_entry;
    }

    if (rt_read_data(TICKET_SERVER_CACHE, sizeof(data_len), save_info, length) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_read_data save_info error\n");
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
    mqtt_info_t *opts = &param->opts;
    const char *alias = param->alias;
    const char *eid = "";

    if (!mqtt_eid_check_memory(g_mqtt_eid, MAX_EID_LEN, 'F') && !mqtt_eid_check_memory(g_mqtt_eid, MAX_EID_LEN, '0')) {
        eid = g_mqtt_eid;
    }

    mqtt_set_reg_url(g_mqtt_oti_addr, ADAPTER_PORT);
    MSG_PRINTF(LOG_DBG, "OTI server addr:%s, port:%d\r\n", g_mqtt_oti_addr, ADAPTER_PORT);

    /* connect redtea adpater server with max 3 times to get ticket server addr and port */
    do {
        if (mqtt_adapter_setup_with_appkey(ADAPTER_APPKEY, opts, eid) == 0) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "rt_mqtt_setup_with appkey num:%d\n", num++);
        rt_os_sleep(1);
    } while((num != MAX_CONNECT_SERVER_TIMER) && (g_mqtt_param.state != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (g_mqtt_param.state == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect adapter error\n");
        return RT_FALSE;
    }

    if (!rt_os_strncmp(opts->channel, "EMQ", 3)) {
        MSG_PRINTF(LOG_INFO, "mqtt alias: %s\r\n", alias);
        snprintf(opts->client_id, sizeof(opts->client_id), "%s", alias);
    }

    /* save ticket server into cache file */
    mqtt_save_ticket_server((const mqtt_info_t *)opts);

    return RT_TRUE;
}

//connect yunba server
static rt_bool mqtt_connect_yunba(mqtt_param_t *param, const char *ticket_server)
{
    uint8_t num = 0;
    int32_t ret;
    MQTTClient *c = &param->client;
    mqtt_info_t *opts = &param->opts;
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
    } while((num != MAX_CONNECT_SERVER_TIMER) && (g_mqtt_param.state != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (g_mqtt_param.state == NETWORK_DIS_CONNECTED)) {
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
    mqtt_info_t *opts = &param->opts;
    const char *alias = param->alias;

    if (ticket_server == NULL) {
        mqtt_set_reg_url(g_mqtt_emq_addr, EMQ_SERVER_PORT);
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
    } while((num != MAX_CONNECT_SERVER_TIMER) && (g_mqtt_param.state != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (g_mqtt_param.state == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect _emq error\n");
        return RT_FALSE;
    }

    snprintf(opts->channel, sizeof(opts->channel), "%s", "EMQ");
    snprintf(opts->client_id, sizeof(opts->client_id), "%s", alias);

    return RT_TRUE;
}

#if 1
#define TEST_FORCE_TO_ADAPTER           1
#define TEST_FORCE_TO_EMQ               0
#define TEST_FORCE_TO_YUNBA             0

#define MQTT_PASSAGEWAY_DEFINITION(x)\
do {\
    if (TEST_##x) {\
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
    if (g_mqtt_param.state == NETWORK_DIS_CONNECTED) {
        return RT_FALSE;
    }

    //attemp to connect adapter
    do{
        MQTT_PASSAGEWAY_DEFINITION(FORCE_TO_ADAPTER);
        MQTT_PASSAGEWAY_DEFINITION(FORCE_TO_EMQ);
        MQTT_PASSAGEWAY_DEFINITION(FORCE_TO_YUNBA);

FORCE_TO_ADAPTER:
        if (USE_ADAPTER_SERVER){
            if (mqtt_connect_adapter(param) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "connect adapter server to get mqtt server address EMQ or YUNBA successfull\n");
                goto ok_exit_entry;
            }

            if (mqtt_get_ticket_server(&param->opts) == RT_TRUE) {
                /* If connect yunba ticket server before, and then try this */
                if (!rt_os_strncmp(param->opts.channel, "YUNBA", 5) &&
                      (mqtt_connect_yunba(param, param->opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "get YUNBA mqtt server connect param successfull\n");
                    goto ok_exit_entry;
                }

                /* If connect EMQ ticket server before, and then try this */
                if (!rt_os_strncmp(param->opts.channel, "EMQ", 3) &&
                      (mqtt_connect_emq(param, param->opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "get EMQ mqtt server connect param successfull\n");
                    goto ok_exit_entry;
                }
            }
        }

        /* If connect adapter and ticket server all fail, and then try dead yunba server or EMQ server */
        if (!rt_os_strncmp(param->opts.channel, "YUNBA", 5)) {
FORCE_TO_EMQ:
            if (mqtt_connect_emq(param, NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "get EMQ mqtt server connect param successfull\n");
                goto ok_exit_entry;
            }
        } else if (!rt_os_strncmp(param->opts.channel, "EMQ", 3)) {
FORCE_TO_YUNBA:
            if (mqtt_connect_yunba(param, NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "get yunba mqtt server connect param successfull\n");
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

/************* mqtt server process ******************/

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
    int32_t rc;
    
    pubmsg.payload      = (void *)data;
    pubmsg.payloadlen   = data_len;
    pubmsg.qos          = qos;
    pubmsg.retained     = 0;

    #if 1
    while (1) {
        rc = MQTTClient_publishMessage(handle, topic, &pubmsg, &token);
        //rc = MQTTClient_publish(client, topic, data_len, data);
        MSG_PRINTF(LOG_WARN, "Waiting for up to %d seconds for publication of (%d bytes): \r\n\r\n%s\r\n\r\n"
                "on topic %s for client with ClientID: %s, rc=%d, token=%d\n",
                (int32_t)(MQTT_PUBLISH_TIMEOUT/1000), data_len, (const char *)data, topic, (const char *)g_mqtt_device_id, rc, token);
        MQTTClient_waitForCompletion(handle, token, MQTT_PUBLISH_TIMEOUT);
        MSG_PRINTF(LOG_INFO, "Message with delivery token %d delivered, rc=%d\n", token, rc);
        //sleep(TIMEOUT/1000);
        if (rc != 0 && token != 0) {
            MSG_PRINTF(LOG_WARN, "MQTT publish msg fail !\r\n");
            rc = -1;
            goto exit_entry;
        }
        break;
    }
    #else    
    deliveredtoken = 0;
    rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MSG_PRINTF(LOG_INFO, "Waiting for up to %d seconds for publication of %s\n"
                "on topic %s for client with ClientID: %s, rc=%d\n",
                (int)(TIMEOUT/1000), (const char *)data, topic, (const char *)client_id, rc);
    while(deliveredtoken != token);
    #endif

    rc = 0;

exit_entry:

    //MSG_PRINTF(LOG_INFO, "mqtt pulish (%d bytes) ret=%d\r\n", data_len, rc);

    return rc;
}

static int32_t mqtt_pulish(const char* topic, const void* data, int32_t dataå_len)
{
    int32_t ret;

    ret = mqtt_client_pulish_msg(g_mqtt_param.client, g_mqtt_param.opts.qos, topic, data, data_len);
    //MSG_PRINTF(LOG_WARN, "mqtt pulish (%d bytes): %s, ret=%d\r\n", data_len, (const char*)data, ret);

    return ret;
}

int32_t mqtt_pulish_msg(const void* data, int32_t data_len)
{
    return mqtt_pulish(MQTT_PUBLISH_TOPIC, data, data_len);
}

static int32_t mqtt_subcribe_all(void)
{
    int32_t ret;
    int32_t offset = 0;
    int32_t cnt = 0;
    rt_bool need_subcribe = RT_FALSE;
    char topic_list[256] = {0};
    
    /* subscribe [cid/eid] */
    if ((GET_EID_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE) && \
        (rt_os_strlen(g_mqtt_param.alias)) && \
        (rt_os_strcmp((const char *)g_mqtt_param.alias, g_mqtt_device_id))) {
        g_mqtt_sub_info.topic[g_mqtt_sub_info.cnt] = (const char *)g_mqtt_param.alias;  
        offset = offset + cnt;
        cnt = snprintf(&topic_list[offset], sizeof(topic_list) - offset, "[%s] ", g_mqtt_sub_info.topic[g_mqtt_sub_info.cnt]);
        g_mqtt_sub_info.cnt++;
        need_subcribe = RT_TRUE;
    }

    /* subscribe device-id */
    if ((GET_DEVICE_ID_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
        g_mqtt_sub_info.topic[g_mqtt_sub_info.cnt] = (const char *)g_mqtt_device_id;
        offset = offset + cnt;
        cnt = snprintf(&topic_list[offset], sizeof(topic_list) - offset, "[%s] ", g_mqtt_sub_info.topic[g_mqtt_sub_info.cnt]);
        g_mqtt_sub_info.cnt++;
        need_subcribe = RT_TRUE;
    }

    /* subscribe imei */
    if ((GET_IMEI_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
        g_mqtt_sub_info.topic[g_mqtt_sub_info.cnt] = (const char *)g_mqtt_imei;
        offset = offset + cnt;
        cnt = snprintf(&topic_list[offset], sizeof(topic_list) - offset, "[%s] ", g_mqtt_sub_info.topic[g_mqtt_sub_info.cnt]);
        g_mqtt_sub_info.cnt++; 
        need_subcribe = RT_TRUE;
    }

    /* subscribe agent  */
    if ((GET_AGENT_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
        g_mqtt_sub_info.topic[g_mqtt_sub_info.cnt] = (const char *)AGENT_ALIAS;
        offset = offset + cnt;
        cnt = snprintf(&topic_list[offset], sizeof(topic_list) - offset, "[%s] ", g_mqtt_sub_info.topic[g_mqtt_sub_info.cnt]);
        g_mqtt_sub_info.cnt++;
        need_subcribe = RT_TRUE;
    }

    if (!need_subcribe) {
        return 0;
    }

    ret = mqtt_subcribe_many_with_default_qos(g_mqtt_param.client, g_mqtt_sub_info.cnt, g_mqtt_sub_info.topic);
    if (ret == 0) {
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
        g_mqtt_sub_info.cnt = 0;  // clear counter
        MSG_PRINTF(LOG_WARN, "mqtt subcribe %s fail !\r\n", topic_list);
        if (ret == MQTTCLIENT_DISCONNECTED) {
            g_mqtt_param.state = NETWORK_DIS_CONNECTED;
            MSG_PRINTF(LOG_WARN, "mqtt disconnected !\r\n");
        }
    }

    return ret;
}

static rt_bool mqtt_check_topic(const char *topic)
{
    int32_t i;

#if 0
    MSG_PRINTF(LOG_INFO, "g_mqtt_sub_info.cnt=%d\r\n", g_mqtt_sub_info.cnt);
    for (i = 0; i < g_mqtt_sub_info.cnt; i++) {
        MSG_PRINTF(LOG_INFO, "%d: %s\r\n", i, g_mqtt_sub_info.topic[i]);
    }
#endif

    for (i = 0; i < g_mqtt_sub_info.cnt; i++) {
        if (!rt_os_strcmp(g_mqtt_sub_info.topic[i], topic)) {
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
    
    msg[len] = '\0';
    MSG_PRINTF(LOG_INFO, "msg arrived, topic:%s, len: %d bytes, %s\r\n", topic_name, len, msg);
    
    if (!mqtt_check_topic(topic_name)) {
        MSG_PRINTF(LOG_WARN, "mqtt unexpected topic [%s] arrived, ignore !\r\n", topic_name);
    } else {
        downstream_msg_handle((const char *)msg, len);
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
        g_mqtt_param.mqtt_flag = RT_TRUE;
        g_mqtt_param.lost_flag = RT_FALSE;
        MSG_PRINTF(LOG_WARN, "Connect mqtt ok ! [%p]\n", pthread_self());
        return RT_TRUE;
    } else {
        MSG_PRINTF(LOG_WARN, "Failed to connect error:%d [%p]\n", c, pthread_self());
        return RT_FALSE;
    }
}

static void mqtt_connection_lost(void *context, char *cause)
{
    MSG_PRINTF(LOG_WARN, "connection lost: %s, %s\r\n",(char *)context, cause);
    if (mqtt_connect(&g_mqtt_param.client, &g_mqtt_param.conn_opts) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "connect again fail after connection lost !!!\r\n");
        if (!rt_os_strncmp(g_mqtt_param.opts.channel, "EMQ", 3)) {
            g_mqtt_param.opts.last_connect_status = MQTT_CONNECT_EMQ_ERROR;
        } else {
            g_mqtt_param.opts.last_connect_status = MQTT_CONNECT_YUNBA_ERROR;
        }

        if (g_mqtt_param.state == NETWORK_USING) {
            g_mqtt_param.state = NETWORK_DIS_CONNECTED;
        }
        g_mqtt_param.lost_flag = RT_TRUE;
    } else {
        MSG_PRINTF(LOG_WARN, "connect again ok after connection lost !!!\r\n");
    }
}

static rt_bool mqtt_eid_check_upload(void)
{
    if (mqtt_eid_check_memory(g_mqtt_eid, MAX_EID_LEN, '0')) {
        upload_event_report("NO_CERT", NULL, 0, NULL);
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
    mqtt_info_t *opts = &param->opts;

    /* force to connect EMQ mqtt server with [client_id = g_mqtt_device_id] */
    if (!rt_os_strncmp(opts->channel, "EMQ", 3)) {
        snprintf(opts->client_id, sizeof(opts->client_id), "%s", g_mqtt_device_id);
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
        MSG_PRINTF(LOG_WARN, "connecting yunba mqtt server ...\n");
        pconn_opts->MQTTVersion = 0x13;
    } else if (!rt_os_strncmp(opts->channel, "EMQ", 3)) {
        MSG_PRINTF(LOG_WARN, "connecting emq mqtt server ...\n");
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

    upload_event_report("REGISTERED", NULL, 0, NULL);
    mqtt_eid_check_upload();
    
    return RT_TRUE;
}

static void rt_mqtt_set_alias(const char *eid, const char *device_id, rt_bool init_flag)
{
    rt_os_memset(g_mqtt_param.alias, 0, sizeof(g_mqtt_param.alias));
    if (mqtt_eid_check_memory(g_mqtt_eid, MAX_EID_LEN, 'F') || mqtt_eid_check_memory(g_mqtt_eid, MAX_EID_LEN, '0')) {
        rt_os_memcpy(g_mqtt_param.alias, device_id, rt_os_strlen(device_id));
    } else {
        rt_os_memcpy(g_mqtt_param.alias, eid, rt_os_strlen(eid));
    }
    
    rt_os_memset(g_mqtt_param.opts.device_id, 0, sizeof(g_mqtt_param.opts.device_id));
    rt_os_memcpy(g_mqtt_param.opts.device_id, device_id, rt_os_strlen(device_id));
    
    if (init_flag) {
        rt_os_memset(g_mqtt_param.opts.client_id, 0, sizeof(g_mqtt_param.opts.client_id));
        rt_os_memcpy(g_mqtt_param.opts.channel, "EMQ", 3);  // default for EMQ
        g_mqtt_param.alias_rc = 1;
    }
}

static void mqtt_process_task(void)
{
    int32_t rc;
    int32_t wait_cnt = 0;
    int32_t connect_cnt = 0;
    
    while(1) {        
        if (g_mqtt_param.state == NETWORK_STATE_INIT) {
            /* check network state every 60 seconds after mqtt connect lost */
            if (g_mqtt_param.lost_flag == RT_TRUE && ++wait_cnt > MQTT_NETWORK_STATE_TIMEOUT) {
                wait_cnt = 0;
                network_state_update(1);  // update newest network state after 1 seconds
                rt_os_sleep(1);
            }
        } else if (g_mqtt_param.state == NETWORK_CONNECTING) {
            if(g_mqtt_param.mqtt_flag == RT_FALSE) {
                if (++connect_cnt > MQTT_RECONNECT_MAX_CNT) {
                    MSG_PRINTF(LOG_DBG, "force to set network disconnected !\n");
                    network_state_force_update(MSG_NETWORK_DISCONNECTED);
                    g_mqtt_param.state = NETWORK_STATE_INIT;  // reset network state
                    connect_cnt = 0;
                    continue;
                }
                
                /* If cache mqtt server addr ok, and then needn't to conenct ticket server to get mqtt server */
                if (g_mqtt_param.mqtt_get_addr == RT_FALSE) {
                    /* re-subcribe when re-connect mqtt server */
                    if (mqtt_get_server_addr(&g_mqtt_param) == RT_FALSE) {
                        continue;
                    }
                }

                /* set mqtt global alias */
                rt_mqtt_set_alias(g_mqtt_eid, g_mqtt_device_id, RT_FALSE);

                /* conenct mqtt server which has been got from cache ticket server or from adapter server */
                if (mqtt_connect_server(&g_mqtt_param) == RT_TRUE) {
                    /* set network using */
                    g_mqtt_param.state = NETWORK_USING;
                    connect_cnt = 0;
                    continue;
                } else {
                    /* If cache mqtt server and hardcode mqtt server all fail, and then connect ticket server to get mqtt server */
                    g_mqtt_param.mqtt_get_addr = RT_FALSE;
                }
            }
        } else if (g_mqtt_param.state == NETWORK_DIS_CONNECTED) {
            if (g_mqtt_param.mqtt_flag == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "MQTTClient disconnect\n");
                MQTTClient_disconnect(g_mqtt_param.client, 0);                
                g_mqtt_param.mqtt_flag      = RT_FALSE;
                g_mqtt_param.subscribe_flag = 0;  // reset subscribe flag
                g_mqtt_param.state          = NETWORK_STATE_INIT;  // reset network state
                wait_cnt                    = 0;  // reset wait counter
            }
        } else if (g_mqtt_param.state == NETWORK_USING) {
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

            mqtt_subcribe_all();
            
            rt_os_sleep(5); //delay more time is ok
        }

        rt_os_sleep(1);
    }

    MSG_PRINTF(LOG_DBG, "exit mqtt task\n");
    MQTTClient_destroy(&g_mqtt_param.client);
}

static int32_t mqtt_create_task(void)
{
    int32_t ret = RT_ERROR;
    rt_task id_connect;

    ret = rt_create_task(&id_connect, (void *)mqtt_process_task, NULL);
    if (ret == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "creat mqtt pthread error, err(%d)=%s\r\n", errno, strerror(errno));
    }
    return ret;
}

//init parameter
static void mqtt_init_param(void)
{
    g_mqtt_param.state                      = NETWORK_STATE_INIT;
    g_mqtt_param.opts.nodelimiter           = 0;
    g_mqtt_param.opts.qos                   = MQTT_QOS_1;
    g_mqtt_param.opts.port                  = 0;
    g_mqtt_param.opts.showtopics            = 0;
    g_mqtt_param.opts.node_name             = NULL;
    g_mqtt_param.opts.try_connect_timer     = 0;  // Initialize the connect timer
    g_mqtt_param.opts.last_connect_status   = MQTT_CONNECT_SUCCESS;  // Initialize the last link push the state of the system
    rt_mqtt_set_alias(g_mqtt_eid, g_mqtt_device_id, RT_TRUE);
    MQTTClient_init();
}

int32_t mqtt_connect_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    (void)buf;
    (void)len;

    //MSG_PRINTF(LOG_INFO, "mqtt connect event, mode: %d\r\n", mode);
    if (MSG_NETWORK_CONNECTED == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv network connected\r\n");
        g_mqtt_param.state = NETWORK_CONNECTING;
    } else if (MSG_NETWORK_DISCONNECTED == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv network disconnected\r\n");
        g_mqtt_param.state = NETWORK_DIS_CONNECTED;
    } else if (MSG_MQTT_SUBSCRIBE_EID == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv subcsribe eid request\r\n");
        /* set mqtt global alias */
        rt_mqtt_set_alias(g_mqtt_eid, g_mqtt_device_id, RT_FALSE);
        CLR_EID_FLAG(g_mqtt_param.subscribe_flag);
    }

    return RT_SUCCESS;
}

int32_t init_mqtt(void *arg)
{
    int32_t ret = RT_ERROR;
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    public_value_list->push_channel = (const char *)g_mqtt_param.opts.channel;
    g_mqtt_eid = (const char *)public_value_list->card_info->eid;
    g_mqtt_device_id = (const char *)public_value_list->device_info->device_id;
    g_mqtt_imei = (const char *)public_value_list->device_info->imei;
    g_mqtt_emq_addr = (const char *)public_value_list->config_info->emq_addr;
    g_mqtt_oti_addr = (const char *)public_value_list->config_info->oti_addr;

    mqtt_init_param();

    ret = mqtt_create_task();
    if (ret) {
        goto exit_entry;
    }

exit_entry:

    return ret;
}

