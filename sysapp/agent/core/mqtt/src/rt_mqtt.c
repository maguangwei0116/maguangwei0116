
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

typedef enum BOOT_STATE {
    BOOT_STATE_INIT = 0,
    BOOT_STATE_NO_CONFIG,
    BOOT_STATE_NO_NET,
    BOOT_STATE_NO_CARD,
    BOOT_STATE_SUCCESS,
    BOOT_STRAP_PROCESS
} boot_state_info_e;

typedef struct _mqtt_param_t {
    MQTTClient_connectOptions   conn_opts;
    mqtt_info                   opts;
    MQTTClient                  client;    
    network_state_info_e        state;
    char                        alias[40];    
    rt_bool                     mqtt_get_addr;
    rt_bool                     mqtt_flag;          // mqtt connect ok falg
    rt_bool                     subscribe_flag;     // subscribe ok flag
    rt_bool                     alias_rc;           // need set alias flag                  
} mqtt_param_t;

static mqtt_param_t g_mqtt_param = {MQTTClient_connectOptions_initializer, 0};

#if 0
/*
* the data used to indicate topic subscribe
*bit0 topic of agent
*bit1 topid of CID
*/
static mqtt_info opts;  // mqtt connect info
static MQTTClient client;
static MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

static rt_bool mqtt_get_addr = RT_FALSE;  // if get mqtt server addr
static int8_t alias[40] = {'F'};
static int32_t mqtt_flag = 0;  // que ren buffer da xiao
static int8_t subscribe_flag = 0;
static int8_t rc = 1;
#endif

//TODO:
static g_network_state = NETWORK_CONNECTING;
network_state_info_e get_network_state(void)
{
    return g_network_state;
}

void set_network_state(network_state_info_e state)
{
    //MSG_PRINTF(LOG_INFO("set network state: %d\r\n", state);
    g_network_state = state;
}

boot_state_info_e get_boot_flag(void)
{
    //return g_boot_flag;
    return 1;
}

void msg_parse(int8_t *message, int32_t len)
{
    MSG_PRINTF(LOG_WARN, "mqtt recv msg (%d bytes): %s\r\n", len, message);

    downstram_msg_parse((const char *)message);
}

//本地缓存之前的从adapter获取的ticket server
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
        rt_os_free(save_info);
    }
    if (obj) {
        cJSON_Delete(obj);
    }
    
    return ret;
}

//获取本地缓存的从adapter获取的ticket server
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

    set_reg_url(OTI_ENVIRONMENT_ADDR, ADAPTER_PORT);
    MSG_PRINTF(LOG_DBG, "OTI server addr:%s, port:%d\r\n", OTI_ENVIRONMENT_ADDR, ADAPTER_PORT);

    //连接我们红茶自己的ticket server adopter,最多尝试3次
    do {
        if (rt_mqtt_setup_with_appkey(ADAPTER_APPKEY, opts) == 0) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "rt_mqtt_setup_with_appkey num:%d\n", num++);
        rt_os_sleep(1);
    } while((num != MAX_CONNECT_SERVER_TIMER) && (get_network_state() != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (get_network_state() == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect adapter error\n");
        return RT_FALSE;
    }

    if (!rt_os_strncmp(opts->rt_channel, "EMQ", 3)) {
        snprintf(opts->client_id, sizeof(opts->client_id), "%s", alias);
    }

    //保存获取到的ticket server
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

    //连接yunba server 最多尝试3次
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

#if 1  // only for test
#define TEST_FORCE_TO_ADAPTER       0
#define TEST_FORCE_TO_EMQ           1
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
                //如果之前连接的是yunba的ticket server，那么尝试连接
                if (!rt_os_strncmp(param->opts.rt_channel, "YUNBA", 5) &&
                      (rt_mqtt_connect_yunba(param, param->opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "connect YUNBA mqtt server successfull\n");
                    break;
                }

                //如果之前连接的是EMQ的ticket server，那么尝试连接
                if (!rt_os_strncmp(param->opts.rt_channel, "EMQ", 3) &&
                      (rt_mqtt_connect_emq(param, param->opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "connect EMQ mqtt server successfull\n");
                    break;
                }
            }
        }

        //如果adapter 和 系统缓存的ticket server都无法使用，用本地写死的ticket server地址进行连接
        if (!rt_os_strncmp(param->opts.rt_channel, "YUNBA", 5)) {
FORCE_TO_EMQ:
            if (rt_mqtt_connect_emq(param, NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "connect EMQ mqtt server successfull\n");
                break;
            }
        } else if (!rt_os_strncmp(param->opts.rt_channel, "EMQ", 3)) {
FORCE_TO_YUNBA:
            if (rt_mqtt_connect_yunba(param, NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "connect yunba mqtt server successfull\n");
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
        MSG_PRINTF(LOG_WARN, "Connect mqtt ok !\n", c);
        return RT_TRUE;
    } else {
        MSG_PRINTF(LOG_WARN, "Failed to connect error:%d\n", c);
        return RT_FALSE;
    }
}

static void connection_lost(void *context, char *cause)
{
    MSG_PRINTF(LOG_WARN, "connection lost: %s, %s\r\n",(char *)context, cause);
    if (my_connect(&g_mqtt_param.client, &g_mqtt_param.conn_opts) != RT_TRUE) {
        g_mqtt_param.mqtt_flag = RT_FALSE;
        MSG_PRINTF(LOG_WARN, "connect again fail after connection lost !!!\r\n");
        
        //if (get_network_state() == NETWORK_USING) {
        //    set_network_state(NETWORK_GET_IP);
        //}
    } else {
        MSG_PRINTF(LOG_WARN, "connect again ok after connection lost !!!\r\n");
    }
}

//connect mqtt server
static rt_bool rt_mqtt_connect_server(mqtt_param_t *param)
{
    int32_t ret = 0;
    MQTTClient *c = &param->client;
    mqtt_info *opts = &param->opts;
    MQTTClient_connectOptions *pconn_opts = &param->conn_opts;

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

    MSG_PRINTF(LOG_DBG, "MQTTClient_setCallbacks %d\n", ret);

    pconn_opts->username = (const char *)opts->username;
    pconn_opts->password = (const char *)opts->password;
    if (!rt_os_strncmp(opts->rt_channel, "YUNBA", 5)) {
        MSG_PRINTF(LOG_DBG, "connecting yunba mqtt server ...\n");
        pconn_opts->MQTTVersion = 0x13;
    } else if (!rt_os_strncmp(opts->rt_channel, "EMQ", 3)) {
        MSG_PRINTF(LOG_DBG, "connecting emq mqtt server ...\n");
        pconn_opts->MQTTVersion = MQTTVERSION_3_1;
    }
    pconn_opts->keepAliveInterval   = 300;
    pconn_opts->reliable            = 0;
    pconn_opts->cleansession        = 0;
    //MSG_PRINTF(LOG_DBG, "pconn_opts->struct_version=%d\n", pconn_opts->struct_version);
    if (my_connect(c, pconn_opts) == RT_FALSE) {
        if (++opts->try_connect_timer > MAX_TRY_CONNECT_TIME) {
            if (!rt_os_strncmp(opts->rt_channel, "EMQ", 3)) {
                opts->last_connect_status = MQTT_CONNECT_EMQ_ERROR;
            } else {
                opts->last_connect_status = MQTT_CONNECT_YUNBA_ERROR;
            }
        }
        return RT_FALSE;
    }

    //rt_os_sleep(10);

    opts->last_connect_status = MQTT_CONNECT_SUCCESS;
    opts->try_connect_timer = 0;
    param->alias_rc = 1;
    
    return RT_TRUE;
}

#include "upload.h"
static void mqtt_process_task(void)
{
    rt_os_sleep(10);

    while(1) {
        if (get_network_state() == NETWORK_CONNECTING) {
            if(g_mqtt_param.mqtt_flag == RT_FALSE) {
                //如果本地已经缓存了mqtt地址，且地址可用，就不再去ticket server获取mqtt地址
                if (g_mqtt_param.mqtt_get_addr == RT_FALSE) {
                    //这边由于重新请求mqtt连接所以需要重新进行订阅
                    if (mqtt_get_server_addr(&g_mqtt_param) == RT_FALSE) {
                        continue;
                    }
                }

                //连接mqtt服务器，连接前已经从本地缓存或者ticket server adapter 获取mqtt地址。
                if (rt_mqtt_connect_server(&g_mqtt_param) == RT_TRUE) {
                    //if (get_boot_flag() != BOOT_STRAP_PROCESS) {
                        //upload_set_values(REGISTER_PUSH_ID, NULL);
                    //}

                    //成功连接服务器后将网络状态置为已连接
                    set_network_state(NETWORK_USING);
                } else {
                    //如果本地缓存的mqtt server和hardcode的mqtt server都无法使用，那么重新去获ticket server获取一下mqtt server
                    g_mqtt_param.mqtt_get_addr = RT_FALSE;
                }
            }
        } else if ((get_network_state() == NETWORK_DIS_CONNECTED) || 
                (get_network_state() == NETWORK_GET_IP)) {
            if (g_mqtt_param.mqtt_flag == RT_TRUE) {
                MQTTClient_disconnect(g_mqtt_param.client, 0);
                MSG_PRINTF(LOG_DBG, "MQTTClient disconnect\n");
                g_mqtt_param.mqtt_flag      = RT_FALSE;
                g_mqtt_param.subscribe_flag = RT_FALSE;  // 复位订阅标志  
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

            if ((GET_CID_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE) || 
                (GET_AGENT_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE)) {
                if(rt_os_strlen(g_mqtt_param.alias)) {
                    // 如果alias还未订阅，那么订阅alias
                    if ((GET_CID_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE) &&
                            (MQTTClient_subscribe(g_mqtt_param.client, (const char *)g_mqtt_param.alias, 1) == 0)) {
                        MSG_PRINTF(LOG_DBG, "MQTTClient subscribe : %s OK !\n", g_mqtt_param.alias);
                        SET_CID_FLAG(g_mqtt_param.subscribe_flag);
                    } else {
                        MSG_PRINTF(LOG_WARN, "MQTTClient subscribe %s error !\n", g_mqtt_param.alias);
                    }
                } else {
                    MSG_PRINTF(LOG_WARN, "alias is error\n");
                }

#if 0
                //如果agent的topic还未订阅，订阅agent
                if ((GET_AGENT_FLAG(g_mqtt_param.subscribe_flag) != RT_TRUE) && 
                        (MQTTClient_subscribe(g_mqtt_param.client, "agent", 1) == 0)) {
                    SET_AGENT_FLAG(g_mqtt_param.subscribe_flag);
                } else {
                    MSG_PRINTF(LOG_WARN, "MQTTClient_subscribe agent error\n");
                }
#endif
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

//init parameter
static void mqtt_init_param(void)
{
    g_mqtt_param.opts.nodelimiter           = 0;
    g_mqtt_param.opts.qos                   = 1;
    g_mqtt_param.opts.port                  = 0;
    g_mqtt_param.opts.showtopics            = 0;
    g_mqtt_param.opts.nodeName              = NULL;
    g_mqtt_param.opts.try_connect_timer     = 0;  // Initialize the connect timer
    g_mqtt_param.opts.last_connect_status   = 0;  // Initialize the last link push the state of the system
    rt_mqtt_set_alias("89086657727465610100000000000171");  // only for test
    MQTTClient_init();
}

int32_t init_mqtt(void *arg)
{
    int32_t ret;
    public_value_list_t *public_value_list = (public_value_list_t *)arg;
    
    public_value_list->push_channel = (const char *)g_mqtt_param.opts.rt_channel;

    mqtt_init_param();

    ret = mqtt_create_task();
    if (ret) {
        goto exit_entry;   
    }

exit_entry:

    return ret;
}

void rt_mqtt_set_alias(int8_t *obj)
{
    rt_os_memset(g_mqtt_param.alias, 0, sizeof(g_mqtt_param.alias));
    rt_os_memcpy(g_mqtt_param.alias, obj, rt_os_strlen(obj));
    rt_os_memset(g_mqtt_param.opts.device_id, 0,sizeof(g_mqtt_param.opts.device_id));
    rt_os_memcpy(g_mqtt_param.opts.device_id, obj, rt_os_strlen(obj));
    rt_os_memset(g_mqtt_param.opts.client_id, 0, sizeof(g_mqtt_param.opts.client_id));
    rt_os_memcpy(g_mqtt_param.opts.rt_channel, "EMQ", 3);  // default for EMQ
    g_mqtt_param.alias_rc = 1;
}

int8_t *rt_mqtt_get_channel(void)
{
    return g_mqtt_param.opts.rt_channel;
}

