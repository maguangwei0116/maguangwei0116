
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
#include "ota.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)                           (sizeof((a)) / sizeof((a)[0]))
#endif
#define SHARE_PROFILE                           "/data/redtea/profile_list.der"
#define DEFAULT_SINGLE_INTERVAL_TIME            10                                      // default interval time (seconds)
#define MAX_WAIT_REGIST_TIME                    30

/* define your interval time table, unit: seconds, max 2.1h */
static const uint32_t g_time_table[]            = {10, 30, 90, 270, 840, 2520, 7560};
static uint32_t g_single_interval_time          = DEFAULT_SINGLE_INTERVAL_TIME;         // single interval time for activating fail
static uint32_t g_max_retry_times               = ARRAY_SIZE(g_time_table);             // max retry counter
static uint32_t g_is_profile_damaged            = RT_ERROR;
static uint32_t g_retry_times                   = 0;
static rt_bool g_bootstrap_network              = RT_FALSE;
static rt_bool g_network_timer_flag             = RT_FALSE;
static public_value_list_t * g_public_value     = NULL;

static void bootstrap_network_timer_callback(void)
{
    if (g_bootstrap_network == RT_FALSE) {  // network disconnected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_BOOTSTRAP_DISCONNECTED, NULL, 0);
    }
    g_network_timer_flag = RT_FALSE;
    MSG_PRINTF(LOG_INFO, "%s, netwrok state: %d\r\n", __func__, g_bootstrap_network);
}

static void bootstrap_network_start_timer(void)
{
    if (g_network_timer_flag == RT_FALSE) {
        register_timer(MAX_WAIT_REGIST_TIME, 0 , &bootstrap_network_timer_callback);
        g_network_timer_flag = RT_TRUE;
    }
}

int32_t bootstrap_select_profile(uint16_t mcc, char *apn, uint8_t *profile, uint16_t *profile_len)
{
    return selected_profile(mcc, apn, profile, profile_len);    
}

int32_t bootstrap_init_profile(const char *file)
{
    return init_profile_file(file);   
}

static void bootstrap_local_select_profile(void)
{
    if (g_retry_times > g_max_retry_times) {
        g_retry_times           = 0;
        g_single_interval_time  = DEFAULT_SINGLE_INTERVAL_TIME;
        MSG_PRINTF(LOG_ERR, "bootstrap select un-work profiles too many times !\r\n");
    } else { 
        uint16_t mcc = 0;
        char apn[128] = {0};
        uint8_t profile_buffer[1024] = {0};
        uint16_t profile_len = 0;
        
        #if 0  // only for debug
        unsigned long cur_time = time(NULL);
        static unsigned long last_time = 0;
        
        if (!last_time) {
            last_time = cur_time;
        }
        MSG_PRINTF(LOG_INFO, "<<<< bootstrap select card (%d/%d) [%ld] >>>>>>\r\n", g_retry_times, g_max_retry_times, cur_time - last_time);
        last_time = cur_time;
        #else
        MSG_PRINTF(LOG_INFO, "<<<< bootstrap select card (%d/%d) >>>>>>\r\n", g_retry_times, g_max_retry_times);
        #endif

        rt_qmi_get_mcc_mnc(&mcc, NULL);
        bootstrap_select_profile(mcc, apn, profile_buffer, &profile_len); 
        rt_qmi_modify_profile(1, 0,apn, 0);
        msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_SETTING_PROFILE, profile_buffer, profile_len);       
        msg_send_agent_queue(MSG_ID_BOOT_STRAP, MSG_START_NETWORK_DETECTION, NULL, 0);
        if (g_retry_times < g_max_retry_times) {            
            g_single_interval_time = g_time_table[g_retry_times];                        
        } 
        g_retry_times++;
        MSG_PRINTF(LOG_INFO, "next single interval time(%d)=%d\r\n", g_retry_times, g_single_interval_time);
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
    g_is_profile_damaged = bootstrap_init_profile(SHARE_PROFILE);

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
            bootstrap_local_select_profile();
        } else if (mode == MSG_BOOTSTRAP_DISCONNECTED) {
            MSG_PRINTF(LOG_INFO, "wait interval time(%d)=%d to select card ...\r\n", g_retry_times, g_single_interval_time);
            register_timer(g_single_interval_time, 0, &bootstrap_local_select_profile);
        } else if (mode == MSG_NETWORK_CONNECTED) {
            g_retry_times           = 0;
            g_single_interval_time  = DEFAULT_SINGLE_INTERVAL_TIME;
            g_bootstrap_network     = RT_TRUE;
        } else if (mode == MSG_NETWORK_DISCONNECTED) {
            g_bootstrap_network     = RT_FALSE;
        } else if (mode == MSG_START_NETWORK_DETECTION) {
            bootstrap_network_start_timer();
        }     
    } else {        
        MSG_PRINTF(LOG_INFO, "The share profile is damaged ! mode: %d, type: %d\n", mode, g_public_value->card_info->type);
        if (mode == MSG_BOOTSTRAP_SELECT_CARD) {
            static int32_t frist_bootstrap_msg = 1;

            /* ignore frist bootstrap msg when it's using provsioning profile */
            if (g_public_value->card_info->type == PROFILE_TYPE_PROVISONING && frist_bootstrap_msg == 1) {
                frist_bootstrap_msg = 0;   
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
            g_bootstrap_network     = RT_TRUE;
            bootstrap_download_default_share_profile_once();  
        } else if (mode == MSG_NETWORK_DISCONNECTED) {
            g_bootstrap_network     = RT_FALSE;
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

