
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
#include "socket.h"

int32_t socket_create(void)
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

int32_t socket_connect(int32_t socket_id, uint8_t *addr, int32_t port)
{
    struct sockaddr_in server_sai;
    server_sai.sin_family = AF_INET;
    server_sai.sin_port = htons(port);
    server_sai.sin_addr.s_addr = inet_addr(addr);
    memset(&(server_sai.sin_zero), 0, sizeof(server_sai.sin_zero));
    return connect(socket_id, (struct sockaddr *)&server_sai, sizeof(struct sockaddr));
}

int32_t socket_bind(int32_t socket_id, uint8_t *addr, int32_t port)
{
    int32_t on = 1;
    struct sockaddr_in server_sai;
    server_sai.sin_family = AF_INET;
    server_sai.sin_port = htons(port);
    server_sai.sin_addr.s_addr = htonl(addr);
    memset(&(server_sai.sin_zero), 0, sizeof(server_sai.sin_zero));
    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    return bind(socket_id, (struct sockaddr *)&server_sai, sizeof(struct sockaddr));
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
