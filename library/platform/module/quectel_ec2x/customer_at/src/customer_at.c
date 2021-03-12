
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
#include "rt_os.h"
#include "customer_at.h"
/* include quectel library header file */
#include "ql_customer_at.h"

#define AT_REQ_SELECT_TIMEOUT   5
#define MAX_CUSTOMER_AT_NUM     6

static at_cmd_t g_at_cmd[MAX_CUSTOMER_AT_NUM];
static int32_t g_at_cmd_cnt = 0;
static int32_t g_at_sockfd = -1;

static int32_t atcommand_handler(int32_t sockfd, struct sockaddr_un *un, const char *ptr, int32_t len)
{
    int32_t retval = RT_ERROR;
    int32_t index = 0;
    int32_t label_len = 0;
    const char *label = NULL;
    atcommand_callback func = NULL;
    char response[1024] = {0};

    for (index = 0; index < g_at_cmd_cnt; index++) {
        label = g_at_cmd[index].label;
        func = g_at_cmd[index].handle;
        label_len = rt_os_strlen(label);
        
        if (!rt_os_strncasecmp(ptr, label, label_len) && func) {
            snprintf(response, sizeof(response), "%s", label);
            retval = func((char *)(ptr + label_len), &response[label_len], sizeof(response)-label_len);
        }
    }

    if (retval == RT_ERROR) {
        snprintf(response, sizeof(response), "%s", "ERROR");
    }
    
    MSG_PRINTF(LOG_DBG, "at req: %s\n", ptr);
    MSG_PRINTF(LOG_DBG, "at rsp: %s\n", response);
    retval = ql_customer_at_send_response(sockfd, un, true, response, rt_os_strlen(response));
    if (retval) {
        MSG_PRINTF(LOG_ERR, "send at command failed\n");
    }

    return retval;
}

static void customer_at_listen_sock(int32_t *sockfd)
{
    int32_t retval;
    fd_set rfds;
    struct timeval tv;
    struct sockaddr_un un;
    char buf[1024];

    /* check socket fd */
    if (!sockfd && *sockfd < 0) {
        MSG_PRINTF(LOG_ERR, "sockfd error\n");
        goto exit_entry;
    }
    
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(*sockfd, &rfds);

        tv.tv_sec = AT_REQ_SELECT_TIMEOUT;
        tv.tv_usec = 0;

        retval = select(*sockfd + 1, &rfds, NULL, NULL, &tv);
        if (retval > 0) {
            if(FD_ISSET(*sockfd, &rfds)) {
                rt_os_memset(buf, 0, sizeof(buf));
                rt_os_memset(&un, 0, sizeof(un));
                if (ql_customer_at_get_request(*sockfd, &un, buf, sizeof(buf))) {
                    MSG_PRINTF(LOG_ERR, "get at command request failed\n");
                    continue;
                }
                atcommand_handler(*sockfd, &un, (const char *)buf, rt_os_strlen(buf));
            }
        } else if (retval < 0){
            MSG_PRINTF(LOG_ERR, "select error(%d)=%s\n", errno, strerror(errno));
        }
    }

exit_entry:

    rt_exit_task(NULL);
    g_at_sockfd = -1;
}

static void customer_at_regist_callback(void *arg)
{
    at_cmd_t *at_cmd = (at_cmd_t *)arg;

    /* add "" is needed !!! */
    snprintf(g_at_cmd[g_at_cmd_cnt].label, sizeof(g_at_cmd[g_at_cmd_cnt].label), "\"%s\"", at_cmd->label);
    g_at_cmd[g_at_cmd_cnt].handle = at_cmd->handle;
    g_at_cmd_cnt++;  
}

int32_t init_customer_at(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_SUCCESS;

    if (g_at_sockfd < 0) {
        g_at_sockfd = ql_customer_at_get_socket();
        if (g_at_sockfd < 0) {
            MSG_PRINTF(LOG_ERR, "Can not get socket fd\n");
            ret = RT_ERROR;
        } else {
            ret = rt_create_task(&task_id, (void *)customer_at_listen_sock, &g_at_sockfd);
            if (ret != RT_SUCCESS) {
                MSG_PRINTF(LOG_ERR, "create task fail\n");
                ret = RT_ERROR;
            }
        }
    }

    customer_at_regist_callback(arg);

    return ret;
}
