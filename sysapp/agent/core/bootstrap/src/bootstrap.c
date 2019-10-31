
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
#include "agent2monitor.h"

#define DEFAULT_SINGLE_INTERVAL_TIME    10                              // default interval time (seconds)
static uint32_t g_single_interval_time  = DEFAULT_SINGLE_INTERVAL_TIME; // single interval time for activating fail
static uint32_t g_max_retry_times       = 7;                            // max retry counter
static int32_t g_is_profile_damaged     = RT_ERROR;
static uint32_t g_retry_times           = 0;

static void bootstrap_select_profile(void)
{
    if (g_retry_times > g_max_retry_times) {
        g_retry_times           = 0;
        g_single_interval_time  = DEFAULT_SINGLE_INTERVAL_TIME;
    } else {
        selected_profile((uint32_t)rt_get_random_num());
        g_retry_times++;
        g_single_interval_time *= g_retry_times;
        MSG_PRINTF(LOG_INFO, "g_single_interval_time(%d)=%d\n", g_retry_times, g_single_interval_time);
    }
}

int32_t init_bootstrap(void *arg)
{
    ((public_value_list_t *)arg)->profile_damaged = &g_is_profile_damaged;
    g_is_profile_damaged = init_profile_file(NULL);
    
    return g_is_profile_damaged;
}

void bootstrap_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    if (g_is_profile_damaged != RT_SUCCESS){
        MSG_PRINTF(LOG_INFO, "The share profile is damaged !\n");
        return;
    }
    MSG_PRINTF(LOG_INFO, "The current mode is %d\n", mode);
    if (mode == MSG_BOOTSTRAP_SELECT_CARD) {
        g_retry_times           = 0;
        g_single_interval_time  = DEFAULT_SINGLE_INTERVAL_TIME;
        bootstrap_select_profile();
    } else if (mode == MSG_BOOTSTRAP_DISCONNECTED) {
        MSG_PRINTF(LOG_INFO, "g_single_interval_time(%d)=%d\n", g_retry_times, g_single_interval_time);
        register_timer(g_single_interval_time, 0, &bootstrap_select_profile);
    } else if (mode == MSG_NETWORK_CONNECTED) {

    }
}

void bootstrap_monitor_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    if (g_is_profile_damaged != RT_SUCCESS) {
        if (mode == MSG_BOOTSTRAP_SELECT_CARD) {
            MSG_PRINTF(LOG_WARN, "share profile damaged, select a profile from monitor !!!\r\n");
            if (RT_ERROR == ipc_select_profile_by_monitor()) {
                MSG_PRINTF(LOG_WARN, "select a profile from monitor fail !!!\r\n");
            }
        } else if (mode == MSG_NETWORK_CONNECTED) {
            MSG_PRINTF(LOG_WARN, "download default share profile ...\r\n");
            ota_download_default_share_profile();  
        }
    }    
}

int32_t bootstrap_get_profile_version(char *batch_code, int32_t b_size, char *version, int32_t v_size)
{
    if (g_is_profile_damaged != RT_SUCCESS) {
        /* set a temp batch code and version */
        snprintf(batch_code, b_size, "%s", "Bxxxxxxxxxxxxxxxxxx");
        snprintf(version, v_size, "%s", "0.0.0.0");
        return RT_ERROR;
    } else {
        return get_share_profile_version(batch_code, b_size, version, v_size);
    }
}   

