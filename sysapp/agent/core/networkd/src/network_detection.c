
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

#include "rt_os.h"
#include "network_detection.h"
#include "agent_queue.h"
#include "usrdata.h"

#include "card_manager.h"
#include "card_detection.h"
#include "bootstrap.h"

#define MAX_INIT_RETRY_CNT              5
#define DELAY_100MS                     100
#define MAX_WAIT_BOOTSTRAP_TIME         150  // 150 * 100ms = 15000ms = 15S

typedef struct TASK_PARAM {
    int32_t *           profile_damaged;
    profile_type_e *    type;
} task_param_t;

static task_param_t             g_task_param;
static network_update_switch_e  g_update_network_state = NETWORK_UPDATE_ENABLE;

static void rt_iptables_allow()
{
    system("iptables -D FORWARD -i bridge0  -o rmnet_data+ -j ACCEPT");
    system("iptables -I FORWARD -i bridge0  -o rmnet_data+ -j ACCEPT");
    system("iptables -D FORWARD -i bridge0 -p tcp -m state --state INVALID -j DROP");
    system("iptables -A FORWARD -i bridge0 -p tcp -m state --state INVALID -j DROP");
}

static void rt_iptables_forbid()
{
    system("iptables -D FORWARD -i bridge0 -o rmnet_data+ -j ACCEPT");
    system("iptables -D FORWARD -i bridge0 -p tcp -m state --state INVALID -j DROP");
}

static void rt_provsioning_forbit_ping()
{
    MSG_PRINTF(LOG_INFO, "====> provisoning forbit ping\n");

    system("iptables -A INPUT -i bridge0 -p tcp --dport 23 -j ACCEPT");     // 允许telnet输入
    system("iptables -A OUTPUT -p tcp --sport 23 -j ACCEPT");               // 允许telnet输出

    system("iptables -A FORWARD -i bridge0 -p icmp -j DROP");
    system("iptables -A FORWARD -i bridge0 -p tcp -j DROP");                // icmp, tcp, udp 所有的转发数据包全部丢掉
    system("iptables -A FORWARD -i bridge0 -p udp -j DROP");

    system("iptables -A INPUT -i bridge0 -p icmp -j DROP");
    system("iptables -A INPUT -i bridge0 -p tcp -j DROP");                  // icmp, tcp, udp 所有的输入数据包全部丢掉
    system("iptables -A INPUT -i bridge0 -p udp -j DROP");
}

static void rt_allow_ping()
{
    system("iptables -F");
}

void network_update_switch(network_update_switch_e state)
{
    g_update_network_state = state;
}

void network_update_state(int32_t state)
{
    #define NETWORK_STATE_NOT_READY     -1

    static int32_t g_network_state      = NETWORK_STATE_NOT_READY;

    if (state == g_network_state) {
        return;
    } else if (g_update_network_state == NETWORK_UPDATE_DISABLE) {
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_DISCONNECTED, &g_update_network_state, sizeof(g_update_network_state));
        card_detection_disable();
        return;
    }

    MSG_PRINTF(LOG_INFO, "network state changed: %d -> %d\n", g_network_state, state);
    g_network_state = state;

    if (g_network_state == RT_DSI_STATE_CALL_CONNECTED) {  // network connected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_CONNECTED, NULL, 0);
        card_detection_disable();

        if (*g_task_param.type == PROFILE_TYPE_PROVISONING) {
            rt_provsioning_forbit_ping();
        } else {
            rt_allow_ping();
        }
        rt_iptables_allow();

    } else if (g_network_state == RT_DSI_STATE_CALL_IDLE) {  // network disconnected
        msg_send_agent_queue(MSG_ID_BROAD_CAST_NETWORK, MSG_NETWORK_DISCONNECTED, NULL, 0);
        rt_iptables_forbid();
        card_detection_enable();
        g_network_state = NETWORK_STATE_NOT_READY;
    }

    #undef NETWORK_STATE_NOT_READY
}

#ifdef CFG_PLATFORM_9X07

/*
avoid this case:
boot -> exist provisoning profile dial up -> bootstrap start -> switch provisoning profile -> NO-NET -> dial up again
*/
static int32_t network_wait_bootstrap_start(int32_t max_delay_100ms)
{
    while (max_delay_100ms-- > 0) {
        if (card_manager_install_profile_ok()) {
            return RT_SUCCESS;
        }
        rt_os_msleep(DELAY_100MS);
    }

    return RT_ERROR;
}

static void network_detection_task(void *arg)
{
    int32_t ret;
    int32_t cnt = 0;
    dsi_call_info_t dsi_net_hndl;
    profile_type_e *type = ((task_param_t *)arg)->type;
    int32_t *profile_damaged = ((task_param_t *)arg)->profile_damaged;

    MSG_PRINTF(LOG_DBG, "start with profile (%d,%d) ...\r\n", *type, *profile_damaged);

    /* non-operational profile && share profile ok */
    if ((*type == PROFILE_TYPE_TEST || *type == PROFILE_TYPE_PROVISONING) && *profile_damaged == RT_SUCCESS) {
        network_wait_bootstrap_start(MAX_WAIT_BOOTSTRAP_TIME);
    }

    /* operational profile && share profile damaged */
    if (*type == PROFILE_TYPE_OPERATIONAL && *profile_damaged == RT_ERROR) {
        operational_network_start_timer();
    }

    /* set callback function */
    dial_up_set_dial_callback((void *)network_update_state);

    /* add retry more times */
    while (1) {
        /* init dial up */
        ret = dial_up_init(&dsi_net_hndl);
        rt_os_sleep(3);

        if (ret != RT_SUCCESS) {
            if (++cnt < MAX_INIT_RETRY_CNT) {
                MSG_PRINTF(LOG_ERR, "dial up init error (%d)\r\n", cnt);
                rt_os_sleep(3);
                continue;
            }
            MSG_PRINTF(LOG_ERR, "dial up init final error\r\n");
            goto  dial_up_init_error_entry;
        }
        cnt = 0;

        /* do dial up */
        dial_up_to_connect(&dsi_net_hndl);

        /* release dial up */
        dial_up_deinit(&dsi_net_hndl);

        rt_os_sleep(5);
    }

dial_up_init_error_entry:
    
    MSG_PRINTF(LOG_WARN, "sleep 10 seconds to reboot terminal ...\r\n");
    rt_os_sleep(10);
    rt_os_reboot();

exit_entry:

    rt_exit_task(NULL);
}

void network_force_down(void)
{
    /* force to make network down */
    dial_up_reset();
}

int32_t init_network_detection(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;

    g_task_param.type               = &(((public_value_list_t *)arg)->card_info->type);
    g_task_param.profile_damaged    = ((public_value_list_t *)arg)->profile_damaged;

    ret = rt_create_task(&task_id, (void *)network_detection_task, &g_task_param);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

#endif

#ifdef CFG_PLATFORM_ANDROID

void network_force_down(void)
{
    /* force to make network down */
    // TODO
}

int32_t init_network_detection(void *arg)
{
    (void)arg;
    return RT_SUCCESS;
}

#endif

