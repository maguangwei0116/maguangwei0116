
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
#include "network_detection.h"

#ifdef CFG_OPEN_MODULE
#include "ipc_agent_command.h"

#define AT_CONTENT_DELIMITER            ','
#define MAX_BUFFER_SIZE                  512

#define AT_CONTENT_DELIMITER            ','
#define MAX_BUFFER_SIZE                  512

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
    if (param[0] == AGENT_CMD_CARD_TYPE_SIM) {
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
    } else if (param[0] == AGENT_CMD_CARD_TYPE_VSIM) {
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
    rsp[3] = (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) ? AGENT_CMD_CARD_TYPE_SIM : AGENT_CMD_CARD_TYPE_VSIM;
    *rsp_len = 4;

    return RT_SUCCESS;
}

static int32_t get_iccids(uint8_t *rsp, uint16_t *rsp_len)
{
    int32_t ii = 0, tmp_len = 0, size = 0;
    char num_string[8];
    uint8_t buf[1024] = {0};

    rsp[0] = AGENT_RESULT_OK;
    rsp[1] = 0x00;
    rsp[2] = 0x00;
    *rsp_len = 3;

    snprintf(num_string, sizeof(num_string), "%d", g_p_value_list->card_info->num);
    tmp_len = rt_os_strlen(num_string);
    rt_os_memcpy(&buf[size], num_string, tmp_len);
    size += tmp_len;

    for (ii = 0; ii < g_p_value_list->card_info->num; ii++) {
        buf[size++] = AT_CONTENT_DELIMITER;
        tmp_len = rt_os_strlen(g_p_value_list->card_info->info[ii].iccid);
        rt_os_memcpy(&buf[size], g_p_value_list->card_info->info[ii].iccid, tmp_len);
        size += tmp_len;
        buf[size++] = AT_CONTENT_DELIMITER;
        buf[size++] = g_p_value_list->card_info->info[ii].class + '0';
        buf[size++] = AT_CONTENT_DELIMITER;
        buf[size++] = g_p_value_list->card_info->info[ii].state + '0';
    }

    tmp_len = snprintf(rsp + 3, MAX_BUFFER_SIZE - 3, "%s", buf);
    rsp[1] = (uint8_t)(tmp_len >> 8) & 0xFF;
    rsp[2] = (uint8_t)(tmp_len) & 0xFF;
    *rsp_len = (uint16_t)(tmp_len + 3);

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
        rsp[0] = AGENT_RESULT_ERR_GET_SET_PARAM;
    }

    return RT_SUCCESS;    
}

static int32_t get_network_state(uint8_t *rsp, uint16_t *rsp_len)
{
    int32_t ret = RT_ERROR;
    int32_t mode = 0;
    int32_t regist_state = 0;

    rsp[0] = AGENT_RESULT_OK;
    rsp[1] = 0x00;
    rsp[2] = 0x00;
    *rsp_len = 3;
    if (rt_qmi_get_register_state(&regist_state) != RT_SUCCESS) {
        rsp[0] = AGENT_RESULT_ERR_GET_REGISTRATION_STATE;
        goto exit;
    }
    rsp[2] = 0x02;
    rsp[3] = (uint8_t)regist_state;
    rsp[4] = (uint8_t)network_get_state();
    *rsp_len = 5;
    ret = RT_SUCCESS;
exit:
    return ret;
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
    case AGENT_CMD_PARAM_RESET:
        ret = set_vuicc_mode_and_remove_all_op_profiles(&param->value[0], param_length, rsp, rsp_len);
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
    case AGENT_CMD_PARAM_ICCID:
        ret = get_iccids(rsp, rsp_len);
        break;
    case AGENT_CMD_PARAM_NETWORK_STATE:
        ret = get_network_state(rsp, rsp_len);
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
        MSG_PRINTF(LOG_ERR, "agent_cmd length is invalid %d\r\n", len);
        goto end;
    }
    cmd = *data;
    cmd_len = (data[1] << 8) + data[2];
    if (len != (cmd_len + 3)) {
        MSG_PRINTF(LOG_ERR, "agent_cmd length is invalid len:%d, cmd_len:%d\r\n", len, cmd_len);
        goto end;
    }
    MSG_INFO_ARRAY("agent_cmd REQ: ", data, len);
    for (i = 0; i < ARRAY_SIZE(g_agent_cmd_objs); i++) {
        if (cmd == g_agent_cmd_objs[i].cmd) {
            ret = g_agent_cmd_objs[i].handle(&data[3], cmd_len, rsp, rsp_len);
            if (ret != RT_SUCCESS) {
                MSG_PRINTF(LOG_ERR, "%s FAIL\r\n", g_agent_cmd_objs[i].name);
            } else {
                MSG_INFO_ARRAY("agent_cmd RSP: ", rsp, *rsp_len);
            }
            break;
        }
    }

end:
    return ret;    
}
