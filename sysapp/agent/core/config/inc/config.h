/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_config.h
 * Date        : 2017.09.01
 * Note        :
 * Description : The configuration file is used to configure file function

 *******************************************************************************/
#ifndef __RT_CONFIG_H__
#define __RT_CONFIG_H__

#include "stdint.h"

typedef enum MODE_TYPE {
    MODE_TYPE_SIM_FIRST             = 0,
    MODE_TYPE_EUICC                 = 1,
    MODE_TYPE_VUICC                 = 2,
    MODE_TYPE_SIM_ONLY              = 3,
} mode_type_e;

typedef enum PROJECT_MODE {
    PROJECT_REDTEAREADY             = 0,
    PROJECT_SC                      = 1,
} project_mode_e;

typedef struct CONFIG_INFO {
    char *              oti_addr;           // OTI server addr
    char *              emq_addr;           // EMQ server addr
    char *              proxy_addr;         // proxy server addr
    project_mode_e      project_mode;       // project mode
    mode_type_e         sim_mode;           // sim mode
    uint8_t             lpa_channel_type;   // @ref lpa_channel_type_e
    uint32_t            log_max_size;       // unit: MB
    uint8_t             monitor_log_level;  // @ref log_level_e in log.h
    uint8_t             agent_log_level;    // @ref log_level_e in log.h
    uint8_t             init_profile_type;  // @ref init_profile_type_e in card_manager.h
    uint8_t             mbn_enable;         // 0: disable   1: enable
    uint8_t             flow_control_switch;// 0: close     1: open

    /* some config item which never changed dynamically */
    uint32_t            oti_port;           // port for OTI server
} config_info_t;

int32_t init_config(void *arg);

int32_t config_update_uicc_mode(int32_t mode);  // mode: @ref lpa_channel_type_e

int32_t config_get_uicc_mode(const char *app_path, int32_t *mode); // mode: @ref lpa_channel_type_e

int32_t config_set_restart_reason(const char *reason);

#endif  // __RT_CONFIG_H__

