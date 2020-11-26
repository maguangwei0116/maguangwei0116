
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_flow_control.h
 * Date        : 2019.12.24
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <math.h>
#include "rt_type.h"
#include "rt_os.h"
#include "card_manager.h"
#include "agent_queue.h"
#include "network_detection.h"
#include "card_flow_control.h"

static card_info_t                         *g_p_info;
static card_flow_switch_e                  *g_flow_switch = NULL;

#define INITIAL_WAIT_TIME                  3600         // 1H, ti
#define INITIAL_SLEEP_TIME                 10           // 10s
#define INITIAL_JUDGE_TIMES                (INITIAL_WAIT_TIME / INITIAL_SLEEP_TIME)
#define WAIT_TIME_TO_ENABLE_CARD           600          // 10 min
#define INITIAL_ENABLE_JUDGE_TIMES         (WAIT_TIME_TO_ENABLE_CARD / INITIAL_SLEEP_TIME)
#define THE_MAX_WAIT_TIME                  (6 * INITIAL_WAIT_TIME)

#define CALCULATING_WAIT_TIME(ti, ts, n, tn) \
do {                 \
   tn = pow(2, n-1) * ti + ts;     \
} while (0)

typedef enum FLOW_CONTROL_STATE {
    FLOW_INITIAL_WAIT_STATE = 0,
    FLOW_WAIT_ENABLE_STATE,
    FLOW_WAIT_NEXT_TIME_STATE
} flow_control_state_e;

static void flow_control_main(void)
{
    int32_t time = 0;
    int32_t tn = 0;
    int32_t num = 1;
    int32_t wait_times = 0;
    flow_control_state_e state = FLOW_INITIAL_WAIT_STATE;

    MSG_PRINTF(LOG_INFO, "Start provisoning flow control ...\n");

    while (1) {
        switch (state) {
            case FLOW_INITIAL_WAIT_STATE:
                if (time >= INITIAL_JUDGE_TIMES) {
                    if (g_p_info->type == PROFILE_TYPE_PROVISONING) {
                        MSG_PRINTF(LOG_INFO, "Used iccid:%s, len:%d\n", g_p_info->iccid, rt_os_strlen(g_p_info->iccid));
                        state = FLOW_WAIT_NEXT_TIME_STATE;
                    } else if (g_p_info->type != PROFILE_TYPE_OPERATIONAL) {
                        state = FLOW_WAIT_ENABLE_STATE;
                    }
                    time = 0;
                }
            break;

            case FLOW_WAIT_ENABLE_STATE:
                if (time == 1) {
                    MSG_PRINTF(LOG_INFO, "Provisioning Enable ...\n");
                    network_update_switch(NETWORK_UPDATE_ENABLE);
                    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_UPDATE_SEED, NULL, 0);
                } else if (time >= INITIAL_ENABLE_JUDGE_TIMES) {
                    if (g_p_info->type == PROFILE_TYPE_PROVISONING) {
                        state = FLOW_WAIT_NEXT_TIME_STATE;
                        time = 0;
                    }
                }
            break;

            case FLOW_WAIT_NEXT_TIME_STATE:
                if (time == 1) {
                    CALCULATING_WAIT_TIME(INITIAL_WAIT_TIME, WAIT_TIME_TO_ENABLE_CARD, num, tn);
                    MSG_PRINTF(LOG_INFO, "time:%d,tn:%d, pow:%d\n", time, tn, pow(2, num - 1));
                    wait_times = tn / INITIAL_SLEEP_TIME;
                    if (tn > THE_MAX_WAIT_TIME) {
                        state = FLOW_INITIAL_WAIT_STATE;
                        time = 0;
                        num = 1;
                    } else {
                        MSG_PRINTF(LOG_INFO, "Provisioning Disable ...\n");
                        network_update_switch(NETWORK_UPDATE_DISABLE);
                        msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_DISABLE_EXIST_CARD, (void *)(g_p_info->iccid), rt_os_strlen(g_p_info->iccid));
                        num ++;
                    }
                }
                if (time > wait_times) {
                    state = FLOW_WAIT_ENABLE_STATE;
                    time = 0;
                }
            break;
        }
        if (g_p_info->type == PROFILE_TYPE_OPERATIONAL) {
            network_update_switch(NETWORK_UPDATE_ENABLE);
            time = 0;
        }
        rt_os_sleep(INITIAL_SLEEP_TIME);
        time ++;
    }
}

static int32_t flow_control_create_task(void)
{
    int32_t ret = RT_ERROR;
    rt_task id;

    ret = rt_create_task(&id, (void *)flow_control_main, NULL);
    if (ret == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "create card flow pthread error, err(%d)=%s\r\n", errno, strerror(errno));
    }

    return ret;
}

int32_t init_flow_control(void *arg)
{
    int32_t ret = RT_SUCCESS;

    g_p_info = ((public_value_list_t *)arg)->card_info;
    g_flow_switch = (card_flow_switch_e *)&(((public_value_list_t *)arg)->config_info->flow_control_switch);
    MSG_PRINTF(LOG_INFO, "Flow control switch:%d\n", *g_flow_switch);
    if (*g_flow_switch == CARD_FLOW_ENABLE) {
        ret = flow_control_create_task();
    }

    return ret;
}
