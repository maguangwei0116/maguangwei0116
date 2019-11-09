
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
#ifdef CFG_PLATFORM_9X07

#include "dial_up.h"
#include "rt_timer.h"
#include "downstream.h"
#include "card_manager.h"

#define MAX_WAIT_REGIST_TIME     180

static int32_t g_network_state = 0;
static rt_bool g_network_timer_flag = RT_FALSE;

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
        MSG_PRINTF(LOG_INFO, "start new timer event state:%d\n", g_network_state);
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
        dial_up_stop(&dsi_net_hndl);
    }

    rt_exit_task(NULL);
}

int32_t network_detection_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    if (mode == MSG_ALL_SWITCH_CARD || mode == MSG_BOOTSTRAP_START_TIMER) {
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

static void network_set_apn_handler(int32_t state)
{
    static char using_iccid[THE_ICCID_LENGTH + 1] = {0};
    char new_iccid[THE_ICCID_LENGTH + 1] = {0};
    profile_type_e type;
    rt_bool update_using_iccid = RT_FALSE;

    MSG_PRINTF(LOG_INFO, "state: %d ==> %d\r\n", g_network_state, state);
    
    /* (disconnected => connected) [record using iccid] */
    if ((g_network_state == DSI_STATE_CALL_IDLE || g_network_state == DSI_STATE_CALL_CONNECTING) && \
            state == DSI_STATE_CALL_CONNECTED) {
        MSG_PRINTF(LOG_WARN, "state changed: DSI_STATE_CALL_IDLE/DSI_STATE_CALL_CONNECTING ==> DSI_STATE_CALL_CONNECTED\r\n");
        /* update profiles info only */
        card_check_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP, using_iccid, NULL);
    }

    /* (connected => disconnected) or (disconnected => disconnected) [check using iccid] */
    if ((g_network_state == DSI_STATE_CALL_CONNECTED && \
            (state == DSI_STATE_CALL_IDLE || state == DSI_STATE_CALL_CONNECTING)) ||\
            (g_network_state == DSI_STATE_CALL_IDLE && state == DSI_STATE_CALL_IDLE)) {
        if (g_network_state == DSI_STATE_CALL_CONNECTED) {
            MSG_PRINTF(LOG_WARN, "state changed: DSI_STATE_CALL_CONNECTED ==> DSI_STATE_CALL_IDLE/DSI_STATE_CALL_CONNECTING\r\n");
        } else if (g_network_state == DSI_STATE_CALL_IDLE) {
            MSG_PRINTF(LOG_WARN, "state changed: DSI_STATE_CALL_IDLE ==> DSI_STATE_CALL_IDLE\r\n");
            update_using_iccid = RT_TRUE;
        }
        
        /* update profiles info only */
        card_check_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP, new_iccid, &type);

        if (rt_os_strncmp(using_iccid, new_iccid, THE_MAX_CARD_NUM)) {
            MSG_PRINTF(LOG_WARN, "iccid changed: %s ==> %s\r\n", using_iccid, new_iccid);
            if (PROFILE_TYPE_OPERATIONAL == type) {
                /* set apn when detect a operational profile */
                card_set_opr_profile_apn();
            } else if (PROFILE_TYPE_PROVISONING == type) {
                /* start bootstrap when detect a provisioning profile */
                card_check_profile_info(UPDATE_JUDGE_BOOTSTRAP, new_iccid, &type);
            }
        }
    }

    if (update_using_iccid) {
        rt_os_memcpy(using_iccid, new_iccid, THE_MAX_CARD_NUM);   
    }
}

static void network_state(int32_t state)
{
    network_set_apn_handler(state);

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

    dial_up_set_dial_callback((void *)network_state);
    
    ret = rt_create_task(&task_id, (void *)network_detection_task, NULL);
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
    //MSG_PRINTF(LOG_INFO, "msg: %s ==> method: %s ==> event: %s\n", downstream_msg->msg, downstream_msg->method, downstream_msg->event);

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

#endif

#ifdef CFG_PLATFORM_ANDROID

#include "network_detection.h"

int32_t network_detection_event(const uint8_t *buf, int32_t len, int32_t mode)
{
}

int32_t network_detect_event(const uint8_t *buf, int32_t len, int32_t mode)
{
}

int32_t init_network_detection(void *arg)
{
}

void    network_state_update(int32_t timeout)
{
}

void    network_state_force_update(int32_t new_state)
{
}

#endif

