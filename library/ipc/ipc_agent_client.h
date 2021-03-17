
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ipc_agent_client.h
 * Date        : 2021.03.13
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __IPC_AGENT_CLIENT__
#define __IPC_AGENT_CLIENT__

#include "rt_type.h"

typedef enum AGENT_SWITCH_CARD_PARAM {
    AGENT_SWITCH_TO_VSIM		= 0x00,
    AGENT_SWITCH_TO_SIM			= 0x01,
} agent_switch_card_param_e;

typedef enum AGENT_SET_SIM_MONITOR_PARAM {
    AGENT_SIM_MONITOR_DISABLE	= 0x00,
    AGENT_SIM_MONITOR_ENABLE	= 0x01,
} agent_set_sim_monitor_param_e;

int32_t ipc_agent_switch_card(agent_switch_card_param_e param);
int32_t ipc_agent_set_sim_monitor(agent_set_sim_monitor_param_e param);
int32_t ipc_agent_get_card_type(agent_switch_card_param_e *type);
int32_t ipc_agent_get_sim_monitor(agent_set_sim_monitor_param_e *state);
int32_t ipc_agent_get_iccid_list(char *iccid_list, int32_t *size); // max size 512

#endif // __IPC_AGENT_CLIENT__
