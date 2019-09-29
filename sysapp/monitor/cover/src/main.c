
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : main.c
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "rt_type.h"
#include "trigger.h"
#include "card.h"
#include "ipc_socket_server.h"

#define RT_AGENT_WAIT_MONITOR_TIME  3
#define RT_AGENT_PTROCESS           "agent"
#define RT_AGENT_FILE               "/usr/bin/agent"
#define RT_MONITOR_LOG              "/data/redtea/rt_monitor_log"
#define RT_MONITOR_LOG_MAX_SIZE     (1 * 1024 * 1024)

extern int init_file_ops(void);
extern int vsim_get_ver(char *version);

static void cfinish(int32_t sig)
{
    MSG_PRINTF(LOG_DBG, "recv signal %d, process exit !\r\n", sig);
    rt_os_signal(RT_SIGINT, NULL);
    exit(RT_ERROR);
}

static int32_t init_system_signal(void *arg)
{
    rt_os_signal(RT_SIGINT, cfinish);

    return RT_SUCCESS;
}

uint16_t monitor_cmd(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint16_t cmd = 0;
    uint16_t sw = 0;
    static rt_bool reset_flag = RT_FALSE;

    cmd = (data[5] << 8) + data[6];
    if (cmd == 0xFFFF) { // msg from agent
        MSG_PRINTF(LOG_INFO, "Receive msg from agent,uicc type:%s\r\n", (data[7] == 0x00) ? "vUICC" : "eUICC");
        if (data[7] == 0x00) {  // used vuicc
            trigegr_regist_reset(card_reset);
            trigegr_regist_cmd(card_cmd);
            trigger_swap_card(1);
            *rsp_len = 0;
        }
    } else {
        MSG_INFO_ARRAY("E-APDU REQ:", data, len);
        sw = card_cmd((uint8_t *)data, len, rsp, rsp_len);
        rsp[(*rsp_len)++] = (sw >> 8) & 0xFF;
        rsp[(*rsp_len)++] = sw & 0xFF;
        MSG_INFO_ARRAY("E-APDU RSP:", rsp, *rsp_len);
        if ((cmd == 0xBF31) || (cmd == 0xFF7F)) {
            cmd = (rsp[0] << 8) + rsp[1];
            if ((cmd & 0xFF00) == 0x6100) {
                reset_flag = RT_TRUE;
                return RT_SUCCESS;
            } else {
                reset_flag = RT_TRUE;
            }
        }

        if (reset_flag == RT_TRUE) {
            reset_flag = RT_FALSE;
            trigger_swap_card(1);
        }
    }

    return RT_SUCCESS;
}

static void ipc_socket_server_task(void)
{
    ipc_socket_server();
}

static int32_t ipc_socket_server_start(void)
{
    rt_task task_id = 0;
    int32_t ret;

    ret = rt_create_task(&task_id, (void *)ipc_socket_server_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        ret = RT_ERROR;
    }
    return ret;
}

static int32_t agent_task_check_start(void)
{
    int32_t ret;
    int32_t status;
    char cmd[128];
    pid_t child_pid;
    pid_t ret_pid;

    /* kill all agent processes */
    snprintf(cmd, sizeof(cmd), "killall -9 %s > /dev/null 2>&1", RT_AGENT_PTROCESS);
    system(cmd);

    /* start up agent process by fork fucntion */
    child_pid = fork();
    if (child_pid < 0) {
        MSG_PRINTF(LOG_WARN, "error in fork, err(%d)=%s\r\n", errno, strerror(errno));
    } else if (child_pid == 0) {
        MSG_PRINTF(LOG_INFO, "I am the child process, my process id is %d\r\n", getpid());
        ret = execl(RT_AGENT_FILE, RT_AGENT_PTROCESS, NULL);
        if (ret < 0) {
            MSG_PRINTF(LOG_ERR, "Excute agent fail, ret=%d, err(%d)=%s\n", ret, errno, strerror(errno));
        }
        exit(0);
    } else {
        MSG_PRINTF(LOG_INFO, "I am the parent process, my process id is %d, child_pid is %d\r\nn", getpid(), child_pid);

        /* block to wait designative child process's death */
        while (1) {
            ret_pid = waitpid(child_pid, &status, 0);
            if (ret_pid == child_pid) {
                MSG_PRINTF(LOG_WARN, "wait designative pid (%d) died, agent process died !\r\n", child_pid);
                break;
            }
            MSG_PRINTF(LOG_WARN, "wait pid (%d) died !\r\n", ret_pid);
        }
    }

    return RT_SUCCESS;
}

static void init_app_version(void *arg)
{
    uint8_t version[100];

    MSG_PRINTF(LOG_WARN, "App version: %s\n", LOCAL_TARGET_RELEASE_VERSION_NAME);
    vsim_get_ver(version);
    MSG_PRINTF(LOG_WARN, "vUICC version: %s\n", version);
}

int32_t main(int32_t argc, const char *argv[])
{
    log_mode_e def_mode = LOG_PRINTF_TERMINAL;

    /* check input param to debug in terminal */
    if (argc > 1) {
        def_mode = LOG_PRINTF_TERMINAL;
    }

    /* init log param */
    init_log_file(RT_MONITOR_LOG);
    log_set_param(def_mode, LOG_INFO, RT_MONITOR_LOG_MAX_SIZE);

    /* debug versions information */
    init_app_version(NULL);

    /* install ops callbacks */
    init_callback_ops();
    init_card(log_print);

    /* install system signal handle */
    init_system_signal(NULL);

    /* install ipc callbacks and start up ipc server */
    ipc_regist_callback(monitor_cmd);
    ipc_socket_server_start();

    /* start up agent */
    while (1) {
        agent_task_check_start();
        rt_os_sleep(RT_AGENT_WAIT_MONITOR_TIME);
    }

    return RT_SUCCESS;
}
