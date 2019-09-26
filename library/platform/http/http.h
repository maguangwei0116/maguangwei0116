
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : http.h
 * Date        : 2017.09.01
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __HTTP_H__
#define __HTTP_H__

#include "rt_type.h"
#include "rt_os.h"

#define MY_HTTP_DEFAULT_PORT       80
#define BUFFER_SIZE                1024
#define TRY_RECEVICE_TIMER         5
#define HOST_ADDRESS_LEN           100
#define HOST_PATH_LEN              100

typedef int (*socket_call_back)(char *p);

typedef enum HTTP_RESULT {
    HTTP_SUCCESS = 0,           // 0
    HTTP_SYSTEM_CALL_ERROR,     // 1
    HTTP_PARAMETER_ERROR,       // 2
    HTTP_SOCKET_CONNECT_ERROR,  // 3
    HTTP_SOCKET_SEND_ERROR,     // 4
    HTTP_SOCKET_RECV_ERROR,     // 5
    HTTP_RESPOND_ERROR,         // 6
} http_result_e;

http_result_e http_post(const int8_t *url,int8_t *post_str,int8_t *header,socket_call_back cb);
int32_t http_parse_result(int8_t *lpbuf);
int32_t http_tcpclient_recv(int32_t socket,int8_t *lpbuff,int32_t length);
int32_t http_tcpclient_create(const int8_t *addr, int32_t port);
void http_tcpclient_close(int32_t socket);
int32_t http_parse_url(const int8_t *url,int8_t *host,int8_t *file,int32_t *port);

#endif  // __HTTP_H__
