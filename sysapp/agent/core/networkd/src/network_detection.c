
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

void network_update_state(int32_t state)
{
    #define NETWORK_STATE_NOT_READY     -1
    
    static int32_t g_network_state      = NETWORK_STATE_NOT_READY;
    
    if (state == g_network_state) {
        return;
    }

    MSG_PRINTF(LOG_INFO, "network state changed: %d -> %d\r\n", g_network_state, state);
    g_network_state = state;

    if (g_network_state == RT_DSI_STATE_CALL_CONNECTED) {  // network connected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_CONNECTED, NULL, 0);
        card_detection_disable();
    } else if (g_network_state == RT_DSI_STATE_CALL_IDLE) {  // network disconnected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_DISCONNECTED, NULL, 0);
        card_detection_enable();
        g_network_state = NETWORK_STATE_NOT_READY;
    }

    #undef NETWORK_STATE_NOT_READY
}

#ifdef CFG_PLATFORM_9X07

#include "card_manager.h"
#include "card_detection.h"
#include "bootstrap.h"

#define MAX_INIT_RETRY_CNT              3
#define DELAY_100MS                     100
#define MAX_WAIT_BOOTSTRAP_TIME         150  // 150 * 100ms = 15000ms = 15S

typedef struct TASK_PARAM {
    int32_t *           profile_damaged;
    profile_type_e *    type;
} task_param_t;

static task_param_t     g_task_param;

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

void network_force_down(void)
{
    /* force to make network down */
    dial_up_reset();
}

int32_t init_network_detection(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;    

    g_task_param.type               = &(((public_value_list_t *)arg)->card_info->type);
    g_task_param.profile_damaged    = ((public_value_list_t *)arg)->profile_damaged;

    dial_up_set_dial_callback((void *)network_update_state);
    
    ret = rt_create_task(&task_id, (void *)network_detection_task, &g_task_param);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

#endif

#ifdef CFG_PLATFORM_ANDROID

void network_force_down(void)
{
    /* force to make network down */
    // TODO
}

int32_t init_network_detection(void *arg)
{
    (void)arg;
    return RT_SUCCESS;
}

#endif

