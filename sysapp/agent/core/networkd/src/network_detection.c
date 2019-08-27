
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

static int32_t g_network_state = 0;
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
        MSG_PRINTF(LOG_INFO, "event state:%d\n", g_network_state);
    }
}

void network_state(int32_t state)
{
    g_network_state = state;
    MSG_PRINTF(LOG_INFO, "state:%d\n", g_network_state);
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
    return RT_SUCCESS;
}
