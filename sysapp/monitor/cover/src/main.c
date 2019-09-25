
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
#include "ipc_socket_server.h"
#include "trigger.h"
#include "card.h"
#include "esim_api.h"

#define RT_AGENT_WAIT_MONITOR_TIME  3
#define RT_AGENT_PTROCESS           "agent"
#define RT_AGENT_FILE               "/usr/bin/agent"
#define RT_MONITOR_LOG              "/data/redtea/rt_monitor_log"
#define RT_MONITOR_LOG_MAX_SIZE     (1 * 1024 * 1024)
#define RT_MONITOR_RSP_LEN          2048

static uint8_t g_rsp[RT_MONITOR_RSP_LEN];
static uint16_t g_rsp_len;
extern int init_file_ops(void);

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
    static uint8_t cnt = 0, left = 0;
    static uint8_t *rsp_buf = g_rsp;

    cmd = (data[5] << 8) + data[6];
    if (cmd == 0xFFFF) { // msg from agent
        MSG_PRINTF(LOG_INFO, "Receive msg from agent,uicc type:%s\r\n", (data[7] == 0x00) ? "vUICC":"eUICC");
        if (data[7] == 0x00) {  // used vuicc
            trigegr_regist_reset(card_reset);
            trigegr_regist_cmd(card_cmd);
            trigger_swap_card(1);
        }
    } else {
        MSG_PRINTF(LOG_INFO, "Receive msg from lpa\r\n");
        // for get response
        if (data[1] == 0xC0) {
            if (cnt != 0) {
                rt_os_memcpy(rsp, rsp_buf, 256);
                rsp[256] = 0x61;
                rsp[257] = 0x00;
                *rsp_len = 258;
                cnt --;
                rsp_buf += 256;
            } else if (left != 0) {
                rt_os_memcpy(rsp, rsp_buf, left);
                rsp[left] = 0x90;
                rsp[left + 1] = 0x00;
                *rsp_len = left + 2;
                left = 0;
                rsp_buf = g_rsp;
            }
            return RT_SUCCESS;
        } else {
            cnt = 0;
            left = 0;
            rsp_buf = g_rsp;
        }
    
        // send apdu
        softsim_logic_command(1, (uint8_t *)data, len, g_rsp, &g_rsp_len);  // msg from lpa
        if ((cmd == 0xBF31) || (cmd == 0xFF7F)) {
            trigger_swap_card(1);
        }
        
        // for 61xx
        if (g_rsp_len == 2) {
            rt_os_memcpy(rsp, g_rsp, g_rsp_len);
            *rsp_len = g_rsp_len;
            return RT_SUCCESS;
        }
        if (g_rsp_len > 256) {
            rsp[0] = 0x61;
            rsp[1] = 0x00;
            *rsp_len = 2;
        } else {
            rsp[0] = 0x61;
            rsp[1] = g_rsp_len - 2;
            *rsp_len = 2;
        }
        cnt = (g_rsp_len - 2) / 256;
        left = (g_rsp_len - 2) % 256;
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
    snprintf(cmd, sizeof(cmd), "killall -9 %s", RT_AGENT_PTROCESS);
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

int32_t main(int32_t argc, const char *argv[])
{
    pid_t child_pid;

    init_log_file(RT_MONITOR_LOG);
    log_set_param(LOG_PRINTF_TERMINAL, LOG_INFO, RT_MONITOR_LOG_MAX_SIZE);
    MSG_PRINTF(LOG_WARN, "App version: %s\n", LOCAL_TARGET_RELEASE_VERSION_NAME);

    init_file_ops();
    softsim_logic_start(log_print);

    init_system_signal(NULL);

    ipc_regist_callback(monitor_cmd);
    ipc_socket_server_start();

    while (1) {
        agent_task_check_start();
        rt_os_sleep(RT_AGENT_WAIT_MONITOR_TIME);
    }

    return RT_SUCCESS;
}
