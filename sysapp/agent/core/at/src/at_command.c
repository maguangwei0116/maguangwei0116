
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : at_command.c
 * Date        : 2019.12.10
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/
#include "agent_queue.h"
#include "at_command.h"

#define       AT_TYPE_GET_INFO               '0'
#define       AT_TYPE_CONFIG_UICC            '1'

#define       AT_GET_EID                     '0'
#define       AT_GET_ICCIDS                  '1'
#define       AT_GET_UICC_TYPE               '2'

#define       AT_SWITCH_TO_PROVISIONING      '0'
#define       AT_SWITCH_TO_OPERATION         '1'
#define       AT_CONFIG_LPA_CHANNEL          '2'

#define       AT_CONTENT_DELIMITER           ','

static public_value_list_t *g_p_value_list  = NULL;

int32_t init_at_command(void *arg)
{
    g_p_value_list = ((public_value_list_t *)arg);
    return RT_SUCCESS;
}

static int32_t at_switch_card(profile_type_e type, uint8_t *iccid)
{
    int32_t ii = 0;
    int32_t len = 0;
    int32_t used_seq = 0;

    for (ii = 0; ii < g_p_value_list->card_info->num; ii++) {
        len = rt_os_strlen(g_p_value_list->card_info->info[ii].iccid);
        if (PROFILE_TYPE_PROVISONING == type) {
            if (type == g_p_value_list->card_info->info[ii].class) {
                rt_os_memcpy(iccid, g_p_value_list->card_info->info[ii].iccid, len);
                iccid[len] = '\0';
                break;
            }
        } else if ((PROFILE_TYPE_OPERATIONAL == type) \
            && (g_p_value_list->card_info->info[ii].class == PROFILE_TYPE_OPERATIONAL)){
            if (rt_os_strncmp(iccid, g_p_value_list->card_info->info[ii].iccid, len) == 0) {
                break;
            }
            if (g_p_value_list->card_info->info[ii].state == 1) {
                used_seq = ii;
            }
        }
    }

    if (g_p_value_list->card_info->num == 1) { // only one card, return error
        return RT_ERROR;
    }

    if (ii == g_p_value_list->card_info->num) {
        if (used_seq < g_p_value_list->card_info->num - 1) {
            len = rt_os_strlen(g_p_value_list->card_info->info[used_seq + 1].iccid);
            rt_os_memcpy(iccid, g_p_value_list->card_info->info[used_seq + 1].iccid, len);
        } else {
            len = rt_os_strlen(g_p_value_list->card_info->info[1].iccid);
            rt_os_memcpy(iccid, g_p_value_list->card_info->info[1].iccid, len);
        }
    }
    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_ENABLE_EXIST_CARD, iccid, rt_os_strlen(iccid));
    return RT_SUCCESS;
}

int32_t at_commnad(char *cmd, char *rsp)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0, len = 0, size = 0;
    uint8_t buf[1024];

    if (*cmd == AT_CONTENT_DELIMITER) {
        if ((cmd[1] == AT_TYPE_GET_INFO) && (cmd[2] == AT_CONTENT_DELIMITER)) {
            cmd[4] = AT_CONTENT_DELIMITER;
            rt_os_memcpy(rsp, &cmd[1], 4);
            if (cmd[3] == AT_GET_EID) {            // get eid
                rt_os_memcpy(&rsp[4], g_p_value_list->card_info->eid, rt_os_strlen(g_p_value_list->card_info->eid));
                ret = RT_SUCCESS;
            } else if (cmd[3] == AT_GET_ICCIDS) {  // get iccids
                for (ii = 0; ii < g_p_value_list->card_info->num; ii++) {
                    buf[size++] = AT_CONTENT_DELIMITER;
                    len = rt_os_strlen(g_p_value_list->card_info->info[ii].iccid);
                    rt_os_memcpy(&buf[size], g_p_value_list->card_info->info[ii].iccid, len);
                    size += len;
                    buf[size++] = AT_CONTENT_DELIMITER;
                    buf[size++] = g_p_value_list->card_info->info[ii].class + '0';
                    buf[size++] = AT_CONTENT_DELIMITER;
                    buf[size++] = g_p_value_list->card_info->info[ii].state + '0';
                }
                ret = RT_SUCCESS;
                rt_os_memcpy(&rsp[3], buf, size);
            } else if (cmd[3] == AT_GET_UICC_TYPE) {  // get uicc type
                ret = RT_SUCCESS;
                rt_os_memcpy(&rsp[4], (g_p_value_list->config_info->lpa_channel_type == LPA_CHANNEL_BY_IPC) ? "vUICC" : "eUICC", 5);
            }
        } else if ((cmd[1] == AT_TYPE_CONFIG_UICC) && (cmd[2] == AT_CONTENT_DELIMITER)) {
            cmd[4] = AT_CONTENT_DELIMITER;
            rt_os_memcpy(rsp, &cmd[1], 4);
            if (cmd[3] == AT_SWITCH_TO_PROVISIONING) {
                ret = at_switch_card(PROFILE_TYPE_PROVISONING, &cmd[5]);
            } else if (cmd[3] == AT_SWITCH_TO_OPERATION) {
                ret = at_switch_card(PROFILE_TYPE_OPERATIONAL, &cmd[5]);
            }
            if (ret == RT_SUCCESS) {
                rt_os_memcpy(&rsp[4], &cmd[5], THE_ICCID_LENGTH);
            }
            if (cmd[3] == AT_CONFIG_LPA_CHANNEL) {
                ret = RT_SUCCESS;
            }
        }
    }
    return ret;
}