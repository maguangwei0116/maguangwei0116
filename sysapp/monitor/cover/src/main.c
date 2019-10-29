
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
#include "parse_backup.h"
#include "inspect_file.h"
#include "libcomm.h"

#define RT_AGENT_WAIT_MONITOR_TIME  3
#define RT_AGENT_PTROCESS           "rt_agent"
#define RT_AGENT_FILE               "/data/agent1"
#define RT_MONITOR_FILE             "/data/monitor"
#define RT_MONITOR_LOG              "/data/redtea/rt_monitor_log"
#define RT_MONITOR_LOG_MAX_SIZE     (1 * 1024 * 1024)

extern int init_file_ops(void);
extern int vsim_get_ver(char *version);

static log_mode_e g_def_mode = LOG_PRINTF_TERMINAL;

typedef struct {
    uint8_t             hash[64];                  // hash
    uint8_t             signature[128];            // signature data
} signature_data_t;

/* All data should be a string which end with ‘\0’ */
typedef struct {
    uint8_t             name[64];                  // example: linux-euicc-monitor-general
    uint8_t             version[8];                // example: 0.0.0.1
    uint8_t             chip_model[16];            // example: 9x07
} monitor_version_t;

typedef struct {
    uint8_t             vuicc_switch;              // lpa_channel_type_e, IPC used for vuicc
    uint8_t             log_level;                 // log_level_e
    uint8_t             reserve[2];                // reserve for keep 4 bytes aligned
    uint32_t            log_size;                  // unit: bytes, little endian
} info_vuicc_data_t;


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

static uint16_t monitor_deal_agent_msg(uint8_t cmd, const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    if (cmd == 0x00) {  // config monitor
        if (len < sizeof(info_vuicc_data_t)) {
            goto end;
        }
        info_vuicc_data_t *info = (info_vuicc_data_t *)data;
        MSG_PRINTF(LOG_INFO, "Receive msg from agent,uicc type:%s\r\n", (data[7] == 0x00) ? "vUICC" : "eUICC");
        if (info->vuicc_switch == 0x00) {
            trigegr_regist_reset(card_reset);
            trigegr_regist_cmd(card_cmd);
            trigger_swap_card(1);
            *rsp_len = 0;
        }
        MSG_PRINTF(LOG_INFO, "set log_level=%d, log_max_size=%d\n", info->log_level, info->log_size);
        log_set_param(g_def_mode, info->log_level, info->log_size);
        *rsp = 0x01;
        *rsp_len = 1;
    } else if (cmd == 0x01) { // check signature
        if (len < sizeof(signature_data_t)) {
            goto end;
        }
        signature_data_t *info = (signature_data_t *)data;
        *rsp = (uint8_t)inspect_abstract_content(info->hash, info->signature);
        *rsp_len = 1;
    } else if (cmd == 0x02) { // choose one profile from backup profile
        backup_process();
    } else if (cmd == 0x03) { // monitor version info
        monitor_version_t info;
        *rsp_len = sizeof(monitor_version_t);
        rt_os_memset(&info, 0x00, *rsp_len);
        rt_os_memcpy(info.name, LOCAL_TARGET_NAME, rt_os_strlen(LOCAL_TARGET_NAME));
        rt_os_memcpy(info.version, LOCAL_TARGET_VERSION, rt_os_strlen(LOCAL_TARGET_VERSION));
        rt_os_memcpy(info.chip_model, LOCAL_TARGET_PLATFORM_TYPE, rt_os_strlen(LOCAL_TARGET_PLATFORM_TYPE));
        rt_os_memcpy(rsp, &info, *rsp_len);
    }

    return RT_SUCCESS;

end:
    return RT_ERROR;
}

uint16_t monitor_cmd(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint16_t cmd = 0;
    uint16_t sw = 0;
    int8_t log_level;
    uint32_t log_max_size;
    static rt_bool reset_flag = RT_FALSE;

    cmd = (data[5] << 8) + data[6];
    if (cmd == 0xFFFF) { // msg from agent
        return monitor_deal_agent_msg(data[7], &data[12], data[11], rsp, rsp_len);
    } else { // msg for vuicc
        MSG_INFO_ARRAY("E-APDU REQ:", data, len);
        sw = card_cmd((uint8_t *)data, len, rsp, rsp_len);
        rsp[(*rsp_len)++] = (sw >> 8) & 0xFF;
        rsp[(*rsp_len)++] = sw & 0xFF;
        MSG_INFO_ARRAY("E-APDU RSP:", rsp, *rsp_len);

        /* enable profile and load bootstrap profile, need to reset */
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

// choose euicc or vuicc
static int32_t choose_uicc_type(void)
{
    lpa_channel_type_e type = LPA_CHANNEL_BY_IPC;

    if (type == LPA_CHANNEL_BY_IPC) {
        trigegr_regist_reset(card_reset);
        trigegr_regist_cmd(card_cmd);
        trigger_swap_card(1);
    }
    init_apdu_channel(type);

    return RT_SUCCESS;
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

    /* inspect agent, if inspect failed, go to backup process */
    if (monitor_inspect_file(RT_AGENT_FILE) != RT_TRUE) {
        choose_uicc_type();
        network_detection_task();
    }
    /* start up agent process by fork function */
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
        MSG_PRINTF(LOG_INFO, "I am the parent process, my process id is %d, child_pid is %d\r\n", getpid(), child_pid);

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
    /* check input param to debug in terminal */
    if (argc > 1) {
        g_def_mode = LOG_PRINTF_TERMINAL;
    }

    /* init log param */
    init_log_file(RT_MONITOR_LOG);
    log_set_param(g_def_mode, LOG_INFO, RT_MONITOR_LOG_MAX_SIZE);
    /* debug versions information */

    /* install ops callbacks */
    init_callback_ops();
    init_card(log_print);
    init_app_version(NULL);
    /* install system signal handle */
    init_system_signal(NULL);

    /*init lib interface*/
    init_timer(NULL);
    rt_qmi_init(NULL);

    /* inspect monitor */
    while (monitor_inspect_file(RT_MONITOR_FILE) != RT_TRUE) {
        rt_os_sleep(RT_AGENT_WAIT_MONITOR_TIME);
    }

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
