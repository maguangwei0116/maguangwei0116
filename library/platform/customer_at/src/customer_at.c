
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
/* include Quectel library header file */
#include "ql_customer_at.h"

#define AT_COMMAND_TBL "\"test\""

static int test_flag = 0;

static void atcommand_handler(int sockfd, struct sockaddr_un *un, char *ptr, int len)
{
    int retval;
    char response[32] = { 0 };

    MSG_PRINTF(LOG_INFO, "ptr: %s\n", ptr);

    if (0 == strncasecmp(ptr, AT_COMMAND_TBL, strlen(AT_COMMAND_TBL))) {
        if (',' == *(ptr + strlen(AT_COMMAND_TBL))) {
            test_flag = atoi(ptr + strlen(AT_COMMAND_TBL) + 1);
            MSG_PRINTF(LOG_INFO, "set test is: %d\n", test_flag);
            retval = ql_customer_at_send_response(sockfd, un, true, NULL, 0);
        } else {
            snprintf(response, sizeof(response)-1, "\"test\",%d", test_flag);
            MSG_PRINTF(LOG_INFO, "read test is: %s\n", response);
            retval = ql_customer_at_send_response(sockfd, un, true, response, strlen(response));
        }

        if (retval) {
            MSG_PRINTF(LOG_INFO, "send at command failed\n");
        }
    }
}

static void customer_at_listen_sock(int *sockfd)
{
    int retval;
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

int32_t init_customer_at(void *arg)
{
    int32_t sockfd = -1;
    rt_task task_id = 0;
    int32_t ret = RT_SUCCESS;

    sockfd = ql_customer_at_get_socket();
    if (sockfd < 0) {
        MSG_PRINTF(LOG_INFO, "Can not get socket fd\n");
        ret = RT_ERROR;
    } else {
        ret = rt_create_task(&task_id, (void *)customer_at_listen_sock, &sockfd);
        if (ret != RT_SUCCESS) {
            MSG_PRINTF(LOG_ERR, "create task fail\n");
            ret = RT_ERROR;
        }
    }
    return ret;
}
