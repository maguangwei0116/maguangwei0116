
#ifndef __RT_MQTT_H__
#define __RT_MQTT_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
#include "downstream.h"
#include "rt_type.h"
#include "config.h"
#include "cJSON.h"
#include "rt_mqtt_common.h"

#if (CFG_UPLOAD_HTTPS_ENABLE)
    #include "https.h"
#endif

#define MQTT_ALIAS_MAX_LEN              40
#define MAX_CONNECT_SERVER_TIMER        3
#define MQTT_PUBLISH_NO_YUNBA           -88
#define MQTT_PUBLISH_NO_CONNECTED       -89

/* switch for EMQ MQTTS */
// #define CFG_EMQ_MQTTS_ENABLE                0

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

/* different from mqtt_reg_info_t */
typedef struct MQTT_REG_URL {
    /* the url address of MQTT server */
    char        url[100];

    /* the port of MQTT server */
    int32_t     port;
} mqtt_reg_url_t;

typedef struct REG_INFO {
    /* in MQTT v3.1,If the Client ID contains more than 23 characters, the server responds to
     * the CONNECT message with a CONNACK return code 2: Identifier Rejected.
     * */
    char        client_id[200];
    
    /* in MQTT v3.1, it is recommended that passwords are kept to 12 characters or fewer, but
     * it is not required. */
    char        username[200];
    
    /*in MQTT v3.1, It is recommended that passwords are kept to 12 characters or fewer, but
     * it is not required. */
    char        password[200];
    
    /* user define it, and change size of device id. */
    char        device_id[200];

    /* the url address of MQTT server */
    char        url[100];

    /* the name mqtt channel, EMQ or YUNBA */
    char        channel[10];

    /* the address of ticket server */
    char        ticket_server[200];
} mqtt_reg_info_t;

typedef struct MQTT_OPTS {
    int32_t                     nodelimiter;
    int32_t                     qos;
    int32_t                     port;
    int32_t                     showtopics;
    int8_t                      try_connect_timer;
    int8_t                      last_connect_status;    // The last connected push system state
    char *                      node_name;
    char                        username[200];
    char                        password[200];
    char                        client_id[200];
    char                        device_id[200];    
    char                        url[100];               // the address of MQTT server
    char                        channel[10];            // the data channel of OTI upload
    char                        ticket_server[200];     // the ticket server addr
} mqtt_opts_t;

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

typedef struct MQTT_CHANNEL_OBJ {
    const char *        name;
    const char *        cb;      
} mqtt_channel_obj_t;

#define MQTT_CHANNEL_OBJ_INIT(name, topic, packer)\
    static const mqtt_channel_obj_t mqtt_channel_##name##_obj \
    __attribute__((section(".mqtt.channel.init.obj"))) __attribute__((__used__)) = \
    {#name, topic, packer}

DLLExport int32_t init_mqtt(void *arg);

DLLExport int32_t mqtt_connect_event(const uint8_t *buf, int32_t len, int32_t mode);

DLLExport int32_t mqtt_pulish_msg(const void* data, int32_t data_len);

typedef int32_t (*PCALLBACK)(const char *json_data);

DLLExport int32_t MQTTClient_setup_with_appkey_and_deviceid(const char* appkey, const char *deviceid, mqtt_opts_t *opts);

DLLExport int32_t MQTTClient_get_host(const char *node_name, char *url, const char *appkey);

DLLExport int32_t MQTTClient_setup_with_appkey(const char* appkey, mqtt_opts_t *opts);

DLLExport int32_t mqtt_adapter_setup_with_appkey(const char *appkey, mqtt_opts_t *opts, const char *eid);

DLLExport int32_t mqtt_http_post_json(const char *json_data, const char *hostname, uint16_t port, const char *path, PCALLBACK cb);

DLLExport mqtt_reg_info_t * mqtt_get_reg_info(void);

DLLExport void mqtt_set_reg_url(const char url[20], int32_t port);

DLLExport mqtt_reg_url_t * mqtt_get_reg_url(void);

#endif  // __RT_MQTT_H__

