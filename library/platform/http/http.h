
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "rt_type.h"
#include "rt_os.h"

#define MY_HTTP_DEFAULT_PORT    80
#define BUFFER_SIZE             1024
#define TRY_RECEVICE_TIMER      5
#define HOST_ADDRESS_LEN        100
#define HOST_PATH_LEN           100
#define HTTP_CONNECT_TIMEOUT    30
#define HTTP_RECV_TIMEOUT       60

typedef int32_t (*socket_call_back)(const char *p);

typedef enum HTTP_RESULT {
    HTTP_SUCCESS = 0,           // 0
    HTTP_SYSTEM_CALL_ERROR,     // 1
    HTTP_PARAMETER_ERROR,       // 2
    HTTP_SOCKET_CONNECT_ERROR,  // 3
    HTTP_SOCKET_SEND_ERROR,     // 4
    HTTP_SOCKET_RECV_ERROR,     // 5
    HTTP_RESPOND_ERROR,         // 6
} http_result_e;

extern http_result_e http_post(const char *url, const char *post_str, const char *header, socket_call_back cb);
extern http_result_e http_post_raw(const char *host_ip, int32_t port, void *buffer, int32_t len, socket_call_back cb);
extern int32_t       http_tcpclient_create(const char *addr, int32_t port);
extern int32_t       http_tcpclient_send(int32_t socket, const uint8_t *buff, int32_t size);
extern int32_t       http_tcpclient_recv(int32_t socket, uint8_t *lpbuff, int32_t length);
extern int32_t       http_tcpclient_close(int32_t socket);
extern int32_t       http_parse_result(const char *lpbuf);
extern int32_t       http_parse_url(const char *url, char *host, char *file, int32_t *port);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __HTTP_H__ */

