
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
#include "dial_up.h"

int32_t init_network_detection(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;

    ret = rt_create_task(&task_id, (void *) network_detection_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_ERR("create task fail\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

void network_detection_task(void)
{
    dsi_call_info_t dsi_net_hndl;
    init_dial_up(&dsi_net_hndl);
    while (1) {
        dial_up_to_connect(&dsi_net_hndl);
    }
}
