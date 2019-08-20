
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

uint16_t monitor_cmd(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    MSG_PRINTF(LOG_INFO, "data:%s\n", data);
    *rsp_len = 10;
    rt_os_memcpy(rsp, "helloworld", *rsp_len);
}

int32_t main(void)
{
    ipc_regist_callback(monitor_cmd);
    ipc_socket_server();
}
