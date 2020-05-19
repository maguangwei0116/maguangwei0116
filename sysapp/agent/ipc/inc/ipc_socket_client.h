
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ipc_socket_server.h
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __IPC_SOCKET_CLIENT__
#define __IPC_SOCKET_CLIENT__

#include "rt_type.h"

int32_t init_ipc_client(void *arg);
int32_t ipc_send_data(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len);

#endif // __IPC_SOCKET_CLIENT__
