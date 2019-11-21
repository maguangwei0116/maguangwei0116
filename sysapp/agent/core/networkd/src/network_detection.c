
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
#include "card_detection.h"
#include "bootstrap.h"
#include "rt_timer.h"

#define MAX_INIT_RETRY_CNT              3
#define DELAY_100MS                     100
#define MAX_WAIT_BOOTSTRAP_TIME         150  // 150 * 100ms = 15000ms = 15S
#define NETWORK_STATE_NOT_READY         -1

typedef struct TASK_PARAM {
    int32_t *           profile_damaged;
    profile_type_e *    type;
} task_param_t;

static int32_t g_network_state          = NETWORK_STATE_NOT_READY;
static int32_t g_network_new_state      = 0;

/*
avoid this case:
boot -> exist provisoning profile dial up -> bootstrap start -> switch provisoning profile -> NO-NET -> dial up again
*/
static int32_t network_wait_bootstrap_start(int32_t max_delay_100ms)
{
    while (max_delay_100ms-- > 0) {
        if (card_manager_install_profile_ok()) {
            return RT_SUCCESS;
        }
        rt_os_msleep(DELAY_100MS);
    }

    return RT_ERROR;
}

static void network_detection_task(void *arg)
{
    int32_t ret;
    int32_t cnt = 0;
    dsi_call_info_t dsi_net_hndl;
    profile_type_e *type = ((task_param_t *)arg)->type;
    int32_t *profile_damaged = ((task_param_t *)arg)->profile_damaged;
    
    MSG_PRINTF(LOG_INFO, "start with profile (%d,%d) ...\r\n", *type, *profile_damaged);
    
    /* non-operational profile && share profile ok */
    if (*type != PROFILE_TYPE_OPERATIONAL && *profile_damaged == RT_SUCCESS) {
        network_wait_bootstrap_start(MAX_WAIT_BOOTSTRAP_TIME);
    }

    /* operational profile && share profile damaged */
    if (*type == PROFILE_TYPE_OPERATIONAL && *profile_damaged == RT_ERROR) {
        operational_network_start_timer();
    }

    /* add retry more times */
    while (1) {
        ret = dial_up_init(&dsi_net_hndl);
        if (ret != RT_SUCCESS) {            
            if (++cnt <= MAX_INIT_RETRY_CNT) {
                MSG_PRINTF(LOG_ERR, "dial up init error (%d)\r\n", cnt);
                rt_os_sleep(3);
                continue;
            }
            MSG_PRINTF(LOG_ERR, "dial up init final error\r\n");
            goto  exit_entry;
        }
        break;
    }
    
    while (1) {  
        dial_up_to_connect(&dsi_net_hndl);  // it will never exit !
    }

exit_entry:

    dial_up_deinit(&dsi_net_hndl);

    rt_exit_task(NULL);
}

static void network_state(int32_t state)
{
    if (state == g_network_state) {
        return;
    }

    g_network_state = state;

    if (g_network_state == DSI_STATE_CALL_CONNECTED) {  // network connected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_CONNECTED, NULL, 0);
        card_detection_disable();
    } else if (g_network_state == DSI_STATE_CALL_IDLE) {  // network disconnected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_DISCONNECTED, NULL, 0);
        card_detection_enable();
        g_network_state = NETWORK_STATE_NOT_READY;
    }
}

static task_param_t g_task_param;

int32_t init_network_detection(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;    
    profile_type_e *type;
    int32_t *profile_damaged;

    type                            = &(((public_value_list_t *)arg)->card_info->type);
    profile_damaged                 = ((public_value_list_t *)arg)->profile_damaged;
    g_task_param.type               = type;
    g_task_param.profile_damaged    = profile_damaged;

    dial_up_set_dial_callback((void *)network_state);
    
    ret = rt_create_task(&task_id, (void *)network_detection_task, &g_task_param);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

#endif

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

#ifdef CFG_PLATFORM_ANDROID

#include "network_detection.h"

int32_t init_network_detection(void *arg)
{
    (void)arg;
    return RT_SUCCESS;
}

#endif

