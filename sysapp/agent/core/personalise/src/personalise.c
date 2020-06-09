
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

#include "rt_type.h"
#include "device_info.h"
#include "md5.h"
#include "rt_qmi.h"
#include "agent_queue.h"

const devicde_info_t *g_personalise_device_info     = NULL;
const target_versions_t *g_personalise_version_info = NULL;
static int32_t *g_share_profile_damaged             = NULL;

int32_t init_personalise(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;

    g_personalise_device_info   = (const devicde_info_t *)public_value_list->device_info;
    g_personalise_version_info  = (const target_versions_t *)public_value_list->version_info;
    g_share_profile_damaged     = (int32_t *)public_value_list->profile_damaged;

    return RT_SUCCESS;
}

int32_t personalise_upload_no_cert(void *arg)
{
    if (g_share_profile_damaged && *g_share_profile_damaged == RT_SUCCESS) {
        upload_event_report("NO_CERT", NULL, 0, NULL);
    }

    return RT_SUCCESS;
}

