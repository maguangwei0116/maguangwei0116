
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

extern int init_file_ops(void);

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
    static rt_bool reset_flag = RT_FALSE;

    cmd = (data[5] << 8) + data[6];
    if (cmd == 0xFFFF) { // msg from agent
        MSG_PRINTF(LOG_INFO, "Receive msg from agent,uicc type:%s\r\n", (data[7] == 0x00) ? "vUICC":"eUICC");
        if (data[7] == 0x00) {  // used vuicc
            trigegr_regist_reset(card_reset);
            trigegr_regist_cmd(card_cmd);
            trigger_swap_card(1);
        }

    } else {
        MSG_PRINTF(LOG_INFO, "Receive msg from lpa\r\n");
        // send apdu
        softsim_logic_command(1, data, len, rsp, rsp_len);  // msg from lpa
        if ((cmd == 0xBF31) || (cmd == 0xFF7F)) {
            cmd = (data[0] << 8) + data[1];
            if ((cmd & 0xFF00) == 0x6100) {
                reset_flag = RT_TRUE;
                return RT_SUCCESS;
            } else {
                reset_flag = RT_TRUE;
            }
        }
        if (reset_flag == RT_TRUE) {
            trigger_swap_card(1);
        }
    }
    return RT_SUCCESS;
}

int32_t main(void)
{
    MSG_PRINTF(LOG_WARN, "App version: %s\n", LOCAL_TARGET_RELEASE_VERSION_NAME);
    init_file_ops();
    softsim_logic_start(write_log_fun);
    init_system_signal(NULL);
    ipc_regist_callback(monitor_cmd);
    ipc_socket_server();
}
