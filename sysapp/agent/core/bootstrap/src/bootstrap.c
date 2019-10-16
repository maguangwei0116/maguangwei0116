
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : bootstrap.c
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "bootstrap.h"
#include "profile_parse.h"
#include "tlv.h"
#include "file.h"
#include "rt_manage_data.h"
#include "agent_queue.h"
#include "rt_type.h"
#include "random.h"

static uint32_t g_single_interval_time  = 10;           // single interval time for activating fail
static uint32_t g_max_retry_times       = 7;            // max retry counter
static uint32_t g_sleep_time            = 24 * 60 * 60; // sleep time
static uint32_t g_is_profile_damaged    = RT_ERROR;
static uint32_t g_retry_times           = 0;

static void bootstrap_select_profile(void)
{
    if (g_retry_times > g_max_retry_times) {
        g_retry_times = 0;
    } else {
        selected_profile((uint32_t)rt_get_random_num());
        g_retry_times++;
        g_single_interval_time *= g_retry_times;
    }
}

int32_t init_bootstrap(void *arg)
{
    g_is_profile_damaged = init_profile_file(NULL);
    return g_is_profile_damaged;
}

void bootstrap_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    if (g_is_profile_damaged != RT_SUCCESS){
        MSG_PRINTF(LOG_INFO, "The share profile is damaged.\n");
        return;
    }
    MSG_PRINTF(LOG_INFO, "The current mode is %d.\n", mode);
    if (mode == MSG_BOOTSTRAP_SELECT_CARD) {
        g_retry_times = 0;
        bootstrap_select_profile();
    } else if (mode == MSG_BOOTSTRAP_DISCONNECTED) {
        MSG_PRINTF(LOG_INFO, "g_single_interval_time:%d\n", g_single_interval_time);
        register_timer(g_single_interval_time, 0, &bootstrap_select_profile);
    } else if (mode == MSG_NETWORK_CONNECTED) {

    }
}

