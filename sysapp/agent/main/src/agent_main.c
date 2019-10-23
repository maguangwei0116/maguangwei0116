
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_main.c
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "agent_main.h"
#include "agent_queue.h"
#include "card_manager.h"
#include "ipc_socket_client.h"
#include "network_detection.h"
#include "rt_mqtt.h"
#include "config.h"
#include "bootstrap.h"
#include "upload.h"
#include "rt_qmi.h"
#include "lpa.h"
#include "ota.h"
#include "logm.h"
#include "personalise.h"
#include "upgrade.h"
#include "libcomm.h"

#define INIT_OBJ(func, arg)     {#func, func, arg}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           (sizeof((a)) / sizeof((a)[0]))
#endif

typedef int32_t (*init_func)(void *arg);

typedef struct INIT_OBJ {
    const char *    name;
    init_func       init;
    void *          arg;
} init_obj_t ;

static volatile int32_t toStop = 0;
static public_value_list_t g_value_list;

static void cfinish(int32_t sig)
{
    MSG_PRINTF(LOG_DBG, "recv signal %d, process exit !\r\n", sig);
    rt_os_signal(RT_SIGINT, NULL);
    toStop = 1;
}

static int32_t init_system_signal(void *arg)
{
    rt_os_signal(RT_SIGINT, cfinish);
    return RT_SUCCESS;
}

static int32_t init_monitor(void *arg)
{
    info_vuicc_data_t info = {0};
    uint16_t len = sizeof(info);

    rt_os_memset(info.start, 0xFF, sizeof(info.start));
    info.vuicc_switch = ((public_value_list_t *)arg)->config_info->lpa_channel_type;
    info.share_profile_state = 0;
    info.log_level = ((public_value_list_t *)arg)->config_info->monitor_log_level;
    info.log_size = ((public_value_list_t *)arg)->config_info->log_max_size;
    MSG_PRINTF(LOG_INFO, "len:%d, log_max_size:%d, %08x\n", len, info.log_size, info.log_size);
    return ipc_send_data((const uint8_t *)&info, len, (uint8_t *)&info, &len);
}

static int32_t init_qmi(void *arg)
{
    return rt_qmi_init(arg);
}

static int32_t init_versions(void *arg)
{
    char libcomm_ver[128] = {0};

    (void)arg;
    
    libcomm_get_version(libcomm_ver, sizeof(libcomm_ver));
    MSG_PRINTF(LOG_WARN, "App version: %s\n", LOCAL_TARGET_RELEASE_VERSION_NAME);
    MSG_PRINTF(LOG_WARN, "%s\n", libcomm_ver);

    return RT_SUCCESS;
}

static int32_t init_lpa_channel(void *arg)
{
    int8_t *lpa_channel_type = &((public_value_list_t *)arg)->config_info->lpa_channel_type;

    return init_lpa(lpa_channel_type);
}

#ifdef CFG_ENABLE_LIBUNWIND
#include <stdarg.h>
static int32_t agent_printf(const char *fmt, ...)
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

/*
List your init call here !
Adjust the order very carefully !
**/
static const init_obj_t g_init_objs[] =
{
    INIT_OBJ(init_log_file,             NULL),
    INIT_OBJ(init_config,               (void *)&g_value_list),

#ifdef CFG_ENABLE_LIBUNWIND
    INIT_OBJ(init_backtrace,            agent_printf),
#endif
    
    INIT_OBJ(init_versions,             (void *)&g_value_list),   
    INIT_OBJ(init_device_info,          (void *)&g_value_list),
    INIT_OBJ(init_monitor,              (void *)&g_value_list),
    INIT_OBJ(init_lpa_channel,          (void *)&g_value_list),
    INIT_OBJ(init_system_signal,        NULL),
    INIT_OBJ(init_timer,                NULL),
    INIT_OBJ(init_qmi,                  NULL),
    INIT_OBJ(init_queue,                (void *)&g_value_list),
    INIT_OBJ(init_bootstrap,            NULL),
    INIT_OBJ(init_personalise,          (void *)&g_value_list),
    INIT_OBJ(init_card_manager,         (void *)&g_value_list),
    INIT_OBJ(init_network_detection,    (void *)&g_value_list),
    INIT_OBJ(init_mqtt,                 (void *)&g_value_list),
    INIT_OBJ(init_upload,               (void *)&g_value_list),
    INIT_OBJ(init_upgrade,              (void *)&g_value_list),
    INIT_OBJ(init_ota,                  (void *)&g_value_list),
    INIT_OBJ(init_logm,                 (void *)&g_value_list),
};

static int32_t agent_init_call(void)
{
    int32_t i;
    int32_t ret;

    for (i = 0; i < ARRAY_SIZE(g_init_objs); i++) {
        ret = g_init_objs[i].init(g_init_objs[i].arg);
        if (rt_os_strcmp("init_log_file", g_init_objs[i].name)) {
            MSG_PRINTF(LOG_DBG, "%-30s[%s]\r\n", g_init_objs[i].name, !ret ? " OK " : "FAIL");
        }
    }

    return RT_SUCCESS;
}

int32_t main(int32_t argc, int8_t **argv)
{
    agent_init_call();
    
    while (!toStop) {
        rt_os_sleep(3);
    }

    return 0;
}

