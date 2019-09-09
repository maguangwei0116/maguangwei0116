
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_quectel.h
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __AGENT_MAIN_H__
#define __AGENT_MAIN_H__

#include "rt_type.h"
#include "card_manager.h"

#define ICCID_LEN                                20
#define MCC_LEN                                  3
#define EID_LENGTH                               32
#define IMEI_LENGTH                              15
#define IMSI_LENGTH                              16
#define MAX_APN_LENGTH                           100
#define MCC_MNC_LENGTH                           5

#define ICCID_LEN                                20
#define MCC_LEN                                  3
#define EID_LENGTH                               32
#define IMEI_LENGTH                              15
#define IMSI_LENGTH                              16
#define MAX_APN_LENGTH                           100
#define MCC_MNC_LENGTH                           5

typedef struct INFO_VUICC_DATA {
    uint8_t start[7]; // Agent message flag, 0xFFFF
    int8_t vuicc_switch; // lpa_channel_type_e, IPC used for vuicc
    int8_t share_profile_state; // 0:not damage， 1:damaged
} info_vuicc_data_t;

#endif // __AGENT_MAIN_H__
