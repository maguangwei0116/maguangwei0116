
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : main.c
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include <stdlib.h>
#include "rt_type.h"
#include "ipc_socket_server.h"
#include "trigger.h"
#include "card.h"
#include "esim_api.h"

static void cfinish(int32_t sig)
{
    MSG_PRINTF(LOG_DBG, "recv signal %d, process exit !\r\n", sig);
    rt_os_signal(RT_SIGINT, NULL);
    exit(-1);
}

static int32_t init_system_signal(void *arg)
{
    rt_os_signal(RT_SIGINT, cfinish);
    return RT_SUCCESS;
}

uint16_t monitor_cmd(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint16_t cmd = 0;
    cmd = (data[5] << 8) + data[6];
    if (cmd == 0xFFFF) { // msg from agent
        MSG_PRINTF(LOG_INFO, "Receive msg from agent\r\n");
        trigegr_regist_reset(card_reset);
        trigegr_regist_cmd(card_cmd);
        trigger_insert_card(1);
    } else {
        MSG_PRINTF(LOG_INFO, "Receive msg from lpa\r\n");
        softsim_logic_command(1, data, len, rsp, rsp_len);  // msg from lpa
        if ((cmd == 0xBF31) || (cmd == 0xFF7F)) {
            trigger_swap_card(1);
        }
    }
}

int32_t main(void)
{
    MSG_PRINTF(LOG_WARN, "App version: %s\n", RELEASE_TARGET_VERSION);
    softsim_logic_start(write_log_fun);
    init_system_signal(NULL);
    ipc_regist_callback(monitor_cmd);
    ipc_socket_server();
}
