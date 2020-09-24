
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

#include "rt_type.h"

#ifdef   CFG_STANDARD_MODULE


#include "rt_os.h"
#include "ubi.h"
#include "lpa.h"
#include "log.h"
#include "ping_task.h"
#include "agent_queue.h"
#include "at_command.h"
#include "customer_at.h"
#include "agent2monitor.h"

#define AT_TYPE_GET_INFO                '0'
#define AT_TYPE_CONFIG_UICC             '1'

#define AT_GET_EID                      '0'
#define AT_GET_ICCIDS                   '1'
#define AT_GET_UICC_TYPE                '2'
#define AT_GET_SIM_STATUS               '3'
#define AT_GET_ENV_TYPE                 '4'
#define AT_GET_DEVICE_ID                '5'

#define AT_SWITCH_TO_PROVISIONING       '0'
#define AT_SWITCH_TO_OPERATION          '1'
#define AT_CONFIG_LPA_CHANNEL           '2'
#define AT_UPGRADE_UBI_FILE             '3'
#define AT_UPDATE_ENV                   '4'
#define AT_SIM_TO_VUICC                 '5'
#define AT_VUICC_TO_SIM                 '6'

#define AT_CONTENT_DELIMITER            ','

#define AT_CFG_VUICC                    "\"vUICC\""
#define AT_CFG_EUICC                    "\"eUICC\""
#define AT_CFG_UICC_LEN                 7 // 5+2

#define RT_PROD_OTI_ADDR                "oti.redtea.io"
#define RT_STAG_OTI_ADDR                "oti-staging.redtea.io"
#define RT_QA_OTI_ADDR                  "oti-qa.redtea.io"

#define AT_CFG_PROD_ENV                 "\"prod\""
#define AT_CFG_STAG_ENV                 "\"stag\""
#define AT_CFG_ENV_LEN                  6 // 4+2
#define PROD_ENV_MODE                   0
#define STAG_ENV_MODE                   1

#define OTA_UPGRADE_OEMAPP_UBI          "oemapp.ubi"
#define OTA_UPGRADE_USR_AGENT           CFG_AGENT_RUN_PATH"rt_agent"

#define RT_DEFAULT_ICCID                "FFFFFFFFFFFFFFFFFFFF"

/*
handle function name: xxx_at_cmd_handle
*/
#define AT_CMD_DEF(label)               static at_cmd_t g_at_cmd_##label = {#label, label##_at_cmd_handle}
#define AT_CMD_INIT(label)              init_customer_at(&g_at_cmd_##label)
#define AT_CMD_INSTALL(label)           \
        {\
          AT_CMD_DEF(label);\
          AT_CMD_INIT(label);\
        }

static public_value_list_t *g_p_value_list  = NULL;

static int32_t uicc_at_cmd_handle(const char *cmd, char *rsp, int32_t len)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0, tmp_len = 0, size = 0;
    uint8_t buf[1024] = {0};
    uint8_t send_buf[1] = {0};
    rt_bool devicekey_status = RT_FALSE;

    if (*cmd == AT_CONTENT_DELIMITER) {
        if ((cmd[1] == AT_TYPE_GET_INFO) && (cmd[2] == AT_CONTENT_DELIMITER)) {
            MSG_PRINTF(LOG_DBG, "cmd=%s\n", cmd);
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
#ifdef CFG_REDTEA_READY_ON
                if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "SIM");
                } else {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "vUICC");
                }
#else
                snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, \
                    (g_p_value_list->config_info->lpa_channel_type == LPA_CHANNEL_BY_IPC) ? "vUICC" : "eUICC");
#endif
                ret = RT_SUCCESS;
            } else if (cmd[3] == AT_GET_SIM_STATUS) {
                snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, \
                    (g_p_value_list->card_info->sim_info.state == SIM_READY) ? "SIM Ready" : "SIM Not Exist");
                ret = RT_SUCCESS;

            } else if (cmd[3] == AT_GET_ENV_TYPE) {   // get environment
                if(!strcmp(g_p_value_list->config_info->oti_addr, RT_PROD_OTI_ADDR)) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "prod");
                } else if (!strcmp(g_p_value_list->config_info->oti_addr, RT_STAG_OTI_ADDR)) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "stag");
                } else if (!strcmp(g_p_value_list->config_info->oti_addr, RT_QA_OTI_ADDR)) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "qa");
                } else {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "Unknow Environment");
                }
                ret = RT_SUCCESS;
            } else if (cmd[3] == AT_GET_DEVICE_ID) {
                snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, g_p_value_list->device_info->device_id);
                ret = RT_SUCCESS;
            }
        } else if ((cmd[1] == AT_TYPE_CONFIG_UICC) && (cmd[2] == AT_CONTENT_DELIMITER)) {
            MSG_PRINTF(LOG_INFO, "cmd=%s\n", cmd);
            if (cmd[3] == AT_SWITCH_TO_PROVISIONING || cmd[3] == AT_SWITCH_TO_OPERATION) { // switch card
                uint8_t iccid[THE_ICCID_LENGTH + 1] = {0};
#ifdef CFG_REDTEA_READY_ON
                if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
                    return RT_ERROR;
                }
#endif
                if(rt_os_strlen(&cmd[5]) == 0) {
                    rt_os_memcpy(iccid, RT_DEFAULT_ICCID, THE_ICCID_LENGTH);
                } else {
                    rt_os_memcpy(iccid, &cmd[5], THE_ICCID_LENGTH);
                }

                if (cmd[3] == AT_SWITCH_TO_PROVISIONING) {
                    ret = uicc_switch_card(PROFILE_TYPE_PROVISONING, iccid);
                } else if (cmd[3] == AT_SWITCH_TO_OPERATION) {
                    ret = uicc_switch_card(PROFILE_TYPE_OPERATIONAL, iccid);
                }
                if (ret == RT_SUCCESS) {
                    /* rsp: ,para,<iccid> */
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, (char *)iccid);
                }
             } else if (cmd[3] == AT_CONFIG_LPA_CHANNEL) {  // config "vuicc" or "euicc"
#ifndef CFG_REDTEA_READY_ON
                MSG_PRINTF(LOG_INFO, "config uicc type: %s\n", &cmd[5]);
                if (!rt_os_strncasecmp(&cmd[5], AT_CFG_VUICC, AT_CFG_UICC_LEN)) {
                    ret = config_update_uicc_mode(LPA_CHANNEL_BY_IPC);
                } else if (!rt_os_strncasecmp(&cmd[5], AT_CFG_EUICC, AT_CFG_UICC_LEN)) {
                    ret = config_update_uicc_mode(LPA_CHANNEL_BY_QMI);
                }
                if (ret == RT_SUCCESS) {
                    /* rsp: ,para,"uicc-type" */
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, &cmd[5]);
                }
#endif
            } else if (cmd[3] == AT_UPGRADE_UBI_FILE) { // upgrade ubi file
                char ubi_abs_file[128] = {0};
                char real_file_name[32] = {0};
                char *p0 = NULL;
                char *p1 = NULL;

                MSG_PRINTF(LOG_INFO, "ubi file input: %s\n", &cmd[5]);
                p0 = rt_os_strchr(&cmd[5], '"');
                if (p0) {
                    p1 = rt_os_strchr(p0 + 1, '"');
                    if (p1) {
                        rt_os_memcpy(ubi_abs_file, p0 + 1, p1 - p0 - 1);
                        MSG_PRINTF(LOG_INFO, "ubi_abs_file: %s\n", ubi_abs_file);
                    }
                }
                if (rt_os_strlen(ubi_abs_file)) {
                    ret = ipc_file_verify_by_monitor(ubi_abs_file, real_file_name);
                    if (!ret && !rt_os_strcmp(OTA_UPGRADE_OEMAPP_UBI, real_file_name)) {
                        linux_delete_file(OTA_UPGRADE_USR_AGENT);  // must delete agent to create a new one !!!
                        ret = ubi_update((const char *)ubi_abs_file);
                        if (ret == RT_SUCCESS) {
                            /* rsp: ,para,"ubi_file" */
                            snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, &cmd[5]);
                        }
                    } else {
                        MSG_PRINTF(LOG_ERR, "ubi file verify fail\n");
                    }
                } else {
                    ret = RT_ERROR;
                }
            } else if (cmd[3] == AT_UPDATE_ENV) {
                MSG_PRINTF(LOG_INFO, "config env : %s\n", &cmd[5]);
                if (!rt_os_strncasecmp(&cmd[5], AT_CFG_PROD_ENV, AT_CFG_ENV_LEN)) {
                    ret = config_update_env_mode(PROD_ENV_MODE);
                } else if (!rt_os_strncasecmp(&cmd[5], AT_CFG_STAG_ENV, AT_CFG_ENV_LEN)) {
                    ret = config_update_env_mode(STAG_ENV_MODE);
                }
                if (ret == RT_SUCCESS) {
                    /* rsp: ,para,"env-type" */
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, &cmd[5]);
                }
            }
#ifdef CFG_REDTEA_READY_ON
            else if (cmd[3] == AT_SIM_TO_VUICC) {
                if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
                    devicekey_status = rt_get_devicekey_status();
                    if (devicekey_status == RT_FALSE) {
                        return RT_ERROR;
                    }

                    send_buf[0] = SIM_NO_INTERNET;
                    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_SWITCH_CARD, send_buf, sizeof(send_buf));
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "SIM switch to vUICC");
                    ret = RT_SUCCESS;
                }
            } else if (cmd[3] == AT_VUICC_TO_SIM) {
                if (g_p_value_list->card_info->type != PROFILE_TYPE_SIM && g_p_value_list->card_info->sim_info.state == SIM_READY) {
                    send_buf[0] = PROVISONING_NO_INTERNET;
                    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_SWITCH_CARD, send_buf, sizeof(send_buf));
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "vUICC switch to SIM");
                    ret =  RT_SUCCESS;
                }
            }
#endif
        }
    }
    return ret;
}

static int32_t apdu_at_cmd_handle(const char *cmd, char *rsp, int32_t len)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0, tmp_len = 0, size = 0;
    uint8_t buf[1024] = {0};

    MSG_PRINTF(LOG_INFO, "cmd=%s\n", cmd);
    if (*cmd == AT_CONTENT_DELIMITER) {

    }
}

#ifdef CFG_REDTEA_READY_ON
static int32_t dkey_at_cmd_handle(const char *cmd, char *rsp, int32_t len)
{
    int32_t ret = RT_ERROR;

    MSG_PRINTF(LOG_INFO, "cmd=%s\n", cmd);

    ret = config_update_device_key(&cmd[1]);
    if (ret == RT_TRUE) {
        /* rsp: ,para,"device key" */
        snprintf(rsp, len, "%c%s", AT_CONTENT_DELIMITER, "Welcome to RedteaReady!");
    } else if (ret == RT_FALSE) {
        snprintf(rsp, len, "%c%s", AT_CONTENT_DELIMITER, "Please enter the correct Device Key!");
    }

    return RT_SUCCESS;
}
#endif

int32_t init_at_command(void *arg)
{
    g_p_value_list = ((public_value_list_t *)arg);

    /* install "UICC" at command */
    AT_CMD_INSTALL(uicc);

    /* install "APDU" at command */
    AT_CMD_INSTALL(apdu);

#ifdef CFG_REDTEA_READY_ON
    /* install "deviceKey" at command */
    AT_CMD_INSTALL(dkey);
#endif

    return RT_SUCCESS;
}

#else

int32_t init_at_command(void *arg)
{
    (void)arg;

    return RT_SUCCESS;
}

#endif

