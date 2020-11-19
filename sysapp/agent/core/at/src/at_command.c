
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
#define AT_GET_CUR_TYPE                 '3'

#define AT_SWITCH_TO_PROVISIONING       '0'
#define AT_SWITCH_TO_OPERATION          '1'
#define AT_CONFIG_LPA_CHANNEL           '2'
#define AT_SWITCH_CARD                  '3'

#define AT_CONTENT_DELIMITER            ','

#define AT_PROD_OTI_ADDR                "oti.redtea.io"
#define AT_STAG_OTI_ADDR                "oti-staging.redtea.io"
#define AT_QA_OTI_ADDR                  "oti-qa.redtea.io"

#define AT_CFG_SV                       "SV"
#define AT_CFG_EV                       "EV"
#define AT_CFG_PROJ_LEN                 2

#define AT_CFG_VUICC                    "vUICC"
#define AT_CFG_EUICC                    "eUICC"
#define AT_CFG_SIMF                     "SIMF"      // SIM First
#define AT_CFG_SIMO                     "SIMO"      // SIM Only
#define AT_CFG_SIMF_LEN                 4
#define AT_CFG_UICC_LEN                 5


#define RT_ENV_CONFIG                   "Config env"
#define RT_ENV_TIPS                     "Current env"
#define AT_CFG_PROD_ENV                 "prod"
#define AT_CFG_STAG_ENV                 "stag"
#define AT_CFG_ENV_LEN                  4

#define AT_SWITCH_SIM                   "SIM"
#define AT_SWITCH_VSIM                  "SOFTSIM"
#define AT_CFG_SIM_LEN                  3
#define AT_CFG_SOFTSIM_LEN              7

#define OTA_UPGRADE_OEMAPP_UBI          "oemapp.ubi"
#define OTA_UPGRADE_USR_AGENT           CFG_AGENT_RUN_PATH"rt_agent"
#define RT_DEFAULT_ICCID                "FFFFFFFFFFFFFFFFFFFF"

#define PROD_ENV_MODE                   0
#define STAG_ENV_MODE                   1

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
    rt_bool device_key_status = RT_FALSE;

    if (*cmd == AT_CONTENT_DELIMITER) {
        if ((cmd[1] == AT_TYPE_GET_INFO) && (cmd[2] == AT_CONTENT_DELIMITER)) {
            MSG_PRINTF(LOG_DBG, "uicc cmd=%s\n", cmd);
            if (cmd[3] == AT_GET_EID) {            // get EID
                snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], \
                        AT_CONTENT_DELIMITER, (char *)g_p_value_list->card_info->eid);
                ret = RT_SUCCESS;

            } else if (cmd[3] == AT_GET_ICCIDS) {  // get ICCIDs
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
                snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, buf);
                ret = RT_SUCCESS;

            } else if (cmd[3] == AT_GET_UICC_TYPE) {  // get UICC type
                /* rsp: ,cmd,"uicc-type" */
                if (g_p_value_list->config_info->sim_mode == MODE_TYPE_SIM_FIRST) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "SIM First");
                } else if (g_p_value_list->config_info->sim_mode == MODE_TYPE_EUICC) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "eUICC");
                } else if (g_p_value_list->config_info->sim_mode == MODE_TYPE_VUICC) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "vUICC");
                } else if (g_p_value_list->config_info->sim_mode == MODE_TYPE_SIM_ONLY) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "SIM Only");
                }
                ret = RT_SUCCESS;

            } else if (cmd[3] == AT_GET_CUR_TYPE) {   // Get type in using
                if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "SIM");
                } else {
                    snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "xUICC");
                }
                ret = RT_SUCCESS;
            }

        } else if ((cmd[1] == AT_TYPE_CONFIG_UICC) && (cmd[2] == AT_CONTENT_DELIMITER)) {
            MSG_PRINTF(LOG_INFO, "uicc cmd=%s\n", cmd);

            if (cmd[3] == AT_SWITCH_TO_PROVISIONING || cmd[3] == AT_SWITCH_TO_OPERATION) { // switch card
                uint8_t iccid[THE_ICCID_LENGTH + 1] = {0};

                if (g_p_value_list->config_info->proj_mode == PROJECT_SV) {
                    snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Prohibit configuration");
                    return RT_SUCCESS;
                }
                if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
                    return RT_ERROR;
                }
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
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, (char *)iccid);
                }

            } else if (cmd[3] == AT_CONFIG_LPA_CHANNEL) {  // config SIM First/eUICC/vUICC/SIM Only
                MSG_PRINTF(LOG_INFO, "config uicc type: %s\n", &cmd[5]);

                if (g_p_value_list->config_info->proj_mode == PROJECT_SV) {
                    snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Prohibit configuration");
                    return RT_SUCCESS;
                }

                if (!rt_os_strncasecmp(&cmd[5], AT_CFG_SIMF, AT_CFG_SIMF_LEN)) {
                    ret = config_update_uicc_mode(MODE_TYPE_SIM_FIRST);
                } else if (!rt_os_strncasecmp(&cmd[5], AT_CFG_EUICC, AT_CFG_UICC_LEN)) {
                    ret = config_update_uicc_mode(MODE_TYPE_EUICC);
                } else if (!rt_os_strncasecmp(&cmd[5], AT_CFG_VUICC, AT_CFG_UICC_LEN)) {
                    ret = config_update_uicc_mode(MODE_TYPE_VUICC);
                } else if (!rt_os_strncasecmp(&cmd[5], AT_CFG_SIMO, AT_CFG_SIMF_LEN)) {
                    ret = config_update_uicc_mode(MODE_TYPE_SIM_ONLY);
                }
                if (ret == RT_SUCCESS) {
                    snprintf(rsp, len, "%c%c%c%s", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, &cmd[5]);
                }

            } else if (cmd[3] == AT_SWITCH_CARD) { // Switch card
                MSG_PRINTF(LOG_INFO, "Switch to %s\n", &cmd[5]);
                device_key_status = rt_get_device_key_status();
                if (device_key_status == RT_FALSE) {
                    snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "DeviceKey verification failed");
                    return RT_SUCCESS;
                } else if (g_p_value_list->config_info->sim_mode == MODE_TYPE_SIM_ONLY) {
                    snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "SIM Only prohubit switch card");
                    return RT_SUCCESS;
                }

                if (!rt_os_strncasecmp(&cmd[5], AT_SWITCH_SIM, AT_CFG_SIM_LEN)) {           // Switch to SIM
                    if (g_p_value_list->card_info->type != PROFILE_TYPE_SIM && g_p_value_list->card_info->sim_info.state == SIM_READY) {
                        send_buf[0] = PROVISONING_NO_INTERNET;
                        snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "xUICC switch to SIM");
                        msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_SWITCH_CARD, send_buf, sizeof(send_buf));
                        ret = RT_SUCCESS;
                    } else if (g_p_value_list->card_info->sim_info.state == SIM_ERROR) {
                        snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Switch failed, SIM not exist");
                        ret = RT_SUCCESS;
                    } else if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
                        snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Switch failed, SIM is using");
                        ret = RT_SUCCESS;
                    }
                } else if (!rt_os_strncasecmp(&cmd[5], AT_SWITCH_VSIM, AT_CFG_SOFTSIM_LEN)) {   // Switch to vUICC
                    if (g_p_value_list->card_info->type == PROFILE_TYPE_SIM) {
                        send_buf[0] = SIM_NO_INTERNET;
                        snprintf(rsp, len, "%c%c%c\"%s\"", AT_CONTENT_DELIMITER, cmd[3], AT_CONTENT_DELIMITER, "SIM switch to xUICC");
                        msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_SWITCH_CARD, send_buf, sizeof(send_buf));
                        ret = RT_SUCCESS;
                    } else {
                        snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Switch failed, xUICC is using");
                        ret = RT_SUCCESS;
                    }
                }
            }
        }
    }
    return ret;
}

static int32_t dkey_at_cmd_handle(const char *cmd, char *rsp, int32_t len)
{
    int32_t ret = RT_ERROR;
    rt_bool device_key_status = RT_FALSE;

    MSG_PRINTF(LOG_INFO, "devicekey cmd=%s\n", cmd);

    if (*cmd == AT_CONTENT_DELIMITER) {
        ret = config_update_device_key(&cmd[1]);
        if (ret == RT_TRUE) {
            snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Welcome to RedteaReady!");
        } else if (ret == RT_FALSE) {
            snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Please enter the correct device key!");
        }
    } else {
        device_key_status = rt_get_device_key_status();
        if (device_key_status == RT_TRUE) {
            snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Verification successed!");
        } else {
            snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Verification failed!");
        }
    }

    return RT_SUCCESS;
}

static int32_t update_at_cmd_handle(const char *cmd, char *rsp, int32_t len)
{
    int32_t ret = RT_ERROR;
    char ubi_abs_file[128] = {0};
    char real_file_name[32] = {0};
    char *p0 = NULL;
    char *p1 = NULL;

    MSG_PRINTF(LOG_INFO, "ubi file input: %s\n", &cmd[1]);
    p0 = rt_os_strchr(&cmd[1], '"');
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
                snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Update successed!");
            }
        } else {
            MSG_PRINTF(LOG_ERR, "ubi file verify fail\n");
        }
    } else {
        ret = RT_ERROR;
    }

    return ret;
}

static int32_t env_at_cmd_handle(const char *cmd, char *rsp, int32_t len)
{
    int32_t ret = RT_ERROR;
    MSG_PRINTF(LOG_INFO, "environment cmd=%s\n", cmd);

    if (*cmd == AT_CONTENT_DELIMITER) {
        if (!rt_os_strncasecmp(&cmd[1], AT_CFG_PROD_ENV, AT_CFG_ENV_LEN)) {
            ret = config_update_env_mode(PROD_ENV_MODE);
        } else if (!rt_os_strncasecmp(&cmd[1], AT_CFG_STAG_ENV, AT_CFG_ENV_LEN)) {
            ret = config_update_env_mode(STAG_ENV_MODE);
        }
        if (ret == RT_SUCCESS) {
            snprintf(rsp, len, "%c\"%s : %s\"", AT_CONTENT_DELIMITER, RT_ENV_CONFIG, &cmd[1]);
        }
    } else {
        if(!strcmp(g_p_value_list->config_info->oti_addr, AT_PROD_OTI_ADDR)) {
            snprintf(rsp, len, "%c\"%s : %s\"", AT_CONTENT_DELIMITER, RT_ENV_TIPS, "Prod");
        } else if (!strcmp(g_p_value_list->config_info->oti_addr, AT_STAG_OTI_ADDR)) {
            snprintf(rsp, len, "%c\"%s : %s\"", AT_CONTENT_DELIMITER, RT_ENV_TIPS, "Stag");
        } else if (!strcmp(g_p_value_list->config_info->oti_addr, AT_QA_OTI_ADDR)) {
            snprintf(rsp, len, "%c\"%s : %s\"", AT_CONTENT_DELIMITER, RT_ENV_TIPS, "QA");
        } else {
            snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, "Unknow Environment");
        }
        ret = RT_SUCCESS;
    }

    return ret;
}

static int32_t option_at_cmd_handle(const char *cmd, char *rsp, int32_t len)
{
    int32_t ret = RT_ERROR;
    MSG_PRINTF(LOG_INFO, "option cmd=%s\n", cmd);

    if (*cmd == AT_CONTENT_DELIMITER) {
        if (!rt_os_strncasecmp(&cmd[1], AT_CFG_SV, AT_CFG_PROJ_LEN)) {
            ret = config_update_proj_mode(PROJECT_SV);
        } else if (!rt_os_strncasecmp(&cmd[1], AT_CFG_EV, AT_CFG_PROJ_LEN)) {
            ret = config_update_proj_mode(PROJECT_EV);
        }
        if (ret == RT_SUCCESS) {
            snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, &cmd[1]);
        }
    } else {
        snprintf(rsp, len, "%c\"%s\"", AT_CONTENT_DELIMITER, (g_p_value_list->config_info->proj_mode == PROJECT_SV) ? "Standard version" : "Enterprise version");
        ret = RT_SUCCESS;
    }

    return ret;
}

int32_t init_at_command(void *arg)
{
    g_p_value_list = ((public_value_list_t *)arg);

    if (g_p_value_list->config_info->proj_mode == PROJECT_SV) {
        /* install "DeviceKey" at command */
        AT_CMD_INSTALL(dkey);
    }

    /* install "UICC" at command */
    AT_CMD_INSTALL(uicc);

    /* install "Update" at command */
    AT_CMD_INSTALL(update);

    /* install "Environment" at command */
    AT_CMD_INSTALL(env);

    /* install "Option" at command */
    AT_CMD_INSTALL(option);

    return RT_SUCCESS;
}

#else

int32_t init_at_command(void *arg)
{
    (void)arg;

    return RT_SUCCESS;
}

#endif
