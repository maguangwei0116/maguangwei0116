
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

#ifdef CFG_OPEN_MODULE

#define AGENT_SERVER_PATH                             ".data.redtea.agent_server"

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
    if (ipc_send_data(AGENT_SERVER_PATH, buf, 7, rsp, &ret_len) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Command request failed\r\n");
        goto exit;
    }
    if ((ret_len != 3) || (rsp[0] != AGENT_RESULT_OK)) {
        MSG_PRINTF(LOG_ERR, "Command response is invalid len: %d, rsp[0]: %x\r\n", ret_len, rsp[0]);
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
    if (ipc_send_data(AGENT_SERVER_PATH, buf, 7, rsp, &ret_len) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Command request failed\r\n");
        goto exit;
    }
    if ((ret_len != 3) || (rsp[0] != AGENT_RESULT_OK)) {
        MSG_PRINTF(LOG_ERR, "Command response is invalid len: %d, rsp[0]: %x\r\n", ret_len, rsp[0]);
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
    if (ipc_send_data(AGENT_SERVER_PATH, buf, 6, rsp, &ret_len) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Command request failed\r\n");
        goto exit;
    }
    if ((ret_len != 4) || (rsp[0] != AGENT_RESULT_OK)) {
        MSG_PRINTF(LOG_ERR, "Command response is invalid len: %d, rsp[0]: %x\r\n", ret_len, rsp[0]);
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
    if (ipc_send_data(AGENT_SERVER_PATH, buf, 6, rsp, &ret_len) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Command request failed\r\n");
        goto exit;
    }
    if ((ret_len != 4) || (rsp[0] != AGENT_RESULT_OK)) {
        MSG_PRINTF(LOG_ERR, "Command response is invalid len: %d, rsp[0]: %x\r\n", ret_len, rsp[0]);
        goto exit;
    }
    *state = rsp[3];
    ret = RT_SUCCESS;

exit:
    return ret;
}

int32_t ipc_agent_get_iccid_list(char *iccid_list, int32_t *size) // max size 512
{
    uint8_t  buf[256] = {0};
    uint8_t  rsp[512] = {0};
    uint16_t ret_len = 0;
    int32_t  ret = RT_ERROR;

    if (iccid_list == NULL || size == NULL || *size == 0) {
        goto exit;
    }

    buf[0] = AGENT_CMD_GET_PARAM;
    buf[1] = 0x00;
    buf[2] = 0x03;
    buf[3] = AGENT_CMD_PARAM_ICCID;
    buf[4] = 0x00;
    buf[5] = 0x00;
    if (ipc_send_data(AGENT_SERVER_PATH, buf, 6, rsp, &ret_len) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Command request failed\r\n");
        goto exit;
    }

    if ((ret_len < 3) || (rsp[0] != AGENT_RESULT_OK)) {
        MSG_PRINTF(LOG_ERR, "Command response is invalid len: %d, rsp[0]: %x\r\n", ret_len, rsp[0]);
        goto exit;
    }
    ret_len = rsp[1] * 256 + rsp[2];//((uint16_t)rsp[1] << 8) | rsp[2];
    ret_len = (uint16_t)(ret_len > *size) ? *size : ret_len;

    rt_os_memcpy(iccid_list, rsp + 3, ret_len);

    *size = ret_len;
    ret = RT_SUCCESS;

exit:
    return ret;
}

int32_t ipc_agent_get_network_state(agent_registration_state_e *regist_state, agent_dsi_state_call_state_e *dial_up_state)
{
    uint8_t  buf[256] = {0};
    uint8_t  rsp[256] = {0};
    uint16_t ret_len = 0;
    int32_t  ret = RT_ERROR;

    buf[0] = AGENT_CMD_GET_PARAM;
    buf[1] = 0x00;
    buf[2] = 0x03;
    buf[3] = AGENT_CMD_PARAM_NETWORK_STATE;
    buf[4] = 0x00;
    buf[5] = 0x00;
    if (ipc_send_data(AGENT_SERVER_PATH, buf, 6, rsp, &ret_len) != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Command request failed\r\n");
        goto exit;
    }
    if ((ret_len != 5) || (rsp[0] != AGENT_RESULT_OK)) {
        MSG_PRINTF(LOG_ERR, "Command response is invalid len: %d, rsp[0]: %x\r\n", ret_len, rsp[0]);
        goto exit;
    }
    *regist_state  = rsp[3];
    *dial_up_state = rsp[4];
    ret = RT_SUCCESS;

exit:
    return ret;
}

#endif
