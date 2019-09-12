
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : network_detection.c
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "network_detection.h"
#include "agent_queue.h"
#include "dial_up.h"
#include "rt_timer.h"
#include "agent_main.h"

#define MAX_WAIT_REGIST_TIME     180

static int32_t g_network_state = 0;
static rt_bool g_network_timer_flag = RT_TRUE;

static void network_timer_callback(void)
{
    if (g_network_state == DSI_STATE_CALL_IDLE) {  // network disconnected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_BOOTSTRAP_DISCONNECTED, NULL, 0);
    }
    g_network_timer_flag = RT_FALSE;
    MSG_PRINTF(LOG_INFO, "event state:%d\n", g_network_state);
}

static void network_start_timer(void)
{
    if (g_network_timer_flag == RT_FALSE) {
        register_timer(MAX_WAIT_REGIST_TIME, 0 , &network_timer_callback);
        g_network_timer_flag = RT_TRUE;
    }
}

static void network_detection_task(void)
{
    dsi_call_info_t dsi_net_hndl;
    dial_up_init(&dsi_net_hndl);
    while (1) {
        dial_up_to_connect(&dsi_net_hndl);
        dial_up_stop(dsi_net_hndl);
    }
}

int32_t network_detection_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    uint8_t imsi[IMSI_LENGTH + 1];
    if (mode == MSG_ALL_SWITCH_CARD) {
        network_start_timer();
        rt_os_sleep(10);
        rt_qmi_get_current_imsi(imsi);
        MSG_PRINTF(LOG_INFO, "state:%d, imsi:%s\n", g_network_state, imsi);
    }
}

void network_state(int32_t state)
{
    g_network_state = state;
    if (g_network_state == DSI_STATE_CALL_CONNECTED) {  // network connected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_CONNECTED, NULL, 0);
    } else if (g_network_state == DSI_STATE_CALL_IDLE) {  // network disconnected
        network_start_timer();
    }
}

int32_t init_network_detection(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;

    regist_dial_callback((void *)network_state);
    ret = rt_create_task(&task_id, (void *) network_detection_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }
    register_timer(MAX_WAIT_REGIST_TIME, 0 , &network_timer_callback);
    return RT_SUCCESS;
}
