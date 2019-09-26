
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : personalise.c
 * Date        : 2019.09.26
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "device_info.h"
#include "md5.h"
#include "rt_qmi.h"
#include "agent_queue.h"

const devicde_info_t *g_personalise_device_info = NULL;

int32_t init_personalise(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    g_personalise_device_info = (const devicde_info_t *)public_value_list->device_info;

    return RT_SUCCESS;
}

