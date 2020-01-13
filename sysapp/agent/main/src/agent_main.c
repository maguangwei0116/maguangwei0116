
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

#include "agent_queue.h"
#include "card_manager.h"
#include "card_detection.h"
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
#include "agent2monitor.h"
#include "mbn.h"
#include "file.h"
#include "libcomm.h"
#include "agent_main.h"
#include "rt_timer.h"
#ifdef CFG_STANDARD_MODULE
#include "customer_at.h"
#include "at_command.h"
#endif

#define INIT_OBJ(func, arg)     {#func, func, arg}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           (sizeof((a)) / sizeof((a)[0]))
#endif

#define RT_AGENT_LOG            "rt_log"

typedef int32_t (*init_func)(void *arg);

typedef struct INIT_OBJ
{
    const char *name;
    init_func init;
    void *arg;
} init_obj_t;

static public_value_list_t g_value_list;
static char g_app_path[128];

static int32_t init_monitor(void *arg)
{
    return ipc_set_monitor_param(((public_value_list_t *)arg)->config_info);
}

static int32_t init_qmi(void *arg)
{
    return rt_qmi_init(arg);
}

#define SET_STR_PARAM(param, value) snprintf((param), sizeof(param), "%s", (value))

static int32_t get_monitor_version(char *name, int32_t n_size, char *version, \
                                        int32_t v_size, char *chip_modle, int32_t c_size)
{
#ifdef CFG_PLATFORM_ANDROID
#define RT_AGENT_NAME           "agent"
#define RT_MONITOR_NAME         "monitor"

    extern int32_t rt_qmi_get_monitor_version(uint8_t *version);
    char m_version[128] = {0};
    int32_t ret = RT_SUCCESS;
    
    ret = rt_qmi_get_monitor_version(m_version);
    if (!ret) {
        char m_target_name[128] = {0};
        const char *p = LOCAL_TARGET_NAME;
        const char *p0 = p + rt_os_strlen(p);
        const char *p1 = NULL;

        /* "android-euicc-agent-general" => "android-euicc-monitor-general" */
        p0 = rt_os_strstr(LOCAL_TARGET_NAME, RT_AGENT_NAME);
        rt_os_memcpy(m_target_name, p, p0-p);
        rt_os_memcpy(&m_target_name[p0-p], RT_MONITOR_NAME, rt_os_strlen(RT_MONITOR_NAME));
        p1 = rt_os_strrchr(LOCAL_TARGET_NAME, '-');
        rt_os_memcpy(&m_target_name[rt_os_strlen(m_target_name)], p1, rt_os_strlen(p1));
        /* copy data out */
        snprintf(name, n_size, "%s", m_target_name);
        snprintf(version, v_size, "%s", m_version);
        snprintf(chip_modle, c_size, "%s", LOCAL_TARGET_PLATFORM_TYPE);
    }

    return ret;
    
#undef RT_AGENT_NAME
#undef RT_MONITOR
#else
    return ipc_get_monitor_version(name, n_size, version, v_size, chip_modle, c_size);
#endif
}

#ifdef CFG_STANDARD_MODULE
static int32_t version_format(const char *version, int32_t ver_int[4])
{
    sscanf(version, "%d.%d.%d.%d", &ver_int[0], &ver_int[1], &ver_int[2], &ver_int[3]);

    return RT_SUCCESS;
}

/*
# Auto generate softsim-release file
#                  MFR      agent     monitor    so       ubi       share profile batch code 
# e.g. "Release: general_v4.5.6.10_v7.8.9.10_v1.2.3.10_v12.15.18.30#B191213070351591259"
*/
static int32_t get_oemapp_version(const char *a_version, const char * m_version, const char *so_version, \
                                        const char *share_profile_name, char *version, int32_t v_size)
{
    int32_t a_ver_int[4] = {0};
    int32_t m_ver_int[4] = {0};
    int32_t so_ver_int[4] = {0};
    int32_t o_ver_int[4] = {0};
    int32_t i = 0;
    
    version_format(a_version, a_ver_int);
    version_format(m_version, m_ver_int);
    version_format(so_version, so_ver_int);

    for (i = 0; i < 4; i++) {
        o_ver_int[i] = a_ver_int[i] + m_ver_int[i] + so_ver_int[i];
    }

    snprintf(version, v_size, "%d.%d.%d.%d#%s", o_ver_int[0], o_ver_int[1], o_ver_int[2], o_ver_int[3], share_profile_name);

    return RT_SUCCESS;
}
#endif

static int32_t init_versions(void *arg)
{
    char libcomm_ver[128] = {0};
    char share_profile_ver_str[128] = {0};
    static target_versions_t g_target_versions = {0};

    libcomm_get_version(libcomm_ver, sizeof(libcomm_ver));
    MSG_PRINTF(LOG_WARN, "%s\n", libcomm_ver);

    ((public_value_list_t *)arg)->version_info = &g_target_versions;

    /* add agent version */
    SET_STR_PARAM(g_target_versions.versions[TARGET_TYPE_AGENT].name, LOCAL_TARGET_NAME);
    SET_STR_PARAM(g_target_versions.versions[TARGET_TYPE_AGENT].version, LOCAL_TARGET_VERSION);
    SET_STR_PARAM(g_target_versions.versions[TARGET_TYPE_AGENT].chipModel, LOCAL_TARGET_PLATFORM_TYPE);

    /* add share profile version */
    bootstrap_get_profile_version(g_target_versions.versions[TARGET_TYPE_SHARE_PROFILE].name,
                              sizeof(g_target_versions.versions[TARGET_TYPE_SHARE_PROFILE].name),
                              g_target_versions.versions[TARGET_TYPE_SHARE_PROFILE].version,
                              sizeof(g_target_versions.versions[TARGET_TYPE_SHARE_PROFILE].version));
    SET_STR_PARAM(g_target_versions.versions[TARGET_TYPE_SHARE_PROFILE].chipModel, LOCAL_TARGET_PLATFORM_TYPE);

    /* add monitor version */
    get_monitor_version(g_target_versions.versions[TARGET_TYPE_MONITOR].name,
                            sizeof(g_target_versions.versions[TARGET_TYPE_MONITOR].name),
                            g_target_versions.versions[TARGET_TYPE_MONITOR].version,
                            sizeof(g_target_versions.versions[TARGET_TYPE_MONITOR].version),
                            g_target_versions.versions[TARGET_TYPE_MONITOR].chipModel,
                            sizeof(g_target_versions.versions[TARGET_TYPE_MONITOR].chipModel));

    /* add comm so version */
    libcomm_get_all_version(g_target_versions.versions[TARGET_TYPE_COMM_SO].name,
                            sizeof(g_target_versions.versions[TARGET_TYPE_COMM_SO].name),
                            g_target_versions.versions[TARGET_TYPE_COMM_SO].version,
                            sizeof(g_target_versions.versions[TARGET_TYPE_COMM_SO].version),
                            g_target_versions.versions[TARGET_TYPE_COMM_SO].chipModel,
                            sizeof(g_target_versions.versions[TARGET_TYPE_COMM_SO].chipModel));

#ifdef CFG_STANDARD_MODULE
    /* add oemapp version */
    SET_STR_PARAM(g_target_versions.versions[TARGET_TYPE_OEMAPP].name, LOCAL_TARGET_OEMAPP_NAME);
    SET_STR_PARAM(g_target_versions.versions[TARGET_TYPE_OEMAPP].chipModel, LOCAL_TARGET_PLATFORM_TYPE);
    get_oemapp_version(g_target_versions.versions[TARGET_TYPE_AGENT].version,
                       g_target_versions.versions[TARGET_TYPE_MONITOR].version,
                       g_target_versions.versions[TARGET_TYPE_COMM_SO].version,
                       g_target_versions.versions[TARGET_TYPE_SHARE_PROFILE].name,
                       g_target_versions.versions[TARGET_TYPE_OEMAPP].version,
                       sizeof(g_target_versions.versions[TARGET_TYPE_OEMAPP].version));
#endif

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

    return RT_SUCCESS;
}

extern int32_t init_backtrace(void *arg);
#endif

/*
List your init call here !
Adjust the order very carefully !
**/
static const init_obj_t g_init_objs[] =
{
    INIT_OBJ(init_rt_file_path,         g_app_path),
    INIT_OBJ(init_log_file,             RT_AGENT_LOG),
    INIT_OBJ(init_config,               (void *)&g_value_list),

#ifdef CFG_ENABLE_LIBUNWIND
    INIT_OBJ(init_backtrace,            agent_printf),
#endif

#ifdef CFG_STANDARD_MODULE
    INIT_OBJ(init_at_command,           (void *)&g_value_list),
#endif

    INIT_OBJ(init_bootstrap,            (void *)&g_value_list),
    INIT_OBJ(init_qmi,                  NULL),
    INIT_OBJ(init_versions,             (void *)&g_value_list),
    INIT_OBJ(init_device_info,          (void *)&g_value_list),
#ifndef CFG_PLATFORM_ANDROID
    INIT_OBJ(init_mbn,                  (void *)&g_value_list),
#endif
    INIT_OBJ(init_monitor,              (void *)&g_value_list),
    INIT_OBJ(init_lpa_channel,          (void *)&g_value_list),
    INIT_OBJ(init_timer,                NULL),    
    INIT_OBJ(init_queue,                (void *)&g_value_list),
    INIT_OBJ(init_personalise,          (void *)&g_value_list),
    INIT_OBJ(init_card_manager,         (void *)&g_value_list),
    INIT_OBJ(init_card_detection,       (void *)&g_value_list),
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

int32_t agent_main(const char *app_path, log_func logger)
{
    if (app_path) {
        snprintf(g_app_path, sizeof(g_app_path), "%s", app_path);
    }
    if (logger) {
        log_install_func(logger);
    }

    agent_init_call();

    return RT_SUCCESS;
}
