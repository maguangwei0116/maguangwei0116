
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

#include "rt_type.h"
#include "ipc_socket_server.h"
#include "trigger.h"
#include "card.h"
#include "esim_api.h"

uint16_t monitor_cmd(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint16_t cmd = 0;
    softsim_logic_command(1, data, len, rsp, rsp_len);
    cmd = (data[5] << 8) + data[6];
    if ((cmd == 0xBF31) || (cmd == 0xFF7F)) { // enable card command
        trigger_swap_card(1);
    }
}

int32_t main(void)
{
    softsim_logic_start(write_log_fun);
    trigegr_regist_reset(card_reset);
    trigegr_regist_cmd(card_cmd);
    trigger_insert_card(1);
    ipc_regist_callback(monitor_cmd);
    ipc_socket_server();
}
