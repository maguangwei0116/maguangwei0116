
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

#include "dial_up.h"
#include "rt_timer.h"
#include "downstream.h"

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
    if (mode == MSG_ALL_SWITCH_CARD) {
        network_start_timer();
        #if 0 // only for debug
        {
            uint8_t imsi[IMSI_LENGTH + 1] = {0};
            rt_os_sleep(10);
            rt_qmi_get_current_imsi(imsi);
            MSG_PRINTF(LOG_INFO, "state:%d, imsi:%s\n", g_network_state, imsi);
        }
        #endif
    }

    return RT_SUCCESS;
}

static void network_state(int32_t state)
{
    if (state == g_network_state) {
        return;
    }
    
    g_network_state = state;

    if (g_network_state == DSI_STATE_CALL_CONNECTED) {  // network connected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_CONNECTED, NULL, 0);
    } else if (g_network_state == DSI_STATE_CALL_IDLE) {  // network disconnected
        network_start_timer();
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_DISCONNECTED, NULL, 0);
    }
}

static void network_state_notify(void)
{
    if (g_network_state == DSI_STATE_CALL_CONNECTED) {  // network connected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_CONNECTED, NULL, 0);
    } else if (g_network_state == DSI_STATE_CALL_IDLE) {  // network disconnected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_DISCONNECTED, NULL, 0);
    }   
}

/* get newest network state after timeout seconds */
void network_state_update(int32_t timeout)
{
    register_timer(timeout, 0 , &network_state_notify);  
}

/* force to update network state when other module found network is inactive */
void network_state_force_update(int32_t new_state)
{
    if (MSG_NETWORK_DISCONNECTED == new_state) {
        network_state(DSI_STATE_CALL_IDLE);
    } else if (MSG_NETWORK_CONNECTED == new_state) {
        network_state(DSI_STATE_CALL_CONNECTED);
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

int32_t network_detect_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t status = 0;
    int32_t ret = RT_ERROR;
    downstream_msg_t *downstream_msg = (downstream_msg_t *)buf;

    (void)mode;
    MSG_PRINTF(LOG_INFO, "msg: %s ==> method: %s ==> event: %s\n", downstream_msg->msg, downstream_msg->method, downstream_msg->event);

    ret = downstream_msg->parser(downstream_msg->msg, downstream_msg->tranId, &downstream_msg->private_arg);
    if (downstream_msg->msg) {
        rt_os_free(downstream_msg->msg);
        downstream_msg->msg = NULL;
    }
    if (ret == RT_ERROR) {
        return ret;
    }
    
    // MSG_PRINTF(LOG_WARN, "tranId: %s, %p\n", downstream_msg->tranId, downstream_msg->tranId);
    status = downstream_msg->handler(downstream_msg->private_arg,  downstream_msg->event, &downstream_msg->out_arg);
    upload_event_report(downstream_msg->event, (const char *)downstream_msg->tranId, status, downstream_msg->out_arg);

    return RT_SUCCESS;
}

