
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ipc_socket.c
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "ipc_socket_client.h"
#include "socket.h"

int32_t ipc_conect_server(void)
{
    int32_t socket_id = -1;
    int32_t ret = RT_ERROR;

    socket_id = socket_create();
    MSG_PRINTF(LOG_INFO, "socket id %d\n", socket_id);
    if (socket_id <= 0) {
        return RT_ERROR;
    }
    ret = socket_connect(socket_id);
    if (ret == -1) {
        MSG_PRINTF(LOG_ERR, "connet server failed\n");
    }
    ret = socket_send(socket_id, "hello", sizeof("hello"));
    if (ret == -1) {
        MSG_PRINTF(LOG_ERR, "send data failed\n");
    }
    socket_close(socket_id);
}
