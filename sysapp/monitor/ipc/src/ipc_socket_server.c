
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
#include <errno.h>
#include <string.h>

#include "ipc_socket_server.h"
#include "rt_os.h"
#include "socket.h"

#define THE_MAX_CLIENT_NUM      2
#define THT_BUFFER_LEN          512

typedef uint16_t (*ipc_callback)(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len);
static ipc_callback ipc_cmd;
static rt_bool g_ipc_server_ok = RT_FALSE;

void ipc_regist_callback(void *fun)
{
    ipc_cmd = (ipc_callback)fun;
}

rt_bool ipc_server_check(void)
{
    return g_ipc_server_ok;
}

int32_t ipc_socket_server(void)
{
    int32_t socket_id = -1;
    int32_t ret = RT_ERROR;
    int32_t new_fd = -1;
    uint8_t buffer[THT_BUFFER_LEN];
    uint8_t rsp[THT_BUFFER_LEN];
    uint16_t rsp_len = 0;
    uint16_t rcv_len;

    socket_id = socket_create();
    if (socket_id <= 0) {
        return ret;
    }

    ret = socket_bind(socket_id);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "socket bind failed, sock_id=%d, err(%d)=%s\n", socket_id, errno, strerror(errno));
        rt_os_sleep(2);
        MSG_PRINTF(LOG_ERR, "monitor restart ...\r\n");
        /*
        Usual bind error: [err(98)=Address already in use]
        And set SO_REUSEADDR by setsockopt API doesn't work but restart monitor process !
        */
        rt_os_exit(-1);
    }

    ret = socket_listen(socket_id, THE_MAX_CLIENT_NUM);
    if (ret == -1) {
        MSG_PRINTF(LOG_ERR, "socket listen failed\n");
        goto end;
    }

    MSG_PRINTF(LOG_INFO, "monitor server socket_id:%d\r\n", socket_id);

    g_ipc_server_ok = RT_TRUE;

    while (1) {
        new_fd = socket_accept(socket_id);
        if (new_fd == -1) {
            break;
        }
        rcv_len = socket_recv(new_fd, buffer, THT_BUFFER_LEN);
        rt_os_memset(rsp, 0x00, THT_BUFFER_LEN);
        //MSG_INFO_ARRAY(" IPC SERVER REQ: ", buffer, rcv_len);
        ipc_cmd((const uint8_t *)buffer, rcv_len, rsp, &rsp_len);
        socket_send(new_fd, rsp, rsp_len);
        close(new_fd);
        rt_os_memset(buffer, 0x00, THT_BUFFER_LEN);
    }

end:
    socket_close(socket_id);

    return ret;
}


