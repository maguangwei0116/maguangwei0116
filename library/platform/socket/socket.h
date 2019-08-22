
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

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "rt_type.h"

int32_t socket_create(void);
int32_t socket_connect(int32_t socket_id);
int32_t socket_bind(int32_t socket_id);
int32_t socket_listen(int32_t socket_id, int32_t num);
int32_t socket_accept(int32_t socket_id);
int32_t socket_recv(int32_t socket_id, uint8_t *buf, int32_t buf_size);
int32_t socket_send(int32_t socket_id, const uint8_t *buf, int32_t size);
int32_t socket_close(int32_t socket_id);

#endif // __SOCKET_H__
