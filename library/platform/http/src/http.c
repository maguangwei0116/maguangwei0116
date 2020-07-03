
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : http.c
 * Date        : 2017.09.01
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include "log.h"
#include "http.h"

#define HTTP_POST "POST %s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
    "Content-Type:application/json;charset=UTF-8\r\n"\
    "md5sum:%s\r\n"\
    "Content-Length: %d\r\n\r\n%s"
    
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"

int32_t http_tcpclient_create(const char *addr, int32_t port)
{
    struct sockaddr_in server_addr;
    int32_t socket_fd;
    struct in_addr ipAddr;
    struct timeval timeout = {HTTP_CONNECT_TIMEOUT, 0};

    ipAddr.s_addr           = inet_addr(addr);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(port);
    server_addr.sin_addr    = ipAddr;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        MSG_PRINTF(LOG_WARN, "socket error\n");
        return RT_ERROR;
    }

    /* set send/recv timeout */
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval));

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
        MSG_PRINTF(LOG_WARN, "connect fd=%d, error(%d)=%s\r\n", socket_fd, errno, strerror(errno));
        /* MUST: close this fd !!! */
        http_tcpclient_close(socket_fd);
        return RT_ERROR;
    }
    
    return socket_fd;
}

int32_t http_tcpclient_close(int32_t socket)
{
    if (socket < 0) {
        return RT_ERROR;
    }
    close(socket);

    return RT_SUCCESS;
}

int32_t http_parse_url(const char *url, char *host, char *file, int32_t *port)
{
    char *ptr1 = NULL;
    const char *ptr2 = NULL;
    char local_host[128] = {0};
    int32_t len = 0;
    
    if (!url || !host || !file || !port) {
        MSG_PRINTF(LOG_WARN, "url/host/file/port error\n");
        return RT_ERROR;
    }

    ptr1 = (char *)url;

    if (!rt_os_strncmp(ptr1, "http://", 7)) {
        ptr1 += 7;
    } else if (!rt_os_strncmp(ptr1, "https://", 8)) {
        ptr1 += 8;
    } else {
        MSG_PRINTF(LOG_WARN, "error\n");
        return RT_ERROR;
    }

    ptr2 = rt_os_strchr(ptr1, '/');
    if (ptr2) {
        len = rt_os_strlen(ptr1) - rt_os_strlen(ptr2);
        rt_os_memcpy(local_host, ptr1, len);
        local_host[len] = '\0';
        rt_os_strcpy(file, ptr2);
        file[rt_os_strlen(file)] = '\0';
    } else {
        rt_os_memcpy(local_host, ptr1, rt_os_strlen(ptr1));
        local_host[rt_os_strlen(ptr1)] = '\0';
    }
    
    //get host and ip
    ptr1 = rt_os_strchr(local_host, ':');
    if (ptr1) {
        *ptr1++ = '\0';
        *port = atoi(ptr1);
    } else {
        *port = MY_HTTP_DEFAULT_PORT;
    }
    rt_os_strcpy(host, local_host);

    return RT_SUCCESS;
}

void http_get_ip_addr(const char *domain, char *ip_addr)
{
    int32_t i;
    struct hostent *host = gethostbyname(domain);
    if (!host) {
        ip_addr = NULL;
        return;
    }
    
    for (i = 0; host->h_addr_list[i]; i++) {
        rt_os_strcpy(ip_addr, inet_ntoa( * (struct in_addr*) host->h_addr_list[i]));
        break;
    }
}

int32_t http_tcpclient_recv(int32_t socket, uint8_t *lpbuff, int32_t length)
{
    int32_t cnt = 0;
    fd_set   t_set1;
    struct timeval  tv;
    
    FD_ZERO(&t_set1);
    FD_SET(socket, &t_set1);
    tv.tv_sec = HTTP_RECV_TIMEOUT;
    tv.tv_usec = 0;
    if (select(socket + 1, &t_set1, NULL, NULL, &tv)>0) {
        cnt = (int32_t)read(socket, lpbuff, length);
    }
    return cnt;
}

int32_t http_tcpclient_send(int32_t socket, const uint8_t *buff, int32_t size)
{
    int32_t sent = 0;
    int32_t tmpres = 0;

    while (sent < size) {
        tmpres = write(socket, buff + sent, size - sent);
        if (tmpres == -1) {
            MSG_PRINTF(LOG_WARN, "tmpres is error\n");
            return RT_ERROR;
        }
        sent += tmpres;
    }
    return sent;
}

int32_t http_parse_result(const char *lpbuf)
{
    const char *ptmp = NULL;
    
    ptmp = rt_os_strstr(lpbuf, "HTTP/1.1");
    if (!ptmp) {
        MSG_PRINTF(LOG_WARN, "http/1.1 not faind\n");
        return RT_ERROR;
    }
    
    if (atoi(ptmp + 9) != 200) {
        MSG_PRINTF(LOG_WARN, "\n result:\n%s\n", lpbuf);
        return RT_ERROR;
    }

    ptmp = (int8_t*)rt_os_strstr(lpbuf, "\r\n\r\n");
    if (!ptmp) {
        MSG_PRINTF(LOG_WARN, "ptmp is NULL\n");
        return RT_ERROR;
    }
    
    if ((ptmp = rt_os_strstr(lpbuf, "{\"status\"")) == NULL) {
        return RT_ERROR;
    }
    return ptmp - lpbuf;
}

http_result_e http_post(const char *url, const char *post_str, const char *header, socket_call_back cb)
{
    int32_t socket_fd = -1;
    int8_t  *lpbuf = NULL;
    int32_t port = 0;
    int8_t  *file = NULL;
    int8_t  *host_addr = NULL;
    int32_t offset = 0;
    http_result_e ret = HTTP_SUCCESS;

    if (rt_os_strlen(post_str) >= BUFFER_SIZE * 4 || !url || !header || !cb) {
        return HTTP_PARAMETER_ERROR;
    }

    do {
        lpbuf = (int8_t *)rt_os_malloc(BUFFER_SIZE * 4);
        if (lpbuf == NULL) {
            ret = HTTP_SYSTEM_CALL_ERROR;
            MSG_PRINTF(LOG_WARN, "lpbuf memory alloc error\n");
            break;
        }
        rt_os_memset(lpbuf, '0', BUFFER_SIZE * 4);

        host_addr = (int8_t *)rt_os_malloc(HOST_ADDRESS_LEN);
        if (host_addr == NULL) {
            ret = HTTP_SYSTEM_CALL_ERROR;
            MSG_PRINTF(LOG_WARN, "lpbuf memory alloc error\n");
            break;
        }
        rt_os_memset(host_addr, '0', HOST_ADDRESS_LEN);

        file = (int8_t *)rt_os_malloc(HOST_PATH_LEN);
        if (file == NULL) {
            ret = HTTP_SYSTEM_CALL_ERROR;
            MSG_PRINTF(LOG_WARN, "file memory alloc error\n");
            break;
        }
        rt_os_memset(file, '0', HOST_PATH_LEN);

        if (!url || !post_str) {
            MSG_PRINTF(LOG_WARN, "failed!\n");
            break;
        }

        if (http_parse_url(url, host_addr, file, &port)) {
            MSG_PRINTF(LOG_WARN, "http parse url failed!\n");
            ret = HTTP_PARAMETER_ERROR;
            break;
        }

        MSG_PRINTF(LOG_WARN, "http pose host_addr : %s\n", host_addr);

        socket_fd = http_tcpclient_create(host_addr, port);       // connect network
        if (socket_fd < 0) {
            ret = HTTP_SOCKET_CONNECT_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient create failed\n");
            break;
        }
        snprintf(lpbuf, BUFFER_SIZE * 4, HTTP_POST, file, host_addr, port, header, rt_os_strlen(post_str), post_str);

        if (http_tcpclient_send(socket_fd,lpbuf, rt_os_strlen(lpbuf)) < 0) {      // send data
            ret = HTTP_SOCKET_SEND_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient send failed..\n");
            break;
        }
        rt_os_memset(lpbuf, 0, BUFFER_SIZE * 4);
        /*it's time to recv from server*/

        if (http_tcpclient_recv(socket_fd, lpbuf, BUFFER_SIZE * 4) <= 0) {     // recv data
            ret = HTTP_SOCKET_RECV_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient recv failed\n");
            break;
        } else {
             offset = http_parse_result(lpbuf);
             MSG_PRINTF(LOG_DBG, "%s\n", lpbuf + offset);
             if (cb(lpbuf + offset) != 0) {
                ret = HTTP_RESPOND_ERROR;
             }
        }
    } while(0);

    rt_os_free(file);
    rt_os_free(host_addr);
    rt_os_free(lpbuf);

    http_tcpclient_close(socket_fd);

    return ret;
}

http_result_e http_post_raw(const char *host_ip, int32_t port, void *buffer, int32_t len, socket_call_back cb)
{
    int32_t socket_fd = -1;
    int8_t  *lpbuf = NULL;
    char convert_ip[128] = {0};
    int32_t offset = 0;
    http_result_e ret = HTTP_SUCCESS;

    if (len >= BUFFER_SIZE * 4 || !cb) {
        return HTTP_PARAMETER_ERROR;
    }

    do {
        lpbuf = (int8_t *)rt_os_malloc(BUFFER_SIZE * 4);
        if (lpbuf == NULL) {
            ret = HTTP_SYSTEM_CALL_ERROR;
            MSG_PRINTF(LOG_WARN, "lpbuf memory alloc error\n");
            break;
        }
        rt_os_memset(lpbuf, '0', BUFFER_SIZE * 4);

        http_get_ip_addr(host_ip, convert_ip);
        MSG_PRINTF(LOG_TRACE, "convert_ip:%s, port:%d\r\n", convert_ip, port);

        socket_fd = http_tcpclient_create(convert_ip, port);       // connect network
        if (socket_fd < 0) {
            ret = HTTP_SOCKET_CONNECT_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient create failed\n");
            break;
        }

        if (http_tcpclient_send(socket_fd, buffer, len) < 0) {      // send data
            ret = HTTP_SOCKET_SEND_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient send failed..\n");
            break;
        }

        /*it's time to recv from server*/
        rt_os_memset(lpbuf, 0, BUFFER_SIZE * 4);        
        if (http_tcpclient_recv(socket_fd, lpbuf, BUFFER_SIZE * 4) <= 0) {     // recv data
            ret = HTTP_SOCKET_RECV_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient recv failed\n");
            break;
        } else {
            offset = http_parse_result(lpbuf);
            MSG_PRINTF(LOG_DBG, "%s\n", lpbuf + offset);
            if (cb(lpbuf + offset) != 0) {
                ret = HTTP_RESPOND_ERROR;
            }
        }
    } while(0);

    rt_os_free(lpbuf);

    http_tcpclient_close(socket_fd);

    return ret;
}

