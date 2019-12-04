
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

#ifndef __IPC_SOCKET_CLIENT_H__
#define __IPC_SOCKET_CLIENT_H__

#include "rt_type.h"

int32_t ipc_socket_server(void);
void    ipc_regist_callback(void *fun);
rt_bool ipc_server_check(void);

#endif // __IPC_SOCKET_CLIENT_H__

