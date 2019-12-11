
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
#include "download_file.h"
#include "file.h"

#define RT_AGENT_WAIT_MONITOR_TIME  3
#define RT_DEBUG_IN_TERMINAL        "terminal"
#define RT_AGENT_PTROCESS           "rt_agent"
#define RT_AGENT_NAME               "agent"
#define RT_MONITOR_NAME             "monitor"

#if (CFG_OPEN_MODULE)
#define RT_AGENT_FILE               "/usr/bin/rt_agent"
#define RT_MONITOR_FILE             "/usr/bin/rt_monitor"
#define RT_DATA_PATH                "/data/redtea/"
#elif (CFG_STANDARD_MODULE)  // standard
#define RT_OEMAPP_AGENT_FILE        "/oemapp/rt_agent"
#define RT_MONITOR_FILE             "/oemapp/rt_monitor"
#define RT_DATA_PATH                "/usrdata/redtea/"
#define RT_AGENT_FILE               "/usrdata/redtea/rt_agent"
#endif

#define RT_CARD_PATH                RT_DATA_PATH".vcos/"
#define RT_MONITOR_LOG              "rt_monitor_log"

#ifdef CFG_SOFTWARE_TYPE_DEBUG
#define RT_MONITOR_LOG_MAX_SIZE     (30 * 1024 * 1024)
#endif

#ifdef CFG_SOFTWARE_TYPE_RELEASE
#define RT_MONITOR_LOG_MAX_SIZE     (1 * 1024 * 1024)
#endif

extern int init_file_ops(void);
extern int vsim_get_ver(char *version);

static log_mode_e g_def_mode = LOG_PRINTF_FILE;
static rt_bool g_agent_debug_terminal = RT_FALSE;

typedef struct SIGNATURE_DATA {
    uint8_t             hash[64+4];                 // hash, end with "\0"
    uint8_t             signature[128+4];           // signature data, end with "\0"
} signature_data_t;

/* All data should be a string which end with "\0" */
typedef struct MONITOR_VERSION {
    uint8_t             name[64];                  // example: linux-euicc-monitor-general
    uint8_t             version[16];               // example: 0.0.0.1
    uint8_t             chip_model[16];            // example: 9x07
} monitor_version_t;

typedef struct INFO_VUICC_DATA {
    uint8_t             vuicc_switch;              // lpa_channel_type_e, IPC used for vuicc
    uint8_t             log_level;                 // log_level_e
    uint8_t             reserve[2];                // reserve for keep 4 bytes aligned
    uint32_t            log_size;                  // unit: bytes, little endian
} info_vuicc_data_t;

typedef enum AGENT_MONITOR_CMD {
    CMD_SET_PARAM       = 0x00,
    CMD_SIGN_CHK        = 0x01,
    CMD_SELECT_PROFILE  = 0x02,
    CMD_GET_MONITOR_VER = 0x03,
    CMD_RESTART_MONITOR = 0x04,
    CMD_RFU             = 0x05,
} agent_monitor_cmd_e;

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

static void monitor_exit(void)
{
    MSG_PRINTF(LOG_ERR, "restart monitor now ...\r\n");
    rt_os_exit(-1);
}

// choose euicc or vuicc
static int32_t choose_uicc_type(lpa_channel_type_e type)
{
    if (type == LPA_CHANNEL_BY_IPC) {
        trigegr_regist_reset(card_reset);
        trigegr_regist_cmd(card_cmd);
        trigger_swap_card(1);
    }
    init_apdu_channel(type);

    return RT_SUCCESS;
}

static uint16_t monitor_deal_agent_msg(uint8_t cmd, const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    static lpa_channel_type_e type;

    if (cmd == CMD_SET_PARAM) {  // config monitor
        if (len < sizeof(info_vuicc_data_t)) {
            goto end;
        }
        info_vuicc_data_t *info = (info_vuicc_data_t *)data;
        MSG_PRINTF(LOG_INFO, "Receive msg from agent,uicc type:%s\r\n", (info->vuicc_switch == LPA_CHANNEL_BY_IPC) ? "vUICC" : "eUICC");
        type = info->vuicc_switch;
        if (info->vuicc_switch == LPA_CHANNEL_BY_IPC) {
            trigegr_regist_reset(card_reset);
            trigegr_regist_cmd(card_cmd);
            trigger_swap_card(1);
            *rsp_len = 0;
        }
        MSG_PRINTF(LOG_INFO, "set log_level=%d, log_max_size=%d\n", info->log_level, info->log_size);
        log_set_param(g_def_mode, info->log_level, info->log_size);
        *rsp = 0x01;
        *rsp_len = 1;
    } else if (cmd == CMD_SIGN_CHK) { // check signature
        if (len < sizeof(signature_data_t)) {
            goto end;
        }
        signature_data_t *info = (signature_data_t *)data;
        *rsp = (uint8_t)inspect_abstract_content(info->hash, info->signature);
        *rsp_len = 1;
    } else if (cmd == CMD_SELECT_PROFILE) { // choose one profile from backup profile
        int32_t ret;
        choose_uicc_type(type);
        rt_os_sleep(3);  // must have, delay some for card reset !!!
        ret = backup_process(type);
        *rsp = (ret == RT_SUCCESS) ? 0x01 : 0x00;
        *rsp_len = 1;
    } else if (cmd == CMD_GET_MONITOR_VER) { // monitor version info
        monitor_version_t info;
        *rsp_len = sizeof(monitor_version_t);
        rt_os_memset(&info, 0x00, *rsp_len);
        rt_os_memcpy(info.name, LOCAL_TARGET_NAME, rt_os_strlen(LOCAL_TARGET_NAME));
        rt_os_memcpy(info.version, LOCAL_TARGET_VERSION, rt_os_strlen(LOCAL_TARGET_VERSION));
        rt_os_memcpy(info.chip_model, LOCAL_TARGET_PLATFORM_TYPE, rt_os_strlen(LOCAL_TARGET_PLATFORM_TYPE));
        rt_os_memcpy(rsp, &info, *rsp_len);
    } else if (cmd == CMD_RESTART_MONITOR) { // reset monitor after some time
        uint8_t delay = data[0];
        MSG_PRINTF(LOG_ERR, "restart monitor in %d seconds ...\r\n", delay);
        register_timer(delay, 0, &monitor_exit);
        rsp[0] = RT_TRUE;
        *rsp_len = 1;
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
        MSG_INFO_ARRAY("A-APDU REQ:", data, len);
        sw = monitor_deal_agent_msg(data[7], &data[12], data[11], rsp, rsp_len);
        MSG_INFO_ARRAY("A-APDU RSP:", rsp, *rsp_len);
        return sw;
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

static int32_t agent_process_kill(void)
{
    char cmd[128];
    
    /* kill all agent processes */
    snprintf(cmd, sizeof(cmd), "killall -9 %s > /dev/null 2>&1", RT_AGENT_PTROCESS);
    system(cmd); 

    return RT_SUCCESS;
}

#ifdef CFG_STANDARD_MODULE
static int32_t agent_file_copy_check(void)
{
    uint8_t oem_agent_sign[256];
    int32_t oem_agent_sign_len = sizeof(oem_agent_sign);
    uint8_t usr_agent_sign[256];
    int32_t usr_agent_sign_len = sizeof(usr_agent_sign);
    int32_t ret = RT_ERROR;

    if (!linux_file_exist(RT_AGENT_FILE)) {
        goto copy_usrapp_agent;  
    }

    monitor_get_file_sign_data(RT_OEMAPP_AGENT_FILE, oem_agent_sign, &oem_agent_sign_len);
    monitor_get_file_sign_data(RT_AGENT_FILE, usr_agent_sign, &usr_agent_sign_len);

    if (oem_agent_sign_len == usr_agent_sign_len && !rt_os_memcmp(oem_agent_sign, usr_agent_sign, oem_agent_sign_len)) {
        MSG_PRINTF(LOG_WARN, "agent file is the same !!!\r\n");
        ret = RT_SUCCESS;
        goto exit_entry;
    }

copy_usrapp_agent:

    linux_delete_file(RT_AGENT_FILE);
    ret = linux_file_copy(RT_OEMAPP_AGENT_FILE, RT_AGENT_FILE);
    if (!ret) {
        /* add app executable mode */
        rt_os_chmod(RT_AGENT_FILE, RT_S_IRWXU | RT_S_IRWXG | RT_S_IRWXO);
        /* sync data to flash */
        rt_os_sync();

        MSG_PRINTF(LOG_WARN, "copy agent file done !!!\r\n");
    }

exit_entry:

    return ret;
}
#endif

static int32_t agent_task_check_start(void)
{
    int32_t ret;
    int32_t status;    
    pid_t child_pid;
    pid_t ret_pid;

    /* wait ipc server start ok */
    do {
        if (ipc_server_check()) {
            break;
        }
        rt_os_sleep(1);
    } while(1);

    /* inspect agent, if inspect failed, go to backup process */
    if (monitor_inspect_file(RT_AGENT_FILE, RT_AGENT_NAME) != RT_TRUE) {
        upgrade_struct_t upgrade = {0};

        MSG_PRINTF(LOG_WARN, "agent verify error\r\n");
        linux_delete_file(RT_AGENT_FILE);
        snprintf(upgrade.file_name, sizeof(upgrade.file_name), "%s", RT_AGENT_FILE);
        snprintf(upgrade.real_file_name, sizeof(upgrade.real_file_name), "%s", RT_AGENT_NAME);
        init_download(&upgrade);
        choose_uicc_type(LPA_CHANNEL_BY_IPC);
        network_detection_task();
    }
    /* start up agent process by fork function */
    child_pid = fork();
    if (child_pid < 0) {
        MSG_PRINTF(LOG_WARN, "error in fork, err(%d)=%s\r\n", errno, strerror(errno));
    } else if (child_pid == 0) {
        MSG_PRINTF(LOG_INFO, "child process, pid %d\r\n", getpid());
        if (g_agent_debug_terminal) {
            ret = execl(RT_AGENT_FILE, RT_AGENT_PTROCESS, RT_DEBUG_IN_TERMINAL, NULL);
        } else {
            ret = execl(RT_AGENT_FILE, RT_AGENT_PTROCESS, NULL);
        }
        if (ret < 0) {
            MSG_PRINTF(LOG_ERR, "Excute %s fail, ret=%d, err(%d)=%s\n", RT_AGENT_PTROCESS, ret, errno, strerror(errno));
        }
        exit(0);
    } else {
        MSG_PRINTF(LOG_INFO, "parent process, pid %d, child_pid %d\r\n", getpid(), child_pid);

        /* block to wait designative child process's death */
        while (1) {
            ret_pid = waitpid(child_pid, &status, 0);
            if (ret_pid == child_pid) {
                MSG_PRINTF(LOG_WARN, "wait designative pid (%d) died, child process died !\r\n", child_pid);
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

#ifdef CFG_ENABLE_LIBUNWIND
#include <stdarg.h>
static int32_t monitor_printf(const char *fmt, ...)
{
    char msg[1024] = {0};
    va_list vl_list;

    va_start(vl_list, fmt);
    vsnprintf((char *)&msg[0], sizeof(msg), (const char *)fmt, vl_list);
    va_end(vl_list);

    MSG_PRINTF(LOG_ERR, "%s", msg);

    return 0;
}

extern int32_t init_backtrace(void *arg);
#endif

static void debug_with_printf(const char *msg)
{
    printf("%s", msg);
}

/*
Help for debug application in terminal:
Run: 
    ./rt_monitor terminal           -> only monitor debug in terminal
    ./rt_monitor terminal terminal  -> monitor and agent all debug in terminal
*/
int32_t main(int32_t argc, const char *argv[])
{
    rt_bool keep_agent_alive = RT_TRUE;

    /* check input param to debug in terminal */
    if (argc > 1 && !rt_os_strcmp(argv[1], RT_DEBUG_IN_TERMINAL)) {
        /* install external debug function for monitor */
        log_install_func(debug_with_printf);

        /* install external debug function for agent */
        if (argc > 2 && !rt_os_strcmp(argv[2], RT_DEBUG_IN_TERMINAL)) {
            g_agent_debug_terminal = RT_TRUE;
        }
    }

    /* init redtea path */
    init_rt_file_path(RT_DATA_PATH);

    /* init log param */
    init_log_file(RT_MONITOR_LOG);
    log_set_param(g_def_mode, LOG_INFO, RT_MONITOR_LOG_MAX_SIZE);

#ifdef CFG_ENABLE_LIBUNWIND
    init_backtrace(monitor_printf);
#endif

    /* debug versions information */
    init_app_version(NULL);

    /* install ops callbacks */
    init_callback_ops();

    /* init card path before init card */
    init_card_path(RT_CARD_PATH);
    init_card(log_print);
    
    /* install system signal handle */
    init_system_signal(NULL);

    /*init lib interface*/
    init_timer(NULL);
    rt_qmi_init(NULL);

    /* inspect monitor */
    while (monitor_inspect_file(RT_MONITOR_FILE, RT_MONITOR_NAME) != RT_TRUE) {
        rt_os_sleep(RT_AGENT_WAIT_MONITOR_TIME);
    }

    /* kill agent process before ipc server start-up */
    agent_process_kill();

    /* install ipc callbacks and start up ipc server */
    ipc_regist_callback(monitor_cmd);
    ipc_socket_server_start();

    #if 0 /* only for test */
    /* force to change to vUICC mode */
    trigegr_regist_reset(card_reset);
    trigegr_regist_cmd(card_cmd);
    trigger_swap_card(1);

    while (1) {
        rt_os_sleep(1);
    }
    #endif

#ifdef CFG_STANDARD_MODULE
    /* check and copy agent process */
    agent_file_copy_check();
#endif

    /* start up agent */
    do {
        agent_task_check_start();
        rt_os_sleep(RT_AGENT_WAIT_MONITOR_TIME);
    } while(keep_agent_alive);

    /* stop here */
    while (1) {
        rt_os_sleep(1);
    }

    return RT_SUCCESS;
}
