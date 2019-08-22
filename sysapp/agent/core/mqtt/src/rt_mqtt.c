/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_yunba.c
 * Date        : 2017.09.01
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include "errno.h"
#include "rt_mqtt.h"
#include "rt_mqtt_common.h"
#include "dial_up.h"
#include "rt_os.h"
//#include "upload.h"
//#include "message_process.h"
#include "cJSON.h"
#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
#include "agent_config.h"
#include "config.h"

typedef enum NETWORK_STATE {
    NETWORK_STATE_INIT = 0,
    NETWORK_GET_IP,
    NETWORK_CONNECTING,
    NETWORK_DIS_CONNECTED,
    NETWORK_USING
} network_state_info;

typedef enum UP_COMMAND {
    COMMAND_INIT = 0,
    REGISTER_PUSH_ID,
    PROFILE_RELOADED,
    MCC_CHANGED,
    DEVICE_REBOOTED,
    INTERNET_RECONNECTED,
    ON_PUSH_ACTIVATION_CODE,
    ON_ENABLE_PROFILE,
    ON_DISABLE_PROFILE,
    ON_DELETE_PROFILE,
    ON_EUICC_LOOKUP,
    ON_SWITCH_COS,
    ON_LOG,
    ON_UPGRADE,
    ON_CONFIG,
    UN_KNOWN
} up_command_e;

typedef enum ESIM_STATE {
    ESIM_STATE_INIT = 0,
    ESIM_STATE_INSERTED,
    ESIM_STATE_REMOVED,
    ESIM_STATE_ERROR,
} esim_state_info_e;

typedef enum BOOT_STATE {
    BOOT_STATE_INIT = 0,
    BOOT_STATE_NO_CONFIG,
    BOOT_STATE_NO_NET,
    BOOT_STATE_NO_CARD,
    BOOT_STATE_SUCCESS,
    BOOT_STRAP_PROCESS
} boot_state_info_e;


/************************************debug***********************************/

/************************************fallback***********************************/
#define DEFAULT_DIS_CONNECT_WAIT_TIME           100  // Ĭ��fallbackΪ5��
// #define DEFAULT_SEED_CARD_FIRST                 0  // Ĭ�ϲ������ӿ�����

/************************************general***********************************/
#define DEFAULT_OTI_ENVIRONMENT_ADDR            "52.220.34.227"  // Ĭ����������
#define DEFAULT_OTI_ENVIRONMENT_PORT            7082
#define DEFAULT_EMQ_SERVER_ADDR                 "18.136.190.97"  // Ĭ����������EMQ��ַ
#define DEFAULT_PROXY_SERVER_ADDR               "smdp.redtea.io"  //Ĭ����������smdp��ַ
#define DEFAULT_CARD_TYPE_FLAG                  1  // �Ƿ�����/data/card_type�ļ�
#define DEFAULT_MBN_CONFIGURATION               1  // Ĭ�Ͽ���MBN���ù���
#define DEFAULT_LOG_FILE_SIZE                   1  // Ĭ��log��СΪ1M
#define DEFAULT_MBN_CONFIGURATION               1  // Ĭ�Ͽ���MBN���ù���
#define DEFAULT_INIT_PROFILE_TYPE               2  // Ĭ��������һ�ſ�
#define DEFAULT_RPLMN_ENABLE                    1  //Ĭ�Ͽ���rplmn���ù���

/********************************platform**************************************/


#define ADAPTER_APPKEY  "D358134D21684E8FA23CC25740506A82"

#define ADAPTER_PORT    7082


#define YUNBA_APPKEY       "596882bf2a9275716fe3c1e2"  // APPKET of YUNBA server
#define YUNBA_SERVER_IP    "23.91.101.68"  // the IP address of yunba server
#define YUNBA_URL          "tcp://23.91.101.68:1883"
#define YUNBA_SERVER_PORT 8085  // the port of yunba server
#define YUNBA_SERVER_PORT2 8383 


#define EMQ_APPKEY    "12345"  // APPKET of EMQ server

#define EMQ_SERVER_PORT    8085  // the port of EMQ server

static rt_bool mqtt_get_addr = RT_FALSE;  // if get mqtt server addr
static int8_t alias[40] = {'F'};
static int32_t mqtt_flag = 0;  // que ren buffer da xiao
static int8_t subscribe_flag = 0;
static int8_t rc = 1;
/*
* the data used to indicate topic subscribe
*bit0 topic of agent
*bit1 topid of CID
*/
static mqtt_info opts;  // mqtt connect info
static MQTTClient client;
static MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

//TODO:
network_state_info get_network_state(void)
{
    //return g_network_state;
    return 0;
}

void set_network_state(network_state_info state)
{
    //MSG_PRINTF(LOG_INFO("set network state: %d\r\n", state);
    //g_network_state = state;
}

boot_state_info_e get_boot_flag(void)
{
    //return g_boot_flag;
    return 1;
}

void msg_parse(int8_t *message, int32_t len)
{
}

/*****************************************************************************
 * FUNCTION
 *  msg_string_to_int
 * DESCRIPTION
 *  input string output int value.
 * PARAMETERS
 *  str
 * RETURNS
 *  int
 *****************************************************************************/
uint32_t msg_string_to_int(uint8_t* str)
{
    uint32_t length = 0;
    if (str == NULL) {
        MSG_PRINTF(LOG_WARN, "The string is error\n");
        return 0;
    }
    while (*str != '\0') {
        if ((*str >= '0') && (*str <= '9')) {
            length = length * 10 + *str - '0';
        }
        str++;
    }
    return length;
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


/************************************************callback***************************************************/

//Registe callback of message arrive
static int message_arrived(void* context, char* topicName, int32_t topicLen, MQTTClient_message* md)
{
    
#if PLATFORM == PLATFORM_FIBCOM
    if (!strcmp(fibo_thread_GetMyName(),"unknown"))
    {
        fibo_thread_InitFibofwkThreadData("sdk_theard");
    }
#endif
    //parse JSON
    MSG_PRINTF(LOG_DBG, "topicName:%s\n",topicName);
    msg_parse(md->payload,(int32_t) md->payloadlen);
    MQTTClient_freeMessage(&md);
    MQTTClient_free(topicName);
    return 1;
}

//Registe callback of extra message arrive
static int ext_message_arrive(void *context,EXTED_CMD cmd, int32_t status, int32_t ret_string_len, int8_t *ret_string)
{
    MSG_PRINTF(LOG_WARN, "%s, cmd:%d, status:%d, payload: %.*s\n", __func__, cmd, status, ret_string_len, ret_string);
    return 0;
}


static rt_bool my_connect(MQTTClient* client, MQTTClient_connectOptions* opts)
{
    int32_t c = 0;

    if ((c = MQTTClient_connect(*client,opts)) == 0) {
        mqtt_flag = 1;
        return RT_TRUE;
    } else {
        MSG_PRINTF(LOG_WARN, "Failed to connect error:%d\n",c);
        return RT_FALSE;
    }
}

static void connection_lost(void *context, char *cause)
{
    if (my_connect(&client, &conn_opts) != RT_TRUE) {
        mqtt_flag = 0;
        if (get_network_state() == NETWORK_USING) {
            set_network_state(NETWORK_GET_IP);
        }
    }
    MSG_PRINTF(LOG_WARN, "%s, %s\r\n",(char *)context, cause);

}

/**********************************************************************************************************/

/******************************************ticket setver procedd
**********************************************************/

//���ػ���֮ǰ�Ĵ�adapter��ȡ��ticket server
static rt_bool save_ticket_server()
{
    cJSON   *obj = NULL;
    int8_t  *save_info = NULL;
    int8_t  data_len[2];
    int16_t length = 0;
    if ((obj = cJSON_CreateObject()) == NULL) {
        MSG_PRINTF(LOG_WARN, "cJSON_CreateObject error\n");
        return RT_FALSE;
    }

    cJSON_AddItemToObject(obj,"channel",cJSON_CreateString(opts.rt_channel));
    cJSON_AddItemToObject(obj,"ticketServer",cJSON_CreateString(opts.ticket_server));
    save_info = (int8_t *)cJSON_PrintUnformatted(obj);
    length = rt_os_strlen(save_info);

    data_len[0] = (length >> 8) & 0xff;
    data_len[1] = length & 0xff;

    if (rt_create_file(TICKET_SERVER_CACHE) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_create_file  error\n");
        return RT_FALSE;
    }

    if (rt_write_data(TICKET_SERVER_CACHE,0,data_len,2) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_write_data data_len error\n");
        return RT_FALSE;
    }

    if (rt_write_data(TICKET_SERVER_CACHE,2,save_info,rt_os_strlen(save_info)) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_write_data TICKET_SERVER_CACHE error\n");
        return RT_FALSE;
    }

    rt_os_free(save_info);
    cJSON_Delete(obj);
    return RT_TRUE;
}

//��ȡ���ػ���Ĵ�adapter��ȡ��ticket server
static rt_bool get_ticket_server()
{
    int8_t  *save_info;
    int8_t data_len[2];
    int16_t length = 0;
    cJSON   *obj = NULL;
    cJSON   *channel = NULL;
    cJSON   *ticket_server = NULL;


    if (rt_read_data(TICKET_SERVER_CACHE, 0, data_len, 2) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_read_data data_len error\n");
        return RT_FALSE;
    }

    length = (data_len[0] << 8) | data_len[1];

    if ((save_info = (int8_t *)rt_os_malloc(length)) == NULL) {
        MSG_PRINTF(LOG_ERR, "rt_os_malloc error\n");
        return RT_FALSE;
    }

    if (rt_read_data(TICKET_SERVER_CACHE,2, save_info, length) == RT_ERROR) {
        MSG_PRINTF(LOG_WARN, "rt_read_data save_info error\n");
        return RT_FALSE;
    }

    if ((obj = cJSON_Parse(save_info)) == NULL) {
        MSG_PRINTF(LOG_WARN, "cJSON_Parse error\n");
        return RT_FALSE;
    }
    channel = cJSON_GetObjectItem(obj, "channel");
    ticket_server = cJSON_GetObjectItem(obj, "ticketServer");

    if ((channel == NULL) || (ticket_server == NULL)) {
        MSG_PRINTF(LOG_WARN, "channel or ticket_server is NULL\n");
        return RT_FALSE;
    }

    rt_os_memset(opts.rt_channel, 0, sizeof(opts.rt_channel));
    rt_os_memcpy(opts.rt_channel, channel->valuestring, rt_os_strlen(channel->valuestring));
    rt_os_memset(opts.ticket_server, 0, sizeof(opts.ticket_server));
    rt_os_memcpy(opts.ticket_server, ticket_server->valuestring, rt_os_strlen(ticket_server->valuestring));

    rt_os_free(save_info);
    cJSON_Delete(obj);
    return RT_TRUE;


}
/**************************************************************************************************************/
static rt_bool rt_mqtt_connect_adapter(MQTTClient *c)
{
    uint8_t num = 0;

    set_reg_url(OTI_ENVIRONMENT_ADDR,ADAPTER_PORT);

    //�������Ǻ���Լ���ticket server adopter,��ೢ��3��
    do {
        if (rt_mqtt_setup_with_appkey(ADAPTER_APPKEY, &opts) == 0) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "rt_mqtt_setup_with_appkey num:%d\n", num++);
        rt_os_sleep(1);
    } while((num != MAX_CONNECT_SERVER_TIMER) && (get_network_state() != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (get_network_state() == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect_adapter error\n");
        return RT_FALSE;
    }

    if (rt_os_strncmp(opts.rt_channel, "EMQ", 3) == 0) {
        rt_os_memset(opts.client_id, 0, sizeof(opts.client_id));
        rt_os_memcpy(opts.client_id, alias, rt_os_strlen(alias));
        opts.client_id[rt_os_strlen(alias)] = '\0';
    }

    //�����ȡ����ticket server
    save_ticket_server();
    return RT_TRUE;
}


//connect yunba server
static rt_bool rt_mqtt_connect_yunba(MQTTClient *c,int8_t *ticket_server)
{
    uint8_t num = 0;

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
    }

    //����yunba server ��ೢ��3��
    do {
        if (MQTTClient_setup_with_appkey_and_deviceid(YUNBA_APPKEY, (char *)opts.device_id, &opts) == 0) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "MQTTClient_setup_with_appkey_and_deviceid num:%d\n", num++);
        rt_os_sleep(1);
    } while((num != MAX_CONNECT_SERVER_TIMER) && (get_network_state() != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (get_network_state() == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect_yunba error\n");
        return RT_FALSE;
    }

    rt_os_memset(opts.rt_channel, 0, sizeof(opts.rt_channel));
    rt_os_memcpy(opts.rt_channel, "YUNBA", 6);
    rt_os_memset(opts.rt_url, 0, sizeof(opts.rt_url));
    rt_os_memcpy(opts.rt_url, YUNBA_URL, rt_os_strlen(YUNBA_URL));
    opts.rt_url[rt_os_strlen(YUNBA_URL)] = '\0';
    return RT_TRUE;
}


//connect EMQ server
static rt_bool rt_mqtt_connect_emq(MQTTClient *c,int8_t *ticket_server)
{
    uint8_t num = 0;


    if (ticket_server == NULL) {
        set_reg_url(EMQ_SERVER_ADDR, EMQ_SERVER_PORT);
    } else {
        int8_t addr[20];
        int32_t port;
        if (mqtt_get_ip_pair(ticket_server, addr, &port) == RT_TRUE) {
            set_reg_url((char *)addr,port);
        } else {
            MSG_PRINTF(LOG_WARN, "mqtt_get_ip_pair error ticket_serverL:%s\n",ticket_server);
            return RT_FALSE;
        }
    }

    do {
        if ((MQTTClient_setup_with_appkey(EMQ_APPKEY, &opts) == 0) && 
           (MQTTClient_get_host((char *)opts.nodeName, (char *)opts.rt_url, EMQ_APPKEY) == 0)) {
            break;
        }
        MSG_PRINTF(LOG_DBG, "MQTTClient_setup_with_appkey num:%d\n", num++);
        rt_os_sleep(1);
    } while((num != MAX_CONNECT_SERVER_TIMER) && (get_network_state() != NETWORK_DIS_CONNECTED));

    if ((num == MAX_CONNECT_SERVER_TIMER) || (get_network_state() == NETWORK_DIS_CONNECTED)) {
        MSG_PRINTF(LOG_WARN, "rt_mqtt_connect_emq error\n");
        return RT_FALSE;
    }

    rt_os_memset(opts.rt_channel, 0, sizeof(opts.rt_channel));
    rt_os_memcpy(opts.rt_channel, "EMQ", 4);
    rt_os_memset(opts.client_id, 0, sizeof(opts.client_id));
    rt_os_memcpy(opts.client_id, alias, rt_os_strlen(alias));
    opts.client_id[rt_os_strlen(alias)] = '\0';
    return RT_TRUE;
}

/**********************************************************************************************************/

/*********************************************mqtt server process
*******************************************************/
//connect mqtt server

static rt_bool rt_mqtt_connect_server(MQTTClient *c)
{
    int32_t ret = 0;

    if (MQTTClient_create(c, (const char *)opts.rt_url, \
        (const char *)opts.client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL) != 0) {
        MSG_PRINTF(LOG_WARN, "MQTTClient_create error\n");
        return RT_FALSE;
    }

    ret = MQTTClient_setCallbacks(*c, NULL,(MQTTClient_connectionLost *)connection_lost, 
                                 (MQTTClient_messageArrived *)message_arrived, NULL,
                                 (MQTTClient_extendedCmdArrive *)ext_message_arrive);

    MSG_PRINTF(LOG_DBG, "MQTTClient_setCallbacks %d\n", ret);

    conn_opts.username = (const char *)opts.username;
    conn_opts.password = (const char *)opts.password;
    if (rt_os_strncmp(opts.rt_channel, "YUNBA", 5) == 0) {
        MSG_PRINTF(LOG_DBG, "connect yunba mqtt server\n");
        conn_opts.MQTTVersion = 0x13;
    } else if (rt_os_strncmp(opts.rt_channel, "EMQ", 3) == 0) {
        MSG_PRINTF(LOG_DBG, "connect emq mqtt server\n");
        conn_opts.MQTTVersion = MQTTVERSION_3_1;
    }
    conn_opts.keepAliveInterval = 300;
    conn_opts.reliable = 0;
    conn_opts.cleansession = 0;

    if (my_connect(c, &conn_opts) == RT_FALSE) {
        if (++opts.try_connect_timer > MAX_TRY_CONNECT_TIME) {
            if (rt_os_strncmp(opts.rt_channel, "EMQ", 3) == 0) {
                opts.last_connect_status = MQTT_CONNECT_EMQ_ERROR;
            } else {
                opts.last_connect_status = MQTT_CONNECT_YUNBA_ERROR;
            }
        }
        return RT_FALSE;
    }

    sleep(10);

    opts.last_connect_status = MQTT_CONNECT_SUCCESS;
    opts.try_connect_timer = 0;
    rc = 1;
    return RT_TRUE;
}

/**********************************************************************************************************/

//init parameter
static void rt_mqtt_init(void)
{
    opts.nodelimiter = 0;
    opts.qos = 1;
    opts.port = 0;
    opts.showtopics = 0;
    opts.nodeName = NULL;
    opts.try_connect_timer = 0;  // Initialize the connect timer
    opts.last_connect_status = 0;  // Initialize the last link push the state of the system
    MQTTClient_init();
}

//used to get mqtt server addr
static rt_bool mqtt_get_server_addr()
{
    //attemp to connect adapter
    do{
        if (USE_ADAPTER_SERVER){
            if ((get_network_state() != NETWORK_DIS_CONNECTED) &&
                  (rt_mqtt_connect_adapter(&client) == RT_TRUE)) {
                MSG_PRINTF(LOG_DBG, "connect adapter ticket server to get mqtt server address EMQ or YUNBA\n");
                break;
            }
            if (get_ticket_server() == RT_TRUE) {

            //���֮ǰ���ӵ���yunba��ticket server����ô��������
                if ((get_network_state() != NETWORK_DIS_CONNECTED) &&
                      (rt_os_strncmp(opts.rt_channel, "YUNBA", 5) == 0) &&
                      (rt_mqtt_connect_yunba(&client, opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "connect YUNBA mqtt server successfull\n");
                    break;
                }

                //���֮ǰ���ӵ���EMQ��ticket server����ô��������
                if ((get_network_state() != NETWORK_DIS_CONNECTED) &&
                      (rt_os_strncmp(opts.rt_channel, "EMQ", 3) == 0) &&
                      (rt_mqtt_connect_emq(&client, opts.ticket_server) == RT_TRUE)) {
                    MSG_PRINTF(LOG_DBG, "connect EMQ mqtt server successfull\n");
                    break;
                }
            }
        }

        //����adapter �� ϵͳ�����ticket server���޷�ʹ�ã��ñ���д����ticket server��ַ��������

        if ((get_network_state() != NETWORK_DIS_CONNECTED) &&
              (0 == rt_os_strncmp(opts.rt_channel, "YUNBA", 5))) {
            if (rt_mqtt_connect_emq(&client,NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "connect EMQ mqtt server successfull\n");
                break;
            }
        } else if ((get_network_state() != NETWORK_DIS_CONNECTED) &&
                         (0 == rt_os_strncmp(opts.rt_channel, "EMQ", 3))) {
            if (rt_mqtt_connect_yunba(&client,NULL) == RT_TRUE) {
                MSG_PRINTF(LOG_DBG, "connect yunba mqtt server successfull\n");
                break;
            }
        }
    }while(get_network_state() != NETWORK_DIS_CONNECTED);

    if (get_network_state() == NETWORK_DIS_CONNECTED) {
        return RT_FALSE;
    } else {
        mqtt_get_addr = RT_TRUE;
        return RT_TRUE;
    }

}

//yunba task
static void rt_mqtt_task_process(void)
{
    int8_t subscribe_flag = 0;

    //yun ba buffer
    rt_mqtt_init();

    while(1) {
        if (get_network_state() == NETWORK_CONNECTING) {
            if(mqtt_flag == 0) {

                //��������Ѿ�������mqtt��ַ���ҵ�ַ���ã��Ͳ���ȥticket server��ȡmqtt��ַ
                if (mqtt_get_addr == RT_FALSE) {

                    //���������������mqtt����������Ҫ���½��ж���
                    if (mqtt_get_server_addr() == RT_FALSE) {
                        continue;
                    }

                }

                //����mqtt������������ǰ�Ѿ��ӱ��ػ������ticket server adapter ��ȡmqtt��ַ��
                if (rt_mqtt_connect_server(&client) == RT_TRUE) {
                    if (get_boot_flag() != BOOT_STRAP_PROCESS) {
                        //upload_set_values(REGISTER_PUSH_ID, NULL);
                    }

                    //�ɹ����ӷ�����������״̬��Ϊ������
                    set_network_state(NETWORK_USING);
                } else {

                    //������ػ����mqtt server��hardcode��mqtt server���޷�ʹ�ã���ô����ȥ��ticket server��ȡһ��mqtt server
                    mqtt_get_addr = RT_FALSE;
                }
            }
        }
        else if ((get_network_state() == NETWORK_DIS_CONNECTED) || 
                (get_network_state() == NETWORK_GET_IP)) {
            if (mqtt_flag == 1) {
                MQTTClient_disconnect(client,0);
                MSG_PRINTF(LOG_DBG, "MQTTClient_disconnect\n");
                mqtt_flag = 0;
                subscribe_flag = 0;  // ��λ���ı�־  
            }
        }
        else if (get_network_state() == NETWORK_USING) {
            if (rt_os_strncmp(opts.rt_channel, "YUNBA", 5) == 0) {
                if ((alias != NULL) && (rc == 1)) {
                    rc = MQTTClient_set_alias(client, (char *)alias);
                    MSG_PRINTF(LOG_DBG, "MQTTClient_set_alias : %s\n", alias);
                    if (rc != 0) {
                        MSG_PRINTF(LOG_WARN, "MQTTSetAlias error\n");
                    }
                }
            }

            if ((GET_CID_FLAG(subscribe_flag) != 1) || 
                (GET_AGENT_FLAG(subscribe_flag) != 1)) {
                if(alias != NULL) {

                    // ���alias��δ���ģ���ô����alias
                    if ((GET_CID_FLAG(subscribe_flag) != 1) && (MQTTClient_subscribe(client, (const char *)alias, 1) == 0)) {
                        MSG_PRINTF(LOG_DBG, "MQTTClient_subscribe : %s\n", alias);
                        SET_CID_FLAG(subscribe_flag);
                    } else {
                        MSG_PRINTF(LOG_WARN, "MQTTClient_subscribe %s error\n", alias);
                    }
                } else {
                    MSG_PRINTF(LOG_WARN, "alias is error\n");
                }

                //���agent��topic��δ���ģ�����agent
                if ((GET_AGENT_FLAG(subscribe_flag) != 1) && (MQTTClient_subscribe(client, "agent", 1) == 0)) {
                    SET_AGENT_FLAG(subscribe_flag);
                } else {
                    MSG_PRINTF(LOG_WARN, "MQTTClient_subscribe agent error\n");
                }
            }
            
        }
        rt_os_sleep(1);
    }
    MSG_PRINTF(LOG_DBG, "exit rt_mqtt_task\n");
    MQTTClient_destroy(&client);
}


/**********************************************External interface
************************************************************/

// creat yunba task to connect yunba and receive data.
int32_t rt_mqtt_task(void)
{
    int32_t ret = RT_ERROR;
    rt_task id_connect;
    ret = rt_create_task(&id_connect,(void *) rt_mqtt_task_process, NULL);
    if (ret == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "creat pthread error\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

void rt_mqtt_set_alias(int8_t *obj)
{
    rt_os_memset(alias, 0, sizeof(alias));
    rt_os_memcpy(alias, obj, rt_os_strlen(obj));
    rt_os_memset(opts.device_id, 0,sizeof(opts.device_id));
    rt_os_memcpy(opts.device_id,obj, rt_os_strlen(obj));
    rt_os_memset(opts.client_id, 0, sizeof(opts.client_id));
    rt_os_memcpy(opts.rt_channel, "YUNBA", 6);  // ����Ĭ�����ӵ�ַΪ�ư�
    rc = 1;  // �������ñ���
}

int8_t *rt_mqtt_get_channel(void)
{
    return opts.rt_channel;
}

/**********************************************************************************************************/