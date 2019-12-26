
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

typedef struct ATOM_DATA {
    uint8_t         start[7];               // agent message flag, the 6th and 7th must be 0xFF
    uint8_t         cmd;                    // see the docs
    uint8_t         rfu[3];                 // reserve for keep 4 bytes aligned
    uint8_t         data_len;               // data len
    uint8_t         *data;                  // data buf
} atom_data_t;

typedef struct INFO_VUICC_DATA {
    uint8_t         vuicc_switch;           // lpa_channel_type_e, IPC used for vuicc
    uint8_t         log_level;              // log_level_e
    uint8_t         reserve[2];             // reserve for keep 4 bytes aligned
    uint32_t        log_size;               // unit: bytes, little endian
} info_vuicc_data_t;

typedef struct MONITOR_VERSION {
    char            name[64];               // example: linux-euicc-monitor-general
    char            version[16];            // example: 0.0.0.1
    char            chip_model[16];         // example: 9x07
} monitor_version_t;

typedef struct SIGNATURE_DATA {    
    uint8_t         hash[64+4];             // hash, end with ¡®\0¡¯    
    uint8_t         signature[128+4];       // signature data, end with ¡®\0¡¯
} signature_data_t;

int32_t ipc_set_monitor_param(config_info_t *config_info);
int32_t ipc_get_monitor_version(char *name, int32_t n_size, char *version, \
                                    int32_t v_size, char *chip_modle, int32_t c_size);
int32_t ipc_sign_verify_by_monitor(const char *hash, const char *sign);
int32_t ipc_file_verify_by_monitor(const char *abs_file, char *real_file_name); // file: Absolute path !!!
int32_t ipc_restart_monitor(uint8_t delay);
int32_t ipc_select_profile_by_monitor(void);


#endif // __AGENT_2_MONITOR_H__


