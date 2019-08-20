
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : socket.c
 * Date        : 2018.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/un.h>
#include "socket.h"

#define SERVER_PATH     "/data/redtea/server"

int32_t socket_create(void)
{
    return socket(AF_UNIX, SOCK_STREAM, 0);
}

int32_t socket_connect(int32_t socket_id)
{
    struct sockaddr_un server_sai;
    memset(&server_sai, 0, sizeof(server_sai));
    server_sai.sun_family = AF_UNIX;
    strcpy(server_sai.sun_path, SERVER_PATH);
    return connect(socket_id, (struct sockaddr_un *)&server_sai, sizeof(struct sockaddr_un));
}

int32_t socket_bind(int32_t socket_id)
{
    int32_t on = 1;
    rt_os_unlink(SERVER_PATH);
    struct sockaddr_un server_sai;
    memset(&server_sai, 0, sizeof(server_sai));
    server_sai.sun_family = AF_UNIX;
    strcpy(server_sai.sun_path, SERVER_PATH);
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    return bind(socket_id, (struct sockaddr_un *)&server_sai, sizeof(struct sockaddr_un));
}

int32_t socket_listen(int32_t socket_id, int32_t num)
{
    return listen(socket_id, num);
}

int32_t socket_accept(int32_t socket_id)
{
    struct sockaddr_in client_sai;
    int32_t addrlen = sizeof(struct sockaddr);
    return accept(socket_id, (struct sockaddr *)&client_sai, (socklen_t *)&addrlen);
}

int32_t socket_recv(int32_t socket_id, uint8_t *buf, int32_t buf_size)
{
    return recv(socket_id, buf, buf_size, 0);
}

int32_t socket_send(int32_t socket_id, uint8_t *buf, int32_t size)
{
    return send(socket_id, buf, size, 0);
}

int32_t socket_close(int32_t socket_id)
{
    return close(socket_id);
}
