
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

#include "ipc_socket_client.h"
#include "socket.h"

#define MAT_SOCKET_BUFFER   1024

int32_t ipc_send_data(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    int32_t socket_id = -1;
    int32_t ret = RT_ERROR;

    socket_id = socket_create();
    if (socket_id <= 0) {
        return RT_ERROR;
    }


    ret = socket_connect(socket_id);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "connet server failed\n");
        goto end;
    }
    MSG_INFO_ARRAY("IPC-APDU REQ: ", data, len);
    ret = socket_send(socket_id, data, len);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "send data failed\n");
        goto end;
    }

    ret = socket_recv(socket_id, rsp, MAT_SOCKET_BUFFER);
    if (ret <= 0) {
        MSG_PRINTF(LOG_ERR, "recv data failed, ret=%d\n", ret);
        ret = RT_ERROR;
        goto end;
    }
    *rsp_len = ret;
    ret = RT_SUCCESS;
    MSG_INFO_ARRAY("IPC-APDU RSP: ", rsp, *rsp_len);
end:

    socket_close(socket_id);
    return ret;
}
