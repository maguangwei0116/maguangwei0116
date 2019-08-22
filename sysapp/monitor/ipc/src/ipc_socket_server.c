
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

#include "ipc_socket_server.h"
#include "socket.h"

#define THE_MAX_CLIENT_NUM  2

typedef uint16_t (*ipc_callback)(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len);
ipc_callback ipc_cmd;

void ipc_regist_callback(void *fun)
{
    ipc_cmd = (ipc_callback)fun;
}

int32_t ipc_socket_server(void)
{
    int32_t socket_id = -1;
    int32_t ret = RT_ERROR;
    int32_t new_fd = -1;
    uint8_t buffer[512];
    uint8_t rsp[512];
    uint16_t rsp_len;
    uint16_t rcv_len;

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
        rcv_len = socket_recv(new_fd, buffer, 1024);
        ipc_cmd(buffer, rcv_len, rsp, &rsp_len);
        socket_send(new_fd, rsp, rsp_len);
        close(new_fd);
    }
end:
    socket_close(socket_id);
}
