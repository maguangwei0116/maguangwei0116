
#ifndef __RT_MQTT_H__
#define __RT_MQTT_H__

#include "rt_type.h"

#define MQTT_PUBLISH_NO_YUNBA       -88
#define MQTT_PUBLISH_NO_CONNECTED   -89

int32_t init_mqtt(void *arg);
int32_t mqtt_connect_event(const uint8_t *buf, int32_t len, int32_t mode);
int32_t mqtt_pulish_msg(const void* data, int32_t data_len);

#endif  // __RT_MQTT_H__
