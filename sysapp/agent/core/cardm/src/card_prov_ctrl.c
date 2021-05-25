
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_prov_ctrl.c
 * Date        : 2021.04.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "config.h"
#include "usrdata.h"
#include "rt_timer.h"
#include "agent_queue.h"
#include "card_manager.h"
#include "network_detection.h"
#include "card_prov_ctrl.h"

#define MAX_WAIT_REGIST_TIME        180 /* 3 minutes */

typedef enum PROV_CTRL_MODE {
    PROV_CTRL_NORMAL                = 0,
    PROV_CTRL_CONTROL               = 1,
} prov_ctrl_mode_e;

static int32_t                      g_prov_ctrl_mode            = PROV_CTRL_NORMAL;
static int32_t                      g_prov_ctrl_limit           = 0;
static int32_t                      g_prov_ctrl_counter         = 0;
static public_value_list_t          *g_p_value_list             = NULL;
static rt_bool                      g_card_prov_ctrl_timer_flag = RT_FALSE;
static rt_bool                      g_card_prov_reset           = RT_FALSE;
static rt_bool                      g_card_prov_ctrol           = RT_FALSE;

int32_t init_card_prov_ctrl(void *arg)
{
    int32_t ret = RT_ERROR;
    g_p_value_list = ((public_value_list_t *)arg);

    g_card_prov_reset = RT_TRUE;
    g_card_prov_ctrol = RT_FALSE;

    ret = config_get_prov_ctrl_mode(&g_prov_ctrl_mode);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get Provisioning control mode failed!\r\n");
    }
    ret = config_get_prov_ctrl_limit(&g_prov_ctrl_limit);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get Provisioning control mode failed!\r\n");
    }
    ret = rt_read_prov_ctrl_counter(0, &g_prov_ctrl_counter);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get Provisioning control counter failed!\r\n");
    }

    MSG_PRINTF(LOG_DBG, "g_prov_ctrl_mode:%d, g_prov_ctrl_limit:%d, g_prov_ctrl_counter:%d.\r\n", g_prov_ctrl_mode, g_prov_ctrl_limit, g_prov_ctrl_counter);
    return RT_SUCCESS;
}

rt_bool card_prov_ctrl_get(void)
{
    return g_card_prov_ctrol;
}

static int32_t card_prov_ctrl_inc_counter(void)
{
    int32_t ret = RT_ERROR;
    g_prov_ctrl_counter++;
    ret = rt_write_prov_ctrl_counter(0, g_prov_ctrl_counter);    
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Increase Provisioning control counter failed!\r\n");
    }
    return RT_SUCCESS;
}

static int32_t card_prov_ctrl_clr_counter(void)
{
    int32_t ret = RT_ERROR;
    g_prov_ctrl_counter = 0;
    ret = rt_write_prov_ctrl_counter(0, g_prov_ctrl_counter);    
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Clear Provisioning control counter failed!\r\n");
    }
    return RT_SUCCESS;
}

static void card_prov_ctrl_timer_callback(void)
{
    g_card_prov_ctrl_timer_flag = RT_FALSE;    
    if (g_p_value_list == NULL || g_p_value_list->card_info == NULL) {
        MSG_PRINTF(LOG_DBG, "Parameter is not initialized!\r\n");
        return;
    }
    if (card_prov_ctrl_judgement(g_p_value_list->card_info->type) && g_p_value_list->card_info->type == PROFILE_TYPE_PROVISONING) {
        MSG_PRINTF(LOG_INFO, "Provisioning card control ...\n");
        network_update_switch(NETWORK_UPDATE_DISABLE);
        msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_DISABLE_EXIST_CARD, (void *)(g_p_value_list->card_info->iccid), rt_os_strlen(g_p_value_list->card_info->iccid));
    }
}

static void card_prov_ctrl_start_timer(void)
{
    if (g_card_prov_ctrl_timer_flag == RT_FALSE) {
        MSG_PRINTF(LOG_INFO, "Start a new card provisiong control timer ...\r\n");
        register_timer(MAX_WAIT_REGIST_TIME, 0 , &card_prov_ctrl_timer_callback);
        g_card_prov_ctrl_timer_flag = RT_TRUE;
    }
}

rt_bool card_prov_ctrl_increase(profile_type_e type)
{
    if ((type != PROFILE_TYPE_PROVISONING) && (type != PROFILE_TYPE_TEST)) {
        MSG_PRINTF(LOG_DBG, "Do not increase card prov ctrl counter, type: %d\r\n", type);
        return RT_FALSE;
    }
    if (g_prov_ctrl_mode != PROV_CTRL_CONTROL) {
        MSG_PRINTF(LOG_DBG, "Do not increase card prov ctrl counter, g_prov_ctrl_mode: %d\r\n", g_prov_ctrl_mode);
        return RT_FALSE;
    }
    card_prov_ctrl_inc_counter();
    MSG_PRINTF(LOG_DBG, "Increase card prov ctrl counter, g_prov_ctrl_counter: %d\r\n", g_prov_ctrl_counter);
    return RT_TRUE;
}

rt_bool card_prov_ctrl_judgement(profile_type_e type)
{
    if (g_card_prov_reset = RT_FALSE) {
        MSG_PRINTF(LOG_DBG, "Do not execute card prov ctrl, g_card_prov_reset: %d\r\n", g_card_prov_reset);
        return RT_FALSE;
    }
    if ((type != PROFILE_TYPE_PROVISONING) && (type != PROFILE_TYPE_TEST)) {
        MSG_PRINTF(LOG_DBG, "Do not execute card prov ctrl, type: %d\r\n", type);
        return RT_FALSE;
    }
    if (g_prov_ctrl_mode != PROV_CTRL_CONTROL) {
        MSG_PRINTF(LOG_DBG, "Do not execute card prov ctrl, g_prov_ctrl_mode: %d!\r\n", g_prov_ctrl_mode);
        return RT_FALSE;
    }
    MSG_PRINTF(LOG_DBG, "g_prov_ctrl_counter: %d, g_prov_ctrl_limit: %d\r\n", g_prov_ctrl_counter, g_prov_ctrl_limit);
    if (g_prov_ctrl_counter >= g_prov_ctrl_limit) {
        MSG_PRINTF(LOG_DBG, "Do not execute card prov ctrl!\r\n");
        return RT_FALSE;
    }
    g_card_prov_ctrol = RT_TRUE;
    MSG_PRINTF(LOG_DBG, "Execute card prov ctrl!\r\n");
    return RT_TRUE;
}

int32_t card_prov_ctrl_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    if (mode == MSG_NETWORK_CONNECTED) {
        if (g_p_value_list == NULL || g_p_value_list->card_info == NULL) {
            MSG_PRINTF(LOG_DBG, "Parameter is not initialized!\r\n");
            return RT_SUCCESS;
        }
        if ((g_p_value_list->card_info->type == PROFILE_TYPE_PROVISONING) &&
            (g_prov_ctrl_mode == PROV_CTRL_CONTROL) &&
            (g_prov_ctrl_counter >= g_prov_ctrl_limit)) {
            MSG_PRINTF(LOG_DBG, "Clear card prov ctrl counter!\r\n");
            card_prov_ctrl_clr_counter();
            card_prov_ctrl_start_timer();
        }
        if (g_p_value_list->card_info->type != PROFILE_TYPE_PROVISONING) {
            g_card_prov_reset = RT_FALSE;
        }
    }
    return RT_SUCCESS;
}
