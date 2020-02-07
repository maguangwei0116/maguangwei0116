
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

#include "stdint.h"

/**
 * Default MQTT version to connect with.  Use 3.1.1 then fall back to 3.1
 */
#define MQTTVERSION_DEFAULT         0

/**
 * MQTT version to connect with: 3.1
 */
#define MQTTVERSION_3_1             3

/**
 * MQTT version to connect with: 3.1.1
 */
#define MQTTVERSION_3_1_1           4

/**
 * Yunba MQTT version to connect with: 3.1.0
 */
#define MQTTVERSION_YUNBA_3_1       0x13

/**
 * max retry connect time
 */
#define MAX_TRY_CONNECT_TIME        3

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

typedef enum PUBLISH2_TLV {
    PUBLISH2_TLV_TOPIC,
    PUBLISH2_TLV_PAYLOAD,
    PUBLISH2_TLV_PLAT,
    PUBLISH2_TLV_TTL,
    PUBLISH2_TLV_TIME_DELAY,
    PUBLISH2_TLV_LOCATION,
    PUBLISH2_TLV_QOS,
    PUBLISH2_TLV_APN_JSON,
    PUBLISH2_TLV_MAX_NUM
} PUBLISH2_TLV_e;

#endif /* __RT_MQTT_COMMON_H__ */

