
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

static uint8_t g_rsp[2048];
static uint16_t g_rsp_len;

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
    static uint8_t cnt = 0, left = 0;
    static uint8_t *rsp_buf = g_rsp;

    cmd = (data[5] << 8) + data[6];
    if (cmd == 0xFFFF) { // msg from agent
        MSG_PRINTF(LOG_INFO, "Receive msg from agent,uicc type:%s\r\n", (data[7] == 0x00) ? "vUICC":"eUICC");
        if (data[7] == 0x00) {  // used vuicc
            trigegr_regist_reset(card_reset);
            trigegr_regist_cmd(card_cmd);
            trigger_insert_card(1);
        }

    } else {
        MSG_PRINTF(LOG_INFO, "Receive msg from lpa\r\n");
        // for get response
        if (data[1] == 0xC0) {
            if (cnt != 0) {
                rt_os_memcpy(rsp, rsp_buf, 256);
                rsp[256] = 0x61;
                rsp[257] = 0x00;
                *rsp_len = 258;
                cnt --;
                rsp_buf += 256;
            } else if (left != 0) {
                rt_os_memcpy(rsp, rsp_buf, left);
                rsp[left] = 0x90;
                rsp[left + 1] = 0x00;
                *rsp_len = left + 2;
                left = 0;
                rsp_buf = g_rsp;
            }
            return 0;
        } else {
            cnt = 0;
            left = 0;
            rsp_buf = g_rsp;
        }
        // send apdu
        softsim_logic_command(1, data, len, g_rsp, &g_rsp_len);  // msg from lpa
        if ((cmd == 0xBF31) || (cmd == 0xFF7F)) {
            trigger_swap_card(1);
        }
        // for 61xx
        if (g_rsp_len == 2) {
            rt_os_memcpy(rsp, g_rsp, g_rsp_len);
            *rsp_len = g_rsp_len;
            return 0;
        }
        if (g_rsp_len > 256) {
            rsp[0] = 0x61;
            rsp[1] = 0x00;
            *rsp_len = 2;
        } else {
            rsp[0] = 0x61;
            rsp[1] = g_rsp_len - 2;
            *rsp_len = 2;
        }
        cnt = (g_rsp_len - 2) / 256;
        left = (g_rsp_len - 2) % 256;
    }
}

int32_t main(void)
{
    MSG_PRINTF(LOG_WARN, "App version: %s\n", LOCAL_TARGET_RELEASE_VERSION_NAME);
    softsim_logic_start(log_print);
    init_system_signal(NULL);
    ipc_regist_callback(monitor_cmd);
    ipc_socket_server();
}
