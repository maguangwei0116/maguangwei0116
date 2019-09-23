/*
 * rt_mqtt_comman.h
 *
 *  Created on: Nov 16, 2015
 *      Author: yunba
 */

#ifndef __RT_MQTT_COMMON_H__
#define __RT_MQTT_COMMON_H__

#if defined(WIN32) || defined(WIN64)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
  #define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#else
  #define DLLImport extern
  #define DLLExport __attribute__ ((visibility ("default")))
#endif

#if defined(WIN32) || defined(WIN64)
typedef int ssize_t;
#endif

#include "rt_mqtt.h"
#include "stdint.h"

/**
 * Default MQTT version to connect with.  Use 3.1.1 then fall back to 3.1
 */
#define MQTTVERSION_DEFAULT 0
/**
 * MQTT version to connect with: 3.1
 */
#define MQTTVERSION_3_1 3
/**
 * MQTT version to connect with: 3.1.1
 */
#define MQTTVERSION_3_1_1 4
/**
 * Yunba MQTT version to connect with: 3.1.0
 */
#define MQTTVERSION_YUNBA_3_1 0x13
typedef enum {
    GET_ALIAS =1,
    GET_ALIAS_ACK,
    GET_TOPIC,
    GET_TOPIC_ACK,
    GET_ALIAS_LIST,
    GET_ALIAS_LIST_ACK,
    PUBLISH2,
    PUBLISH2_ACK,
    GET_STATUS = 9,
    GET_STATUS_ACK,
    GET_TOPIC_LIST2 = 13,
    GET_TOPIC_LIST2_ACK,
    GET_ALIASLIST2,
    GET_ALIASLIST2_ACK,
    GET_STATUS2 = 19,
    GET_STATUS2_ACK = 20
} EXTED_CMD;

enum {
    PUBLISH2_TLV_TOPIC,
    PUBLISH2_TLV_PAYLOAD,
    PUBLISH2_TLV_PLAT,
    PUBLISH2_TLV_TTL,
    PUBLISH2_TLV_TIME_DELAY,
    PUBLISH2_TLV_LOCATION,
    PUBLISH2_TLV_QOS,
    PUBLISH2_TLV_APN_JSON,
    PUBLISH2_TLV_MAX_NUM
};

typedef int (*PCALLBACK)(char *p);
int http_post_json(char *json_data, char *hostname, uint16_t port, char *path, PCALLBACK cb);

DLLExport void set_reg_url(const char url[20], int port);
DLLExport void set_reg_url_v2(const char url[20], int port);

typedef struct {
    /* in MQTT v3.1,If the Client ID contains more than 23 characters, the server responds to
     * the CONNECT message with a CONNACK return code 2: Identifier Rejected.
     * */
    char client_id[200];
    /* in MQTT v3.1, it is recommended that passwords are kept to 12 characters or fewer, but
     * it is not required. */
    char username[200];
    /*in MQTT v3.1, It is recommended that passwords are kept to 12 characters or fewer, but
     * it is not required. */
    char password[200];
    /* user define it, and change size of device id. */
    char device_id[200];

    char rt_url[100];//the address of MQTT server

    char rt_channel[10];

    char ticket_server[200];
 } REG_info;

typedef struct MQTT_INFO
{
#define MAX_TRY_CONNECT_TIME    3
    int32_t     nodelimiter;
    int32_t     qos;
    int8_t      username[200];
    int8_t      password[200];
    int8_t      client_id[200];
    int8_t      device_id[200];
    int32_t     port;
    int32_t     showtopics;
    int8_t      *nodeName;
    int8_t      rt_url[100];//the address of MQTT server
    int8_t      rt_channel[10];//the data channel of OTI upload
    int8_t      ticket_server[200];
    int8_t      try_connect_timer;
    int32_t     last_connect_status;  // The last connected push system state
}mqtt_info;

DLLExport int MQTTClient_setup_with_appkey_v2(char* appkey, REG_info *info);

DLLExport int MQTTClient_setup_with_appkey_and_deviceid(const char* appkey, const char *deviceid, mqtt_info *info);

DLLExport int MQTTClient_setup_with_appkey_and_deviceid_v2(char* appkey, char *deviceid, REG_info *info);

DLLExport int MQTTClient_get_host(char *nodeName, char *url, const char *appkey);
DLLExport int MQTTClient_get_host_v2(char *appkey, char* url);
DLLExport int rt_mqtt_setup_with_appkey(const char *appkey, mqtt_info *info, const char *eid);
DLLExport int MQTTClient_setup_with_appkey(char* appkey, mqtt_info *info);



#endif /* __RT_MQTT_COMMON_H__ */

