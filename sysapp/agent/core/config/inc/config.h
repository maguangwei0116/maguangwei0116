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
    char *              oti_addr;
    char *              emq_addr;
    char *              proxy_addr;
    uint8_t             lpa_channel_type;   // @ref lpa_channel_type_e
    uint32_t            log_max_size;       // unit: MB
    uint8_t             monitor_log_level;  // @ref log_level_e in log.h
    uint8_t             agent_log_level;    // @ref log_level_e in log.h
    uint8_t             init_profile_type;  // @ref init_profile_type_e in card_manager.h       
} config_info_t;

int32_t init_config(void *arg);

#endif  // __RT_CONFIG_H__
