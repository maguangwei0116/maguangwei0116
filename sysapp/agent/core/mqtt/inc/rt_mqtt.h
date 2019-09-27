
#ifndef __RT_MQTT_H__
#define __RT_MQTT_H__

#include "rt_type.h"

int32_t init_mqtt(void *arg);
int32_t mqtt_connect_event(const uint8_t *buf, int32_t len, int32_t mode);

#endif  // __RT_MQTT_H__
