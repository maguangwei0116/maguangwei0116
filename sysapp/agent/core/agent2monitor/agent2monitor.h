
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent2monitor.h
 * Date        : 2019.10.29
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __AGENT_2_MONITOR_H__
#define __AGENT_2_MONITOR_H__

#include "config.h"

typedef enum AGENT_MONITOR_CMD {
    CMD_SET_PARAM       = 0x00,
    CMD_SIGN_CHK        = 0x01,
    CMD_SELECT_PROFILE  = 0x02,
    CMD_GET_MONITOR_VER = 0x03,
    CMD_RFU             = 0x04,
} agent_monitor_cmd_e;

typedef struct MONITOR_VERSION {
    char     name[64];           // example: linux-euicc-monitor-general
    char     version[16];        // example: 0.0.0.1
    char     chip_model[16];     // example: 9x07
} monitor_version_t;

int32_t agent_set_monitor_param(config_info_t *config_info);
int32_t agent_get_monitor_version(char *name, int n_size, char *version, int v_size, char *chip_modle, int c_size);


#endif // __AGENT_2_MONITOR_H__


