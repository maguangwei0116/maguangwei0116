
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

#define ADAPTER_APPKEY          "D358134D21684E8FA23CC25740506A82"
#define ADAPTER_PORT            7082

#define YUNBA_APPKEY            "596882bf2a9275716fe3c1e2"  // APPKET of YUNBA server
#define YUNBA_SERVER_IP         "23.91.101.68"  // the IP address of yunba server
#define YUNBA_URL               "tcp://23.91.101.68:1883"
#define YUNBA_SERVER_PORT       8085  // the port of yunba server
#define YUNBA_SERVER_PORT2      8383

#define EMQ_APPKEY              "12345"  // APPKET of EMQ server
#define EMQ_SERVER_PORT         8085  // the port of EMQ server

typedef enum NETWORK_STATE {
    NETWORK_STATE_INIT = 0,
    NETWORK_GET_IP,
    NETWORK_CONNECTING,
    NETWORK_DIS_CONNECTED,
    NETWORK_USING
} network_state_info_e;

typedef struct MQTT_PARAM {
    MQTTClient_connectOptions   conn_opts;
    mqtt_info                   opts;
    MQTTClient                  client;
    network_state_info_e        state;
    char                        alias[40];
    rt_bool                     mqtt_get_addr;
    rt_bool                     mqtt_flag;          // mqtt connect ok falg
    rt_bool                     alias_rc;           // need set alias flag
    uint8_t                     subscribe_flag;     // subscribe ok flag
} mqtt_param_t;

static mqtt_param_t g_mqtt_param    = {MQTTClient_connectOptions_initializer, 0};
static const char *g_mqtt_eid       = NULL;
static const char *g_mqtt_device_id = NULL;
static const char *g_mqtt_imei      = NULL;
static network_state_info_e g_mqtt_network_state = NETWORK_STATE_INIT;

static network_state_info_e get_network_state(void)
{
    return g_mqtt_network_state;
}

static void set_network_state(network_state_info_e state)
{
    //MSG_PRINTF(LOG_INFO("set network state: %d\r\n", state);
    g_mqtt_network_state = state;
}

static void msg_parse(int8_t *message, int32_t len)
{
    message[len] = '\0';
    MSG_PRINTF(LOG_WARN, "mqtt recv msg (%d bytes): %s\r\n", len, message);
    downstream_msg_handle(message, len);
}

static rt_bool eid_check_memory(const void *buf, int32_t len, int32_t value)
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
static rt_bool save_ticket_server(mqtt_info *opts)
{
    cJSON   *obj = NULL;
    int8_t  *save_info = NULL;
    int8_t  data_len[2];
    int16_t length = 0;
    rt_bool ret;

    if ((obj = cJSON_CreateObject()) == NULL) {
        MSG_PRINTF(LOG_WARN, "cJSON_CreateObject error\n");
        ret = RT_FALSE;
        goto exit_entry;
    }

    cJSON_AddItemToObject(obj, "channel", cJSON_CreateString(opts->rt_channel));
    cJSON_AddItemToObject(obj, "ticketServer", cJSON_CreateString(opts->ticket_server));
    save_info = (int8_t *)cJSON_PrintUnformatted(obj);
    length = rt_os_strlen(save_info);

    data_len[0] = (length >> 8) & 0xff;
    data_len[1] = length & 0xff;

    if (rt_create_file(TICKET_SERVER_CACHE) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_create_file  error\n");
        ret = RT_FALSE;
        goto exit_entry;
    }

    if (rt_write_data(TICKET_SERVER_CACHE, 0, data_len, 2) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_write_data data_len error\n");
        ret = RT_FALSE;
        goto exit_entry;
    }

    if (rt_write_data(TICKET_SERVER_CACHE, 2, save_info, rt_os_strlen(save_info)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_write_data TICKET_SERVER_CACHE error\n");
        ret = RT_FALSE;
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
static rt_bool get_ticket_server(mqtt_info *opts)
{
    int8_t  *save_info;
    int8_t  data_len[2];
    int16_t length = 0;
    cJSON   *obj = NULL;
    cJSON   *channel = NULL;
    cJSON   *ticket_server = NULL;
    rt_bool ret;

    if (rt_read_data(TICKET_SERVER_CACHE, 0, data_len, 2) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_read_data data_len error\n");
        ret = RT_FALSE;
        goto exit_entry;
    }

    length = (data_len[0] << 8) | data_len[1];

    if ((save_info = (int8_t *)rt_os_malloc(length)) == NULL) {
        MSG_PRINTF(LOG_ERR, "rt_os_malloc error\n");
        ret = RT_FALSE;
        goto exit_entry;
    }

    if (rt_read_data(TICKET_SERVER_CACHE,2, save_info, length) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_read_data save_info error\n");
        ret = RT_FALSE;
        goto exit_entry;
    }

    if ((obj = cJSON_Parse(save_info)) == NULL) {
        MSG_PRINTF(LOG_WARN, "cJSON_Parse error\n");
        ret = RT_FALSE;
        goto exit_entry;
    }
    channel = cJSON_GetObjectItem(obj, "channel");
    ticket_server = cJSON_GetObjectItem(obj, "ticketServer");

    if (!channel || !ticket_server) {
        MSG_PRINTF(LOG_WARN, "channel or ticket_server is NULL\n");
        ret = RT_FALSE;
        goto exit_entry;
    }

    snprintf((char *)opts->rt_channel, sizeof(opts->rt_channel), "%s", channel->valuestring);
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

static rt_bool mqtt_get_ip_pair(const int8_t *url, int8_t *addr, int32_t *port)
{
    int8_t *p = (int8_t *)strstr((char *)url, "http://");
    if (p) {
        p += 7;
        int8_t *q = (int8_t *)strstr((char *)p, ":");
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

/**************************************************************************************************************/
static rt_bool rt_mqtt_connect_adapter(mqtt_param_t *param)
{
    uint8_t num = 0;
    MQTTClient *c = &param->client;
    mqtt_info *opts = &param->opts;
    const char *alias = param->alias;
    const char *eid = NULL;

    eid = eid_check_memory(g_mqtt_eid, MAX_EID_LEN, '0') ? "" : g_mqtt_eid;

    set_reg_url(OTI_ENVIRONMENT_ADDR, ADAPTER_PORT);
    MSG_PRINTF(LOG_DBG, "OTI server addr:%s, port:%d\r\n", OTI_ENVIRONMENT_ADDR, ADAPTER_PORT);

    /* connect redtea adpater server with max 3 times to get ticket server addr and port */
    do {
        if (rt_mqtt_setup_with_appkey(ADAPTER_APPKEY, opts, eid) == 0) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "rt_mqtt_setup_with appkey num:%d\n", num++);
        rt_os_sleep(1);
    } while((num != MAX_CONNECT_SERVER_TIMER) && (get_network_state() != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (get_network_state() == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect adapter error\n");
        return RT_FALSE;
    }

    if (!rt_os_strncmp(opts->rt_channel, "EMQ", 3)) {
        MSG_PRINTF(LOG_INFO, "mqtt alias: %s\r\n", alias);
        snprintf(opts->client_id, sizeof(opts->client_id), "%s", alias);
    }

    /* save ticket server into cache file */
    save_ticket_server(opts);

    return RT_TRUE;
}

//connect yunba server
static rt_bool rt_mqtt_connect_yunba(mqtt_param_t *param, int8_t *ticket_server)
{
    uint8_t num = 0;
    MQTTClient *c = &param->client;
    mqtt_info *opts = &param->opts;
    const char *alias = param->alias;

    if (ticket_server == NULL) {
        set_reg_url((const char *)YUNBA_SERVER_IP, YUNBA_SERVER_PORT2);
    } else {
        int8_t addr[20];
        int32_t port;
        if (mqtt_get_ip_pair(ticket_server, addr, &port) == RT_TRUE) {
            set_reg_url((char *)addr, port);
        } else {
            MSG_PRINTF(LOG_WARN, "mqtt_get_ip_pair error ticket_serverL:%s\n", ticket_server);
            return RT_FALSE;
        }
        MSG_PRINTF(LOG_WARN, "ticket_server:%s, yunba addr:%s, port:%d\n", ticket_server, addr, port);
    }

    /* connect yunba mqtt server with max 3 times */
    do {
        int ret;
        if ((ret = MQTTClient_setup_with_appkey_and_deviceid(YUNBA_APPKEY, (char *)opts->device_id, opts)) == 0) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "MQTTClient_setup_with_appkey_and_deviceid num:%d, error ret:%d\n", num++, ret);
        rt_os_sleep(1);
    } while((num != MAX_CONNECT_SERVER_TIMER) && (get_network_state() != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (get_network_state() == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect yunba error\n");
        return RT_FALSE;
    }

    snprintf(opts->rt_channel, sizeof(opts->rt_channel), "%s", "YUNBA");
    snprintf(opts->rt_url, sizeof(opts->rt_url), "%s", YUNBA_URL);

    return RT_TRUE;
}


//connect EMQ server
static rt_bool rt_mqtt_connect_emq(mqtt_param_t *param, int8_t *ticket_server)
{
    uint8_t num = 0;
    MQTTClient *c = &param->client;
    mqtt_info *opts = &param->opts;
    const char *alias = param->alias;

    if (ticket_server == NULL) {
        set_reg_url(EMQ_SERVER_ADDR, EMQ_SERVER_PORT);
    } else {
        int8_t addr[20];
        int32_t port;
        if (mqtt_get_ip_pair(ticket_server, addr, &port) == RT_TRUE) {
            set_reg_url((char *)addr, port);
        } else {
            MSG_PRINTF(LOG_WARN, "mqtt_get_ip_pair error ticket_serverL:%s\n", ticket_server);
            return RT_FALSE;
        }
        MSG_PRINTF(LOG_WARN, "ticket_server:%s, EMQ addr:%s, port:%d\n", ticket_server, addr, port);
    }

    /* connect EMQ mqtt server with max 3 times */
    do {
        if ((MQTTClient_setup_with_appkey(EMQ_APPKEY, opts) == 0) &&
           (MQTTClient_get_host((char *)opts->nodeName, (char *)opts->rt_url, EMQ_APPKEY) == 0)) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "MQTTClient_setup_with_appkey num:%d\n", num++);
        rt_os_sleep(1);
    } while((num != MAX_CONNECT_SERVER_TIMER) && (get_network_state() != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (get_network_state() == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect _emq error\n");
        return RT_FALSE;
    }

    snprintf(opts->rt_channel, sizeof(opts->rt_channel), "%s", "EMQ");
    snprintf(opts->client_id, sizeof(opts->client_id), "%s", alias);

    return RT_TRUE;
}

#if 1
#define TEST_FORCE_TO_ADAPTER       1
#define TEST_FORCE_TO_EMQ           0
#define TEST_FORCE_TO_YUNBA         0

#define MQTT_PASSAGEWAY_DEF(x)\
do {\
    if (TEST_##x) {\
        MSG_PRINTF(LOG_DBG, "MQTT %s ...\r\n", #x);\
        goto x;\
    }\
} while(0)
#else
#define MQTT_PASSAGEWAY_DEF(x)      do {} while(0)
#endif

//used to get mqtt server addr
static rt_bool mqtt_get_server_addr(mqtt_param_t *param)
{
    if (get_network_state() == NETWORK_DIS_CONNECTED) {
        return RT_FALSE;
    }

    //attemp to connect adapter
    do{
        MQTT_PASSAGEWAY_DEF(FORCE_TO_ADAPTER);
        MQTT_PASSAGEWAY_DEF(FORCE_TO_EMQ);
        MQTT_PASSAGEWAY_DEF(FORCE_TO_YUNBA);

FORCE_TO_ADAPTER:
        if (USE_ADAPTER_SERVER){
            if (rt_mqtt_connect_adapter(param) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "connect adapter ticket server to get mqtt server address EMQ or YUNBA\n");
                break;
            }

            if (get_ticket_server(&param->opts) == RT_TRUE) {
                /* If connect yunba ticket server before, and then try this */
                if (!rt_os_strncmp(param->opts.rt_channel, "YUNBA", 5) &&
                      (rt_mqtt_connect_yunba(param, param->opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "get YUNBA mqtt server connect param successfull\n");
                    break;
                }

                /* If connect EMQ ticket server before, and then try this */
                if (!rt_os_strncmp(param->opts.rt_channel, "EMQ", 3) &&
                      (rt_mqtt_connect_emq(param, param->opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "get EMQ mqtt server connect param successfull\n");
                    break;
                }
            }
        }

        /* If connect adapter and ticket server all fail, and then try dead yunba server or EMQ server */
        if (!rt_os_strncmp(param->opts.rt_channel, "YUNBA", 5)) {
FORCE_TO_EMQ:
            if (rt_mqtt_connect_emq(param, NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "get EMQ mqtt server connect param successfull\n");
                break;
            }
        } else if (!rt_os_strncmp(param->opts.rt_channel, "EMQ", 3)) {
FORCE_TO_YUNBA:
            if (rt_mqtt_connect_yunba(param, NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "get yunba mqtt server connect param successfull\n");
                break;
            }
        }
    }while(1);

    g_mqtt_param.mqtt_get_addr = RT_TRUE;
    return RT_TRUE;
}

/************* mqtt server process ******************/

//Registe callback of message arrive
static int message_arrived(void* context, char* topicName, int32_t topicLen, MQTTClient_message* md)
{
#if 0 && PLATFORM == PLATFORM_FIBCOM
    if (!strcmp(fibo_thread_GetMyName(),"unknown"))
    {
        fibo_thread_InitFibofwkThreadData("sdk_theard");
    }
#endif

    //parse JSON
    MSG_PRINTF(LOG_DBG, "msg arrived, topicName:%s\n",topicName);
    msg_parse(md->payload, (int32_t)md->payloadlen);
    MQTTClient_freeMessage(&md);
    MQTTClient_free(topicName);
    //MSG_PRINTF(LOG_DBG, "topicName:%s ok!\n",topicName);

    return 1;
}

//Registe callback of extra message arrive
static int ext_message_arrive(void *context, EXTED_CMD cmd, int32_t status, int32_t ret_string_len, int8_t *ret_string)
{
    MSG_PRINTF(LOG_WARN, "%s, cmd:%d, status:%d, payload: %.*s\n", __func__, cmd, status, ret_string_len, ret_string);
    return 0;
}

static rt_bool my_connect(MQTTClient* client, MQTTClient_connectOptions* opts)
{
    int32_t c = 0;

    //MSG_PRINTF(LOG_WARN, "Connect mqtt broker [%s] !\n", opts->serverURIs);
    if ((c = MQTTClient_connect(*client, opts)) == 0) {
        g_mqtt_param.mqtt_flag = RT_TRUE;
        MSG_PRINTF(LOG_WARN, "Connect mqtt ok ! [%p]\n", pthread_self());
        return RT_TRUE;
    } else {
        MSG_PRINTF(LOG_WARN, "Failed to connect error:%d [%p]\n", c, pthread_self());
        return RT_FALSE;
    }
}

static void connection_lost(void *context, char *cause)
{
    MSG_PRINTF(LOG_WARN, "connection lost: %s, %s\r\n",(char *)context, cause);
    if (my_connect(&g_mqtt_param.client, &g_mqtt_param.conn_opts) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "connect again fail after connection lost !!!\r\n");
        if (!rt_os_strncmp(g_mqtt_param.opts.rt_channel, "EMQ", 3)) {
            g_mqtt_param.opts.last_connect_status = MQTT_CONNECT_EMQ_ERROR;
        } else {
            g_mqtt_param.opts.last_connect_status = MQTT_CONNECT_YUNBA_ERROR;
        }

        if (get_network_state() == NETWORK_USING) {
            set_network_state(NETWORK_DIS_CONNECTED);
        }
    } else {
        MSG_PRINTF(LOG_WARN, "connect again ok after connection lost !!!\r\n");
    }
}

static rt_bool eid_check_upload(void)
{
    if (!g_mqtt_eid || !rt_os_strlen(g_mqtt_eid) || eid_check_memory(g_mqtt_eid, MAX_EID_LEN, '0')){
        upload_event_report("NO_CERT", NULL, 0, NULL);
        return RT_TRUE;
    }  

    return RT_FALSE;
}

//connect mqtt server
static rt_bool rt_mqtt_connect_server(mqtt_param_t *param)
{
    int32_t ret = 0;
    MQTTClient *c = &param->client;
    mqtt_info *opts = &param->opts;
    MQTTClient_connectOptions *pconn_opts = &param->conn_opts;

    /* force to connect EMQ mqtt server with [client_id = g_mqtt_device_id] */
    if (!rt_os_strncmp(opts->rt_channel, "EMQ", 3)) {
        snprintf(opts->client_id, sizeof(opts->client_id), "%s", g_mqtt_device_id);
    }

    MSG_PRINTF(LOG_DBG, "MQTT broker: addr [%s] id [%s] user [%s] passwd [%s]\n",
                    (const char *)opts->rt_url,
                    (const char *)opts->client_id,
                    (const char *)opts->username,
                    (const char *)opts->password);

    if (MQTTClient_create(c, (const char *)opts->rt_url, \
        (const char *)opts->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL) != 0) {
        MSG_PRINTF(LOG_WARN, "MQTTClient_create error\n");
        return RT_FALSE;
    }

    ret = MQTTClient_setCallbacks(*c, NULL,(MQTTClient_connectionLost *)connection_lost,
                                 (MQTTClient_messageArrived *)message_arrived, NULL,
                                 (MQTTClient_extendedCmdArrive *)ext_message_arrive);
    MSG_PRINTF(LOG_INFO, "MQTTClient_setCallbacks %d\n", ret);

    pconn_opts->username = (const char *)opts->username;
    pconn_opts->password = (const char *)opts->password;
    if (!rt_os_strncmp(opts->rt_channel, "YUNBA", 5)) {
        MSG_PRINTF(LOG_WARN, "connecting yunba mqtt server ...\n");
        pconn_opts->MQTTVersion = 0x13;
    } else if (!rt_os_strncmp(opts->rt_channel, "EMQ", 3)) {
        MSG_PRINTF(LOG_WARN, "connecting emq mqtt server ...\n");
        pconn_opts->MQTTVersion = MQTTVERSION_3_1;
    }
    pconn_opts->keepAliveInterval   = MQTT_KEEP_ALIVE_INTERVAL;
    pconn_opts->reliable            = 0;
    pconn_opts->cleansession        = 0;
    
    //MSG_PRINTF(LOG_DBG, "pconn_opts->struct_version=%d\n", pconn_opts->struct_version);
    if (my_connect(c, pconn_opts) == RT_FALSE) {
        MSG_PRINTF(LOG_WARN, "connecting %s mqtt server fail\r\n", opts->rt_channel);
        if (++opts->try_connect_timer > MAX_TRY_CONNECT_TIME) {
            if (!rt_os_strncmp(opts->rt_channel, "EMQ", 3)) {
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
    eid_check_upload();
    
    return RT_TRUE;
}

static void rt_mqtt_set_alias(const char *eid, const char *device_id, rt_bool init_flag)
{
    rt_os_memset(g_mqtt_param.alias, 0, sizeof(g_mqtt_param.alias));
    if (eid_check_memory(g_mqtt_eid, MAX_EID_LEN, '0')) {
        rt_os_memcpy(g_mqtt_param.alias, device_id, rt_os_strlen(device_id));
    } else {
        rt_os_memcpy(g_mqtt_param.alias, eid, rt_os_strlen(eid));
    }
    
    rt_os_memset(g_mqtt_param.opts.device_id, 0, sizeof(g_mqtt_param.opts.device_id));
    rt_os_memcpy(g_mqtt_param.opts.device_id, device_id, rt_os_strlen(device_id));
    
    if (init_flag) {
        rt_os_memset(g_mqtt_param.opts.client_id, 0, sizeof(g_mqtt_param.opts.client_id));
        rt_os_memcpy(g_mqtt_param.opts.rt_channel, "EMQ", 3);  // default for EMQ
        g_mqtt_param.alias_rc = 1;
    }
}

static void mqtt_process_task(void)
{
    int32_t rc;
    
    while(1) {        
        if (get_network_state() == NETWORK_STATE_INIT) {
            ;
        } else if (get_network_state() == NETWORK_CONNECTING) {
            if(g_mqtt_param.mqtt_flag == RT_FALSE) {
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
                if (rt_mqtt_connect_server(&g_mqtt_param) == RT_TRUE) {
                    /* set network using */
                    set_network_state(NETWORK_USING);
                    continue;
                } else {
                    /* If cache mqtt server and hardcode mqtt server all fail, and then connect ticket server to get mqtt server */
                    g_mqtt_param.mqtt_get_addr = RT_FALSE;
                }
            }
        } else if (get_network_state() == NETWORK_DIS_CONNECTED){
            if (g_mqtt_param.mqtt_flag == RT_TRUE) {
                MQTTClient_disconnect(g_mqtt_param.client, 0);
                MSG_PRINTF(LOG_DBG, "MQTTClient disconnect\n");
                g_mqtt_param.mqtt_flag      = RT_FALSE;
                g_mqtt_param.subscribe_flag = 0;  // reset subscribe flag
            }
        } else if (get_network_state() == NETWORK_USING) {
            //MSG_PRINTF(LOG_DBG, "alias:%s, channel:%s\n", g_mqtt_param.alias, g_mqtt_param.opts.rt_channel);
            if (!rt_os_strncmp(g_mqtt_param.opts.rt_channel, "YUNBA", 5)) {
                if (rt_os_strlen(g_mqtt_param.alias) && (g_mqtt_param.alias_rc == RT_TRUE)) {
                    g_mqtt_param.alias_rc = MQTTClient_set_alias(g_mqtt_param.client, (char *)g_mqtt_param.alias);
                    MSG_PRINTF(LOG_DBG, "MQTTClient set alias : %s\n", g_mqtt_param.alias);
                    if (g_mqtt_param.alias_rc != 0) {
                        MSG_PRINTF(LOG_WARN, "MQTTSetAlias error, g_mqtt_param.alias_rc=%d\r\n", g_mqtt_param.alias_rc);
                    }
                }
            }

            /* subscribe [cid/eid] */
            if ((GET_EID_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
                if(rt_os_strlen(g_mqtt_param.alias)) {
                    if (rt_os_strcmp((const char *)g_mqtt_param.alias, g_mqtt_device_id)) {
                        rc = MQTTClient_subscribe(g_mqtt_param.client, (const char *)g_mqtt_param.alias, 1);
                        if (rc == 0) {
                            MSG_PRINTF(LOG_DBG, "MQTTClient subscribe eid : %s OK !\n", g_mqtt_param.alias);
                            SET_EID_FLAG(g_mqtt_param.subscribe_flag);
                        } else {
                            MSG_PRINTF(LOG_WARN, "MQTTClient subscribe eid : %s error, ret=%d !\n", g_mqtt_param.alias, rc);
                            if (rc == MQTTCLIENT_DISCONNECTED) {
                                set_network_state(NETWORK_DIS_CONNECTED);
                                MSG_PRINTF(LOG_WARN, "mqtt disconnected !\r\n");
                                continue;
                            }
                        }
                    }
                } else {
                    MSG_PRINTF(LOG_WARN, "alias is error\n");
                }
            }

            /* subscribe device-id */
            if ((GET_DEVICE_ID_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
                rc = MQTTClient_subscribe(g_mqtt_param.client, g_mqtt_device_id, 1);
                if (rc == 0) {
                    MSG_PRINTF(LOG_DBG, "MQTTClient subscribe device id : %s OK !\n", g_mqtt_device_id);
                    SET_DEVICE_ID_FLAG(g_mqtt_param.subscribe_flag);
                } else {
                    MSG_PRINTF(LOG_DBG, "MQTTClient subscribe device id : %s error, ret=%d !\n", g_mqtt_device_id, rc);
                    if (rc == MQTTCLIENT_DISCONNECTED) {
                        set_network_state(NETWORK_DIS_CONNECTED);
                        MSG_PRINTF(LOG_WARN, "mqtt disconnected !\r\n");
                        continue;
                    }
                }
            }

            /* subscribe imei */
            if ((GET_IMEI_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
                rc = MQTTClient_subscribe(g_mqtt_param.client, g_mqtt_imei, 1);
                if (rc == 0) {
                    MSG_PRINTF(LOG_DBG, "MQTTClient subscribe imei : %s OK !\n", g_mqtt_imei);
                    SET_IMEI_FLAG(g_mqtt_param.subscribe_flag);
                } else {
                    MSG_PRINTF(LOG_DBG, "MQTTClient subscribe imei : %s error, ret=%d !\n", g_mqtt_imei, rc);
                    if (rc == MQTTCLIENT_DISCONNECTED) {
                        set_network_state(NETWORK_DIS_CONNECTED);
                        MSG_PRINTF(LOG_WARN, "mqtt disconnected !\r\n");
                        continue;
                    }
                }
            }

            /* subscribe agent  */
            if ((GET_AGENT_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
                rc = MQTTClient_subscribe(g_mqtt_param.client, AGENT_ALIAS, 1); 
                if (rc == 0) {
                    MSG_PRINTF(LOG_DBG, "MQTTClient subscribe %s OK !\n", AGENT_ALIAS);
                    SET_AGENT_FLAG(g_mqtt_param.subscribe_flag);
                } else {
                    MSG_PRINTF(LOG_DBG, "MQTTClient subscribe %s error, ret=%d !\n", AGENT_ALIAS, rc);
                    if (rc == MQTTCLIENT_DISCONNECTED) {
                        set_network_state(NETWORK_DIS_CONNECTED);
                        MSG_PRINTF(LOG_WARN, "mqtt disconnected !\r\n");
                        continue;
                    }
                }
            }
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
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

int8_t *rt_mqtt_get_channel(void)
{
    return g_mqtt_param.opts.rt_channel;
}

//init parameter
static void mqtt_init_param(void)
{
    g_mqtt_param.opts.nodelimiter           = 0;
    g_mqtt_param.opts.qos                   = 1;
    g_mqtt_param.opts.port                  = 0;
    g_mqtt_param.opts.showtopics            = 0;
    g_mqtt_param.opts.nodeName              = NULL;
    g_mqtt_param.opts.try_connect_timer     = 0;  // Initialize the connect timer
    g_mqtt_param.opts.last_connect_status   = MQTT_CONNECT_SUCCESS;  // Initialize the last link push the state of the system
    rt_mqtt_set_alias(g_mqtt_eid, g_mqtt_device_id, RT_TRUE);
    MQTTClient_init();
}

int32_t init_mqtt(void *arg)
{
    int32_t ret;
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    public_value_list->push_channel = (const char *)g_mqtt_param.opts.rt_channel;
    g_mqtt_eid = (const char *)public_value_list->card_info->eid;
    g_mqtt_device_id = (const char *)public_value_list->device_info->device_id;
    g_mqtt_imei = (const char *)public_value_list->device_info->imei;

    mqtt_init_param();

    ret = mqtt_create_task();
    if (ret) {
        goto exit_entry;
    }

exit_entry:

    return ret;
}

int32_t mqtt_connect_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    (void)buf;
    (void)len;

    //MSG_PRINTF(LOG_INFO, "mqtt connect event, mode: %d\r\n", mode);
    if (MSG_NETWORK_CONNECTED == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv network connected\r\n");
        set_network_state(NETWORK_CONNECTING);
    } else if (MSG_NETWORK_DISCONNECTED == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv network disconnected\r\n");
        set_network_state(NETWORK_DIS_CONNECTED);
    } else if (MSG_MQTT_SUBSCRIBE_EID == mode) {
        MSG_PRINTF(LOG_INFO, "mqtt module recv subcsribe eid request\r\n");
        /* set mqtt global alias */
        rt_mqtt_set_alias(g_mqtt_eid, g_mqtt_device_id, RT_FALSE);
        CLR_EID_FLAG(g_mqtt_param.subscribe_flag);
    }

    return RT_SUCCESS;
}

