
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ipc_agent_command.h
 * Date        : 2021.03.13
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __IPC_AGENT_COMMAND_H__
#define __IPC_AGENT_COMMAND_H__

#include "rt_type.h"

#ifdef CFG_OPEN_MODULE

typedef enum AGENT_CMD {
    AGENT_CMD_SET_PARAM       		= 0x00,
    AGENT_CMD_GET_PARAM       		= 0x01,
} agent_cmd_e;

typedef enum AGENT_CMD_PARAM_TYPE {
    AGENT_CMD_PARAM_CARD_TYPE       = 0x00,
    AGENT_CMD_PARAM_SIM_MONITOR    	= 0x01,
    AGENT_CMD_PARAM_ICCID           = 0x02,
} agent_cmd_param_type_e;

typedef struct AGENT_CMD_SET_PARAM_DATA {
    uint8_t         type;             	// type of parameter
    uint8_t         length[2];	    	// length of parameter
    uint8_t         value[1];			// value of parameter
} agent_cmd_set_param_data_t;

typedef enum AGENT_CMD_SET_CARD_TYPE_PARAM {
    AGENT_CMD_SET_CARD_TYPE_VSIM	= 0x00,
    AGENT_CMD_SET_CARD_TYPE_SIM		= 0x01,
} agent_cmd_set_card_type_param_e;

typedef struct AGENT_CMD_GET_PARAM_DATA {
    uint8_t         type;               // type of parameter
    uint8_t         length[2];          // length of parameter
    uint8_t         value[1];           // value of parameter
} agent_cmd_get_param_data_t;

typedef enum AGENT_RESULT {
    AGENT_RESULT_OK       						= 0x00,
    AGENT_RESULT_ERR_PARAM_TYPE_UNKNOWN 		= 0x01,
    AGENT_RESULT_ERR_PARAM_LENGTH_INVALID 		= 0x02,
    AGENT_RESULT_ERR_SWITCH_CARD_WHEN_SIM_ONLY 	= 0x03,
    AGENT_RESULT_ERR_SWITCH_CARD_PARAM_UNKNOWN 	= 0x04,
    AGENT_RESULT_ERR_SWITCH_CARD_SIM_NOT_EXIST 	= 0x05,
    AGENT_RESULT_ERR_SWITCH_CARD_SIM_IS_USING 	= 0x06,
    AGENT_RESULT_ERR_SWITCH_CARD_NOTHING_DONE 	= 0x07,
    AGENT_RESULT_ERR_SWITCH_CARD_VUICC_IS_USING = 0x08,
    AGENT_RESULT_ERR_GET_SET_SIM_MONITOR        = 0x09,
} agent_result_e;

typedef struct AGENT_RSP {
    uint8_t         result;             // result
    uint8_t         length[2];	    	// length of resp
    uint8_t         value[1];			// data of resp
} agent_rsp_t;

#endif // CFG_OPEN_MODULE

#endif // __AGENT_COMMAND_H__
