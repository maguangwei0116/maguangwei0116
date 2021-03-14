
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ipc_agent_client.c
 * Date        : 2021.03.13
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "ipc_agent_client.h"
#include "ipc_socket_client.h"
#include "ipc_agent_command.h"

#define AGENT_SERVER_PATH                             "./data/redtea/agent_server"

int32_t ipc_agent_switch_card(agent_switch_card_param_e param)
{
    uint8_t  buf[256] = {0};
    uint8_t  rsp[256] = {0};
    uint16_t ret_len = 0;
    int32_t  ret = RT_ERROR;

    buf[0] = AGENT_CMD_SET_PARAM;
    buf[1] = 0x00;
    buf[2] = 0x04;
    buf[3] = AGENT_CMD_PARAM_CARD_TYPE;
    buf[4] = 0x00;
    buf[5] = 0x01;
    buf[6] = param;
    if (lib_ipc_send_data(AGENT_SERVER_PATH, buf, 7, rsp, &ret_len) != RT_SUCCESS) {
        goto exit;
    }
    if (ret_len != 3 && rsp[0] != AGENT_RESULT_OK) {
        goto exit;
    }
    ret = RT_SUCCESS;

exit:
    return ret;    
}

int32_t ipc_agent_set_sim_monitor(agent_set_sim_monitor_param_e param)
{
    uint8_t  buf[256] = {0};
    uint8_t  rsp[256] = {0};
    uint16_t ret_len = 0;
    int32_t  ret = RT_ERROR;

    buf[0] = AGENT_CMD_SET_PARAM;
    buf[1] = 0x00;
    buf[2] = 0x04;
    buf[3] = AGENT_CMD_PARAM_SIM_MONITOR;
    buf[4] = 0x00;
    buf[5] = 0x01;
    buf[6] = param;
    if (lib_ipc_send_data(AGENT_SERVER_PATH, buf, 7, rsp, &ret_len) != RT_SUCCESS) {
        goto exit;
    }
    if (ret_len != 3 && rsp[0] != AGENT_RESULT_OK) {
        goto exit;
    }
    ret = RT_SUCCESS;

exit:
    return ret;    
}

int32_t ipc_agent_get_card_type(agent_switch_card_param_e *type)
{
    uint8_t  buf[256] = {0};
    uint8_t  rsp[256] = {0};
    uint16_t ret_len = 0;
    int32_t  ret = RT_ERROR;

    buf[0] = AGENT_CMD_GET_PARAM;
    buf[1] = 0x00;
    buf[2] = 0x03;
    buf[3] = AGENT_CMD_PARAM_CARD_TYPE;
    buf[4] = 0x00;
    buf[5] = 0x00;
    if (lib_ipc_send_data(AGENT_SERVER_PATH, buf, 6, rsp, &ret_len) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "lib_ipc_send_data failed\r\n");
        goto exit;
    }
    if (ret_len != 4 && rsp[0] != AGENT_RESULT_OK) {
        MSG_PRINTF(LOG_ERR, "ipc_agent_get_card_type ret_len: %d, rsp[0]: %d\r\n", ret_len, rsp[0]);
        goto exit;
    }
    *type = rsp[3];
    ret = RT_SUCCESS;

exit:
    return ret;
}

int32_t ipc_agent_get_sim_monitor(agent_set_sim_monitor_param_e *state)
{
    uint8_t  buf[256] = {0};
    uint8_t  rsp[256] = {0};
    uint16_t ret_len = 0;
    int32_t  ret = RT_ERROR;

    buf[0] = AGENT_CMD_GET_PARAM;
    buf[1] = 0x00;
    buf[2] = 0x03;
    buf[3] = AGENT_CMD_PARAM_SIM_MONITOR;
    buf[4] = 0x00;
    buf[5] = 0x00;
    if (lib_ipc_send_data(AGENT_SERVER_PATH, buf, 6, rsp, &ret_len) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "lib_ipc_send_data failed\r\n");
        goto exit;
    }
    if (ret_len != 4 && rsp[0] != AGENT_RESULT_OK) {
        MSG_PRINTF(LOG_ERR, "ipc_agent_get_sim_monitor ret_len: %d, rsp[0]: %d\r\n", ret_len, rsp[0]);        
        goto exit;
    }
    *state = rsp[3];
    ret = RT_SUCCESS;

exit:
    return ret;
}
