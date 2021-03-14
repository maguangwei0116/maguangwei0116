
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_command.c
 * Date        : 2021.03.13
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "rt_os.h"
#include "usrdata.h"
#include "ipc_task.h"
#include "ping_task.h"
#include "agent_queue.h"
#include "ipc_agent_command.h"

#ifdef CFG_OPEN_MODULE

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           sizeof(a)/sizeof(a[0])
#endif

#define INIT_AGENT_CMD_OBJ(func, cmd)     {#func, cmd, func}

typedef int32_t (*agent_command_callback)(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len);

typedef struct AGENT_CMD_OBJ
{
    const char              *name;
    agent_cmd_e             cmd;
    agent_command_callback  handle;
} agent_cmd_obj_t;

static public_value_list_t *g_p_value_list  = NULL;

static int32_t set_card_type(const uint8_t *param, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    int32_t  ret = RT_SUCCESS;
    uint8_t send_buf[1] = {0};

    rsp[0] = AGENT_RESULT_OK;
    rsp[1] = 0x00;
    rsp[2] = 0x00;
    *rsp_len = 3;

    if (g_p_value_list->config_info->sim_mode == MODE_TYPE_SIM_ONLY) {
        rsp[0] = AGENT_RESULT_ERR_SWITCH_CARD_WHEN_SIM_ONLY;
        goto exit;
    }
    if (len != 1) {
        rsp[0] = AGENT_RESULT_ERR_PARAM_LENGTH_INVALID;
        goto exit;
    }
    if (param[0] == AGENT_CMD_SET_CARD_TYPE_SIM) {
        if (g_p_value_list->card_info->type != PROFILE_TYPE_SIM && g_p_value_list->card_info->sim_info.state == SIM_READY) {
            send_buf[0] = PROVISONING_NO_INTERNET;
            msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_SWITCH_CARD, send_buf, sizeof(send_buf));
        } else if (g_p_value_list->card_info->sim_info.state == SIM_ERROR) {
            rsp[0] = AGENT_RESULT_ERR_SWITCH_CARD_SIM_NOT_EXIST;
        } else if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
            rsp[0] = AGENT_RESULT_ERR_SWITCH_CARD_SIM_IS_USING;
        } else {
            /* nothing to do, just response error code */
            rsp[0] = AGENT_RESULT_ERR_SWITCH_CARD_NOTHING_DONE;
        }
    } else if (param[0] == AGENT_CMD_SET_CARD_TYPE_VSIM) {
        if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
            send_buf[0] = SIM_NO_INTERNET;
            msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_SWITCH_CARD, send_buf, sizeof(send_buf));
        } else {
            rsp[0] = AGENT_RESULT_ERR_SWITCH_CARD_VUICC_IS_USING;
        }
    } else {
        rsp[0] = AGENT_RESULT_ERR_SWITCH_CARD_PARAM_UNKNOWN;
    }

exit:
    return ret;
}

static int32_t get_card_type(uint8_t *rsp, uint16_t *rsp_len)
{
    rsp[0] = AGENT_RESULT_OK;
    rsp[1] = 0x00;
    rsp[2] = 0x01;
    rsp[3] = (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) ? AGENT_CMD_SET_CARD_TYPE_SIM : AGENT_CMD_SET_CARD_TYPE_VSIM;
    *rsp_len = 4;

    return RT_SUCCESS;
}

static int32_t set_sim_monitor(const uint8_t *param, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    int32_t ret = RT_SUCCESS;
    uint8_t send_buf[1] = {0};
    uint8_t value = ((param[0] != 0) ? 1 : 0);

    rsp[0] = AGENT_RESULT_OK;
    rsp[1] = 0x00;
    rsp[2] = 0x00;
    *rsp_len = 3;

    if (len != 1) {
        rsp[0] = AGENT_RESULT_ERR_PARAM_LENGTH_INVALID;
        goto exit;
    }
    g_p_value_list->config_info->sim_monitor_enable = value;

    ret = config_update_sim_monitor(value);

exit:
    return ret;
}

static int32_t get_sim_monitor(uint8_t *rsp, uint16_t *rsp_len)
{
    int32_t mode = 0;

    rsp[0] = AGENT_RESULT_OK;
    rsp[1] = 0x00;
    rsp[2] = 0x00;
    *rsp_len = 3;

    if (config_get_sim_monitor(&mode) == RT_SUCCESS) {
        rsp[1] = 0x00;
        rsp[2] = 0x01;
        rsp[3] = (uint8_t)mode;
        *rsp_len = 4;
    } else {
        rsp[0] = AGENT_RESULT_ERR_GET_SET_SIM_MONITOR;
    }

    return RT_SUCCESS;    
}

static int32_t agent_set_param(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    int32_t  ret = RT_ERROR;
    uint16_t param_length = 0;
    agent_cmd_set_param_data_t *param = (agent_cmd_set_param_data_t*)data;
    param_length = (param->length[0] << 8) + param->length[1];
    switch (param->type) {
    case AGENT_CMD_PARAM_CARD_TYPE:
        ret = set_card_type(&param->value[0], param_length, rsp, rsp_len);
        break;
    case AGENT_CMD_PARAM_SIM_MONITOR:
        ret = set_sim_monitor(&param->value[0], param_length, rsp, rsp_len);
        break;
    default:
        MSG_PRINTF(LOG_ERR, "Parameter type is invalid\r\n");
        rsp[0] = AGENT_RESULT_ERR_PARAM_TYPE_UNKNOWN;
        rsp[1] = 0x00;
        rsp[2] = 0x00;
        *rsp = 3;
        break;
    }
    return ret;
}

static int32_t agent_get_param(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    int32_t  ret = RT_ERROR;
    uint16_t param_length = 0;
    agent_cmd_get_param_data_t *param = (agent_cmd_get_param_data_t *)data;
    param_length = (param->length[0] << 8) + param->length[1];
    switch (param->type) {
    case AGENT_CMD_PARAM_CARD_TYPE:
        ret = get_card_type(rsp, rsp_len);
        break;
    case AGENT_CMD_PARAM_SIM_MONITOR:
        ret = get_sim_monitor(rsp, rsp_len);
        break;
    default:
        MSG_PRINTF(LOG_ERR, "Parameter type is invalid\r\n");
        rsp[0] = AGENT_RESULT_ERR_PARAM_TYPE_UNKNOWN;
        rsp[1] = 0x00;
        rsp[2] = 0x00;
        *rsp = 3;
        break;
    }
    return ret;
}

int32_t init_agent_cmd(void *arg)
{
    g_p_value_list = (public_value_list_t *)arg;
    return RT_SUCCESS;
}

static const agent_cmd_obj_t g_agent_cmd_objs[] =
{
    INIT_AGENT_CMD_OBJ(agent_set_param, AGENT_CMD_SET_PARAM),
    INIT_AGENT_CMD_OBJ(agent_get_param, AGENT_CMD_GET_PARAM),
};

int32_t agent_cmd(const uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rsp_len)
{
    agent_cmd_e cmd = 0;
    uint16_t    cmd_len = 0;
    int32_t     i;
    int32_t     ret = RT_ERROR;

    *rsp_len = 0;
    if (len < 3) {
        MSG_PRINTF(LOG_INFO, "agent_cmd length is invalid %d", len);
        goto end;
    }
    cmd = *data;
    cmd_len = (data[1] << 8) + data[2];
    if (len != (cmd_len + 3)) {
        MSG_PRINTF(LOG_INFO, "agent_cmd length is invalid len:%d, cmd_len:%d", len, cmd_len);
        goto end;
    }
    MSG_INFO_ARRAY("agent_cmd REQ: ", data, len);
    for (i = 0; i < ARRAY_SIZE(g_agent_cmd_objs); i++) {
        if (cmd == g_agent_cmd_objs[i].cmd) {
            ret = g_agent_cmd_objs[i].handle(&data[3], cmd_len, rsp, rsp_len);
            MSG_PRINTF(LOG_INFO, "%s", g_agent_cmd_objs[i].name, !ret ? " OK " : " FAIL ");
            MSG_INFO_ARRAY("agent_cmd RSP: ", rsp, *rsp_len);
            ret = RT_SUCCESS;
        }
    }
    ret = RT_SUCCESS;

end:
    return ret;    
}

#endif // CFG_OPEN_MODULE

