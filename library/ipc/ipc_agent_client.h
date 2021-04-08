
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

#ifdef CFG_OPEN_MODULE

typedef enum AGENT_SWITCH_CARD_PARAM {
    AGENT_SWITCH_TO_VSIM		= 0x00,
    AGENT_SWITCH_TO_SIM			= 0x01,
} agent_switch_card_param_e;

typedef enum AGENT_SET_SIM_MONITOR_PARAM {
    AGENT_SIM_MONITOR_DISABLE	= 0x00,
    AGENT_SIM_MONITOR_ENABLE	= 0x01,
} agent_set_sim_monitor_param_e;

typedef enum AGENT_REGISTRATION_STATE {
    AGENT_NOT_REGISTERED            = 0,
    AGENT_REGISTERED                = 1,
    AGENT_NOT_REGISTERED_SEARCHING  = 2,
    AGENT_REGISTRATION_DENIED       = 3,
    AGENT_REGISTRATION_UNKNOWN      = 4,
    AGENT_ROAM_NETWORK              = 5,
} agent_registration_state_e;

typedef enum AGENT_DSI_STATE_CALL_STATE {
    AGENT_DSI_STATE_NOT_READY           = -1, // not ready
    AGENT_DSI_STATE_CALL_IDLE           = 0,  // network idle (disconnected)
    AGENT_DSI_STATE_CALL_CONNECTING     = 1,  // network connecting 
    AGENT_DSI_STATE_CALL_CONNECTED      = 2,  // network connected 
    AGENT_DSI_STATE_CALL_DISCONNECTING  = 3,  // network disconnecting
    AGENT_DSI_STATE_CALL_WAIT_NO_NET    = 4,  // network wait to net
    AGENT_DSI_STATE_CALL_MAX            = 5,  // network unknow state 
} agent_dsi_state_call_state_e;

int32_t ipc_agent_switch_card(agent_switch_card_param_e param);
int32_t ipc_agent_set_sim_monitor(agent_set_sim_monitor_param_e param);
int32_t ipc_agent_get_card_type(agent_switch_card_param_e *type);
int32_t ipc_agent_get_sim_monitor(agent_set_sim_monitor_param_e *state);
int32_t ipc_agent_get_iccid_list(char *iccid_list, int32_t *size); // max size 512
int32_t ipc_agent_get_network_state(agent_registration_state_e *regist_state, agent_dsi_state_call_state_e *dial_up_state);

#endif // CFG_OPEN_MODULE
#endif // __IPC_AGENT_CLIENT__
