
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : network_detection.c
 * Date        : 2019.10.17
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "network_detection.h"
#include "parse_backup.h"
#include "rt_timer.h"
#include "dial_up.h"
#include "download_file.h"

#define MAX_WAIT_REGIST_TIME     180

static int32_t g_network_state = -1;
static rt_bool g_network_timer_flag = RT_FALSE;

static void network_timer_callback(void)
{
    if (g_network_state != DSI_STATE_CALL_CONNECTED) {  // network disconnected
        backup_process(LPA_CHANNEL_BY_IPC);
        g_network_state = -1;
        MSG_PRINTF(LOG_INFO, "timer out g_network_state:%d, restart a new timer ...\r\n", g_network_state);
        register_timer(MAX_WAIT_REGIST_TIME, 0 , &network_timer_callback);
    }
    g_network_timer_flag = RT_FALSE;
    MSG_PRINTF(LOG_INFO, "network state:%d\n", g_network_state);
}

static void network_start_timer(void)
{
    if (g_network_timer_flag == RT_FALSE) {
        register_timer(MAX_WAIT_REGIST_TIME, 0 , &network_timer_callback);
        g_network_timer_flag = RT_TRUE;
    }
}

static void network_state(int32_t state)
{
    if (state == g_network_state) {
        return;
    }

    g_network_state = state;

    if (g_network_state == DSI_STATE_CALL_CONNECTED) {    // network connected
        MSG_PRINTF(LOG_INFO, "Network state:%d\n", state);
        download_start();                                 // start download agent
    } else if (g_network_state == DSI_STATE_CALL_IDLE) {  // network disconnected
        MSG_PRINTF(LOG_INFO, "Network state:%d\n", state);
        network_start_timer();
    }
}

void network_detection_task(void)
{
    dsi_call_info_t dsi_net_hndl;

    dial_up_set_dial_callback((void *)network_state);
    rt_os_sleep(3);
    dial_up_init(&dsi_net_hndl);
    network_start_timer();

    while (1) {
        dial_up_to_connect(&dsi_net_hndl);
    }

    dial_up_deinit(&dsi_net_hndl);
    
    rt_exit_task(NULL);
}
