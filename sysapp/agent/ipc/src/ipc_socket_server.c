
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ipc_socket_server.c
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __IPC_SOCKET_SERVER__
#define __IPC_SOCKET_SERVER__

#include "socket.h"

#define THE_MAX_CLIENT_NUM  2
int32_t ipc_server(void)
{
    int32_t socket_id = -1;
    int32_t ret = RT_ERROR;
    int32_t new_fd = -1;
    char buffer[1024];

    socket_id = socket_create();
    MSG_PRINTF(LOG_INFO, "socket is:%d\n", socket_id);
    if (socket_id <= 0) {
        return ret;
    }
    ret = socket_bind(socket_id);
    if (ret == -1) {
        MSG_PRINTF(LOG_ERR, "socket bind failed\n");
        goto end;
    }
    ret = socket_listen(socket_id, THE_MAX_CLIENT_NUM);
    if (ret == -1) {
        MSG_PRINTF(LOG_ERR, "socket listen failed\n");
        goto end;
    }
    while (1) {
        new_fd = socket_accept(socket_id);
        MSG_PRINTF(LOG_INFO, "client socket id:%d\n",new_fd);
        if (new_fd == -1) {
            break;
        }
        socket_recv(new_fd, buffer, 1024);
        MSG_PRINTF(LOG_INFO, "buf:%s\n",buffer);
        //socket_send();
        close(new_fd);
    }
end:
    socket_close(socket_id);
}

#endif // __IPC_SOCKET_SERVER__
