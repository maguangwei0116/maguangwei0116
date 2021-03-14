
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ipc_socket.h
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __IPC_SOCKET_SERVER_H__
#define __IPC_SOCKET_SERVER_H__

#include "rt_type.h"
#if SERVER_ADDR_EN
int32_t ipc_socket_server(const char *server_addr);
#else
int32_t ipc_socket_server(void);
#endif
void    ipc_regist_callback(void *fun);
rt_bool ipc_server_check(void);

#endif // __IPC_SOCKET_SERVER_H__

