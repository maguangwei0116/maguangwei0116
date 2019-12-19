
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
#include "lpa.h"
#include "log.h"

#define       AT_TYPE_GET_INFO               '0'
#define       AT_TYPE_CONFIG_UICC            '1'

#define       AT_GET_EID                     '0'
#define       AT_GET_ICCIDS                  '1'
#define       AT_GET_UICC_TYPE               '2'

#define       AT_SWITCH_TO_PROVISIONING      '0'
#define       AT_SWITCH_TO_OPERATION         '1'
#define       AT_CONFIG_LPA_CHANNEL          '2'

#define       AT_CONTENT_DELIMITER           ','

#define       AT_CFG_VUICC                   "\"vUICC\""
#define       AT_CFG_EUICC                   "\"eUICC\""
#define       AT_CFG_UICC_LEN                7 // 5+2

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
            if (!rt_os_strncmp(iccid, g_p_value_list->card_info->info[ii].iccid, len)) {
                break;
            }
            if (g_p_value_list->card_info->info[ii].state == 1) {
                used_seq = ii;
            }
        }
    }

    if (g_p_value_list->card_info->num == 1) { // only one card, return error
        MSG_PRINTF(LOG_WARN, "only one card detected !\n");
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

int32_t at_commnad(const char *cmd, char *rsp, int32_t len)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0, tmp_len = 0, size = 0;
    uint8_t buf[1024] = {0};

    MSG_PRINTF(LOG_INFO, "cmd=%s\n", cmd);
    if (*cmd == AT_CONTENT_DELIMITER) {
        if ((cmd[1] == AT_TYPE_GET_INFO) && (cmd[2] == AT_CONTENT_DELIMITER)) {
            if (cmd[3] == AT_GET_EID) {            // get eid
                /* rsp: ,cmd,"eid" */
                snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], \
                        AT_CONTENT_DELIMITER, (char *)g_p_value_list->card_info->eid);
                ret = RT_SUCCESS;
            } else if (cmd[3] == AT_GET_ICCIDS) {  // get iccids 
                char num_string[8];

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
                /* rsp: ,cmd,nums,<iccid, class, state>,<iccid, class, state> */
                snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, buf);
                ret = RT_SUCCESS;
            } else if (cmd[3] == AT_GET_UICC_TYPE) {  // get uicc type
                /* rsp: ,cmd,"uicc-type" */
                snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, \
                    (g_p_value_list->config_info->lpa_channel_type == LPA_CHANNEL_BY_IPC) ? "vUICC" : "eUICC");
                ret = RT_SUCCESS;
            }
        } else if ((cmd[1] == AT_TYPE_CONFIG_UICC) && (cmd[2] == AT_CONTENT_DELIMITER)) {
            if (cmd[3] == AT_SWITCH_TO_PROVISIONING || cmd[3] == AT_SWITCH_TO_OPERATION) { // switch card
                uint8_t iccid[THE_ICCID_LENGTH+1] = {0};
                rt_os_memcpy(iccid, &cmd[5], THE_ICCID_LENGTH);   
                MSG_PRINTF(LOG_INFO, "iccid: %s\n", iccid);
                if (cmd[3] == AT_SWITCH_TO_PROVISIONING) {
                    ret = at_switch_card(PROFILE_TYPE_PROVISONING, iccid);
                } else if (cmd[3] == AT_SWITCH_TO_OPERATION) {
                    ret = at_switch_card(PROFILE_TYPE_OPERATIONAL, iccid);
                }
                if (ret == RT_SUCCESS) {
                    /* rsp: ,cmd,<iccid> */
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, (char *)iccid);
                }
             } else if (cmd[3] == AT_CONFIG_LPA_CHANNEL) { // config "vuicc" or "euicc"
                MSG_PRINTF(LOG_INFO, "config uicc type: %s\n", &cmd[5]);
                if (!rt_os_strncasecmp(&cmd[5], AT_CFG_VUICC, AT_CFG_UICC_LEN)) {
                    ret = config_update_uicc_mode(LPA_CHANNEL_BY_IPC);
                } else if (!rt_os_strncasecmp(&cmd[5], AT_CFG_EUICC, AT_CFG_UICC_LEN)) {
                    ret = config_update_uicc_mode(LPA_CHANNEL_BY_QMI);
                }
                if (ret == RT_SUCCESS) {
                    /* rsp: ,cmd,"uicc-type" */
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, &cmd[5]);
                }
            }
        }
    }
    return ret;
}