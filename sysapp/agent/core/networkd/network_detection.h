
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : network_detection.h
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __NETWORK_DETECTION_H__
#define __NETWORK_DETECTION_H__

#include "rt_type.h"

#ifdef CFG_PLATFORM_9X07

#include "dial_up.h"

// these values defined in dial_up.h

#define RT_DSI_STATE_CALL_IDLE              DSI_STATE_CALL_IDLE
#define RT_DSI_STATE_CALL_CONNECTING        DSI_STATE_CALL_CONNECTING
#define RT_DSI_STATE_CALL_CONNECTED         DSI_STATE_CALL_CONNECTED
#define RT_DSI_STATE_CALL_DISCONNECTING     DSI_STATE_CALL_DISCONNECTING
#define RT_DSI_STATE_CALL_MAX               DSI_STATE_CALL_MAX

#endif

#ifdef CFG_PLATFORM_ANDROID

typedef enum DSI_STATE_CALL_STATE {
    RT_DSI_STATE_CALL_IDLE = 0,             /* network idle (disconnected)  */
    RT_DSI_STATE_CALL_CONNECTING,           /* network connecting           */
    RT_DSI_STATE_CALL_CONNECTED,            /* network connected            */
    RT_DSI_STATE_CALL_DISCONNECTING,        /* network disconnecting        */
    RT_DSI_STATE_CALL_MAX                   /* network unknow state         */
} dsi_state_call_state_e;

#endif

int32_t init_network_detection(void *arg);
int32_t network_detect_event(const uint8_t *buf, int32_t len, int32_t mode);

/**
 brief:     set current network state
 param[in]: state @ref dsi_state_call_state_e
 note:      agent only pay close attention to 2 state: [RT_DSI_STATE_CALL_CONNECTED and RT_DSI_STATE_CALL_IDLE]
 */
void    network_update_state(int32_t state); 

#endif // __NETWORK_DETECTION_H__

