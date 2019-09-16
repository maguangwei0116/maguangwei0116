
#ifndef __RT_MQTT_H__
#define __RT_MQTT_H__

#include "rt_type.h"
//#include "agent.h"

#define MQTT_CONNECT_YUNBA_ERROR        -1
#define MQTT_CONNECT_EMQ_ERROR          1
#define MQTT_CONNECT_SUCCESS            0

#define TICKET_SERVER_CACHE             "/data/redtea/ticket_server"
#define MAX_CONNECT_SERVER_TIMER        3

#define USE_ADAPTER_SERVER              1  // 是否使用mqtt ticket adapter代理服务器

#define AGENT_ALIAS                     "agent"

#define MQTT_KEEP_ALIVE_INTERVAL        300

#define GET_CID_FLAG(flag)              ((flag) & 0x01)
#define GET_AGENT_FLAG(flag)            (((flag) >> 1) & 0x01)
#define GET_DEVICE_ID_FLAG(flag)        (((flag) >> 2) & 0x01)
#define GET_IMEI_FLAG(flag)             (((flag) >> 3) & 0x01)
#define SET_CID_FLAG(flag)              ((flag) |= 0x01)
#define SET_AGENT_FLAG(flag)            ((flag) |= (0x01 << 1))
#define SET_DEVICE_ID_FLAG(flag)        ((flag) |= (0x01 << 2))
#define SET_IMEI_FLAG(flag)             ((flag) |= (0x01 << 3))

int32_t rt_mqtt_task(void);
int8_t *rt_mqtt_get_channel(void);
int32_t init_mqtt(void *arg);

#endif  // __RT_MQTT_H__
