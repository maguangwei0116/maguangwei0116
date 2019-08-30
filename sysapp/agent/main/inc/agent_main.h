
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

typedef struct INFO_VUICC_DATA {
　　int16_t start; // Agent message flag, 0xFFFF
　　int8_t vuicc_switch; // lpa_channel_type_e, IPC used for vuicc
   int8_t share_profile_state; // 0:not damage， 1:damaged
} info_vuicc_data_t;

#endif // __AGENT_MAIN_H__
