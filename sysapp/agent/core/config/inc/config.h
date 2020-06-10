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

typedef struct CONFIG_INFO {
    char *              oti_addr;           // OTI server addr
    char *              emq_addr;           // EMQ server addr
    char *              proxy_addr;         // proxy server addr
    uint8_t             lpa_channel_type;   // @ref lpa_channel_type_e
#ifdef CFG_REDTEA_READY_ON
    uint8_t             sim_mode;           // sim mode
#endif
    uint32_t            log_max_size;       // unit: MB
    uint8_t             monitor_log_level;  // @ref log_level_e in log.h
    uint8_t             agent_log_level;    // @ref log_level_e in log.h
    uint8_t             init_profile_type;  // @ref init_profile_type_e in card_manager.h
    uint8_t             mbn_enable;         // 0: disable   1: enable
    uint8_t             flow_control_switch;// 0: close     1: open

    /* some config item which never changed dynamically */
    uint32_t            oti_port;           // port for OTI server
} config_info_t;

typedef enum SIM_MODE_TYPE {
    SIM_MODE_TYPE_VUICC_ONLY = 0,     // vUICC mode
    SIM_MODE_TYPE_SIM_FIRST,
    SIM_MODE_TYPE_SIM_ONLY
} sim_mode_type_e;

int32_t init_config(void *arg);

int32_t config_update_uicc_mode(int32_t mode);  // mode: @ref lpa_channel_type_e

int32_t config_get_uicc_mode(const char *app_path, int32_t *mode); // mode: @ref lpa_channel_type_e

int32_t config_set_restart_reason(const char *reason);

#endif  // __RT_CONFIG_H__

