
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

uint16_t monitor_cmd(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    card_cmd(data, len, rsp, rsp_len);
}

int32_t main(void)
{
    printf("I am monitor\n");
    trigger_insert_card(1);
    trigegr_regist_reset(card_reset);
    trigegr_regist_cmd(card_cmd);
    ipc_regist_callback(monitor_cmd);
    ipc_socket_server();
}
