
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

#define DEFAULT_SINGLE_INTERVAL_TIME            10                              // default interval time (seconds)
#define DEFAULT_PROFILE_DETECT_TIME             60                              // detect default share profile download time (seconds)
#define MAX_RETRY_TIMES                         7                               // max retry counter

static uint32_t g_single_interval_time          = DEFAULT_SINGLE_INTERVAL_TIME; // single interval time for activating fail                           
static int32_t g_is_profile_damaged             = RT_ERROR;
static public_value_list_t * g_public_value     = NULL;
static uint32_t g_retry_times                   = 0;

static void bootstrap_select_profile(void)
{
    if (g_retry_times > MAX_RETRY_TIMES) {
        g_retry_times           = 0;
        g_single_interval_time  = DEFAULT_SINGLE_INTERVAL_TIME;
    } else {
        selected_profile((uint32_t)rt_get_random_num());
        g_single_interval_time *= ++g_retry_times;
        MSG_PRINTF(LOG_INFO, "g_single_interval_time(%d)=%d\n", g_retry_times, g_single_interval_time);
    }
}

static void bootstrap_download_default_share_profile_once(void)
{
    static rt_bool g_start_download_profile = RT_FALSE;
    
    if (!g_start_download_profile) {
        MSG_PRINTF(LOG_WARN, "download default share profile ...\r\n");
        g_start_download_profile = RT_TRUE;        
        ota_download_default_share_profile();
    }
}

int32_t init_bootstrap(void *arg)
{
    g_public_value = (public_value_list_t *)arg;    
    g_public_value->profile_damaged = &g_is_profile_damaged;
    g_is_profile_damaged = init_profile_file(NULL);
    
    return g_is_profile_damaged;
}

void bootstrap_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    if (g_is_profile_damaged == RT_SUCCESS) {
        if (g_public_value->card_info->type != PROFILE_TYPE_PROVISONING) {
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
            g_retry_times           = 0;
            g_single_interval_time  = DEFAULT_SINGLE_INTERVAL_TIME;
        }      
    } else {        
        MSG_PRINTF(LOG_INFO, "The share profile is damaged ! mode: %d, type: %d\n", mode, g_public_value->card_info->type);
        if (mode == MSG_BOOTSTRAP_SELECT_CARD) {
            static int32_t frist_bootstrap_msg = 1;

            /* ignore frist bootstrap msg when it's using provsioning profile */
            if (g_public_value->card_info->type == PROFILE_TYPE_PROVISONING && frist_bootstrap_msg == 1) {
                frist_bootstrap_msg - 0;   
            } else {
                ipc_select_profile_by_monitor();
            }         
        } else if (mode == MSG_BOOTSTRAP_DISCONNECTED) {
            if (g_public_value->card_info->type == PROFILE_TYPE_OPERATIONAL) {
                /* switch to provisoning profile, and check again */
                card_force_enable_provisoning_profile();     
                msg_send_agent_queue(MSG_ID_NETWORK_DECTION, MSG_BOOTSTRAP_START_TIMER, NULL, 0);
            } else if (g_public_value->card_info->type == PROFILE_TYPE_PROVISONING) {
                ipc_select_profile_by_monitor();                
            }
        } else if (mode == MSG_NETWORK_CONNECTED) {            
            bootstrap_download_default_share_profile_once();  
        } 
    }
}

int32_t bootstrap_get_profile_version(char *batch_code, int32_t b_size, char *version, int32_t v_size)
{
    if (g_is_profile_damaged == RT_SUCCESS) {
        return get_share_profile_version(batch_code, b_size, version, v_size);
    } else {        
        /* set a temp batch code and version */
        snprintf(batch_code, b_size, "%s", "Bxxxxxxxxxxxxxxxxxx");
        snprintf(version, v_size, "%s", "0.0.0.0");
        return RT_ERROR;
    }
}   

