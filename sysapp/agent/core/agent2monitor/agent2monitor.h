
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
    CMD_RESTART_MONITOR = 0x04,
    CMD_RFU             = 0x05,
} agent_monitor_cmd_e;

typedef struct MONITOR_VERSION {
    char        name[64];           // example: linux-euicc-monitor-general
    char        version[16];        // example: 0.0.0.1
    char        chip_model[16];     // example: 9x07
} monitor_version_t;

typedef struct SIGNATURE_DATA {    
    uint8_t     hash[64+4];         // hash, end with ��\0��    
    uint8_t     signature[128+4];   // signature data, end with ��\0��
} signature_data_t;

int32_t ipc_set_monitor_param(config_info_t *config_info);
int32_t ipc_get_monitor_version(char *name, int n_size, char *version, int v_size, char *chip_modle, int c_size);
int32_t ipc_sign_verify_by_monitor(const char *hash, const char *sign);
int32_t ipc_file_verify_by_monitor(const char *file);
int32_t ipc_restart_monitor(uint8_t delay);
int32_t ipc_select_profile_by_monitor(void);


#endif // __AGENT_2_MONITOR_H__


