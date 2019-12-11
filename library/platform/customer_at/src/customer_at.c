
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : customer_at.c
 * Date        : 2019.12.09
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "rt_type.h"
#include "customer_at.h"
/* include quectel library header file */
#include "ql_customer_at.h"

#define AT_COMMAND_TBL "\"redtea\""

typedef int32_t *atcommand_callback(char *cmd, char *rsp);

atcommand_callback *atcommand_fun;

static int32_t g_sockfd = -1;
static void atcommand_handler(int32_t sockfd, struct sockaddr_un *un, char *ptr, int32_t len)
{
    int32_t retval;
    char response[1024] = { 0 };

    if (0 == strncasecmp(ptr, AT_COMMAND_TBL, strlen(AT_COMMAND_TBL))) {
        retval = atcommand_fun(ptr + strlen(AT_COMMAND_TBL), response);
        if (retval == RT_ERROR) {
            rt_os_memcpy(response, "error", 5);
            response[5] = '\0';
        }
        MSG_PRINTF(LOG_INFO, "read test is: %s\n", response);
        retval = ql_customer_at_send_response(sockfd, un, true, response, strlen(response));
        if (retval) {
            MSG_PRINTF(LOG_INFO, "send at command failed\n");
        }
    }
}

static void customer_at_listen_sock(int32_t *sockfd)
{
    int32_t retval;
    fd_set rfds;
    struct timeval tv;
    struct sockaddr_un un;
    char buf[1024];

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(*sockfd, &rfds);

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(*sockfd + 1, &rfds, NULL, NULL, &tv);
        if (retval > 0) {
            if(FD_ISSET(*sockfd, &rfds)) {
                memset(buf, 0, sizeof(buf));
                memset(&un, 0, sizeof(un));
                if (ql_customer_at_get_request(*sockfd, &un, buf, sizeof(buf))) {
                    MSG_PRINTF(LOG_INFO, "get at command request failed\n");
                    continue;
                }
                atcommand_handler(*sockfd, &un, buf, strlen(buf));
            }
        } else if (retval < 0){
            MSG_PRINTF(LOG_INFO, "select error: %s\n", strerror(errno));
        }
    }
}

static void customer_at_regist_callback(atcommand_callback *fun)
{
    atcommand_fun = fun;
}

int32_t init_customer_at(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_SUCCESS;

    g_sockfd = ql_customer_at_get_socket();
    if (g_sockfd < 0) {
        MSG_PRINTF(LOG_INFO, "Can not get socket fd\n");
        ret = RT_ERROR;
    } else {
        ret = rt_create_task(&task_id, (void *)customer_at_listen_sock, &g_sockfd);
        if (ret != RT_SUCCESS) {
            MSG_PRINTF(LOG_ERR, "create task fail\n");
            ret = RT_ERROR;
        }
    }
    customer_at_regist_callback((atcommand_callback *)arg);
    return ret;
}
