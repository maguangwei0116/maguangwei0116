
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ping_task.c
 * Date        : 2020.05.10
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "rt_os.h"
#include "network_detection.h"
#include "agent_queue.h"
#include "usrdata.h"
#include "ping_task.h"

#define RT_LOST_ALL         100
#define RT_AND              1
#define RT_OR               0
#define RT_INSPECT_FILE     128

#define EXCELLENT_DELAY     100
#define GOOD_DELAY          200
#define COMMON_DELAY        500

#define EXCELLENT_LOST      0
#define GOOD_LOST           2
#define COMMON_LOST         5

#define EXCELLENT_MDEV      20
#define GOOD_MDEV           50
#define COMMON_MDEV         100

#define EXCELLENT           3
#define GOOD                2
#define COMMON              1
#define NONE_DEFINE         0

static profile_type_e *             g_sim_type = NULL;
static rt_bool                      g_to_start = RT_FALSE;
static msg_mode_e                   g_ping_task_network_state = MSG_NETWORK_CONNECTED;

int32_t ping_task_get_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    (void)buf;
    (void)len;

    if (MSG_NETWORK_CONNECTED == mode) {
        g_ping_task_network_state = MSG_NETWORK_CONNECTED;
        MSG_PRINTF(LOG_DBG, "network connecte\n");
    } else if (MSG_NETWORK_DISCONNECTED == mode) {
        g_ping_task_network_state = MSG_NETWORK_DISCONNECTED;
        MSG_PRINTF(LOG_DBG, "network disconnected\n");
    }

    return RT_SUCCESS;
}

int32_t ping_provisoning()
{
    MSG_PRINTF(LOG_DBG, "g_ping_task_network_state is %d\n", g_ping_task_network_state);

    if (g_ping_task_network_state == MSG_NETWORK_DISCONNECTED) {
        return RT_ERROR;
    } else {
        return RT_SUCCESS;
    }
}

int32_t rt_ping_event(int8_t *ip, int32_t level, int32_t type)
{
    int32_t lost;
    int32_t network_level;
    double delay, shake;

    MSG_PRINTF(LOG_DBG, "ip : %s, level %d, type : %d\n", ip, level, type);

    rt_local_ping(ip, &delay, &lost, &shake);

    if ((delay <= EXCELLENT_DELAY) && (lost == EXCELLENT_LOST) && (shake <= EXCELLENT_MDEV)) {
        network_level = EXCELLENT;

    } else if ( (delay <= GOOD_DELAY) && ( (lost <= GOOD_LOST) || (shake <= GOOD_MDEV))) {
        network_level = GOOD;

    } else if ( (delay <= COMMON_DELAY) && ( (lost <= COMMON_LOST) || (shake <= COMMON_MDEV))) {
        network_level = COMMON;

    } else {
        network_level = NONE_DEFINE;
    }

    MSG_PRINTF(LOG_DBG, "ping %s network_level : %d, operational is %lf----%lf----%d\n", ip, network_level, delay, shake, lost);

    return network_level;
}

int32_t network_detect_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    static rt_bool device_key_status;
    static rt_bool network_detect_status;
    int32_t ret = RT_ERROR;
    int8_t recv_buff;
    recv_buff = buf[0];

    if (recv_buff == DEVICE_KEY_SUCESS) {
        device_key_status = RT_TRUE;
    } else if (recv_buff == DEVICE_KEY_ERROR) {
        device_key_status = RT_FALSE;
    } else if (recv_buff == NETWORK_DETECT_SUCESS) {
        network_detect_status = RT_TRUE;
    } else if (recv_buff == NETWORK_DETECT_ERROR) {
        network_detect_status = RT_FALSE;
    }

    if(device_key_status == RT_TRUE && network_detect_status == RT_TRUE) {
        switch (mode) {
            case MSG_NETWORK_DETECT:
                g_to_start = RT_TRUE;
                MSG_PRINTF(LOG_DBG, "wait for network detect()\n");
                ret = RT_SUCCESS;
                break;
            default:
                break;
        }
    } else {
        g_to_start = RT_FALSE;
    }

    return ret;
}

static void network_ping_task(void *arg)
{
    int32_t i = 0;
    int32_t ii = 0;
    int32_t ret = RT_ERROR;
    int32_t strategy_num = NULL;
    int8_t inspect_file[RT_INSPECT_FILE] = {0};
    uint8_t tmp_buffer[RT_STRATEGY_LIST_LEN + 1] = {0};
    rt_bool detect_flg = RT_FALSE;
    cJSON *enabled = NULL;
    cJSON *interval = NULL;
    cJSON *network_detect = NULL;
    cJSON *strategy_list = NULL;
    cJSON *strategy_item = NULL;
    cJSON *domain = NULL;
    cJSON *level = NULL;
    cJSON *rt_type = NULL;

    char send_buf[1] = {0};
    int32_t network_level;

    while (1) {
        MSG_PRINTF(LOG_DBG, "g_sim_type is %d\n", *g_sim_type);

        if (!linux_rt_file_exist(RUN_CONFIG_FILE)) {
            rt_create_file(RUN_CONFIG_FILE);
        }

        snprintf(inspect_file, sizeof(RT_DATA_PATH) + sizeof(RUN_CONFIG_FILE), "%s%s", RT_DATA_PATH, RUN_CONFIG_FILE);
        detect_flg = inspect_device_key(inspect_file);

        rt_read_strategy(0, tmp_buffer, RT_STRATEGY_LIST_LEN);
        if (tmp_buffer[0] != '{') {
            MSG_PRINTF(LOG_ERR, "Read data is error, Use default monitor strategy !\n");
            rt_write_default_strategy();
        }

        if (tmp_buffer != NULL) {
            network_detect = cJSON_Parse(tmp_buffer);
            if (network_detect != NULL) {
                enabled = cJSON_GetObjectItem(network_detect, "enabled");
                interval = cJSON_GetObjectItem(network_detect, "interval");
            } else {
                MSG_PRINTF(LOG_ERR, "cJSON_Parse error !\n");
            }
        }

        if (/*detect_flg == RT_TRUE && */enabled->valueint == RT_TRUE) {        // test !!!
            g_to_start = RT_TRUE;
        } else {
            g_to_start = RT_FALSE;
        }

        MSG_PRINTF(LOG_DBG, "======================> wait %d min\n", interval->valueint);

        rt_os_sleep(interval->valueint * 60);       // first wait interval mins for dail up

        if (g_to_start == RT_TRUE) {

            MSG_PRINTF(LOG_DBG, "start monitor !!!!!!!!!!\n");

            rt_type = cJSON_GetObjectItem(network_detect, "type");

            strategy_list = cJSON_GetObjectItem(network_detect, "strategies");
            if (strategy_list != NULL) {
                strategy_num = cJSON_GetArraySize(strategy_list);
                MSG_PRINTF(LOG_DBG, "strategy_num is %d\n", strategy_num);

                for (ii = 0; ii < strategy_num; ii++) {
                    strategy_item = cJSON_GetArrayItem(strategy_list, ii);
                    domain = cJSON_GetObjectItem(strategy_item, "domain");
                    level = cJSON_GetObjectItem(strategy_item, "level");

                    network_level = rt_ping_event(domain->valuestring, level->valueint, rt_type->valueint);

                    if (rt_type->valueint == RT_OR) {
                        if (network_level >= level->valueint) {     // 当为or时，第一个满足则break;
                            ret = RT_SUCCESS;
                            break;
                        }
                    } else if (rt_type->valueint == RT_AND) {
                        if (network_level < level->valueint) {     // 当为and时，第一个不满足则break;
                            ret = RT_ERROR;
                            break;
                        }
                    }
                }
            } else {
                MSG_PRINTF(LOG_WARN, "strategies list is error\n");
            }

            if (*g_sim_type == PROFILE_TYPE_PROVISONING) {
                if (ret == RT_SUCCESS) {
                    send_buf[0] = PROVISONING_HAVE_INTERNET;
                } else {
                    send_buf[0] = PROVISONING_NO_INTERNET;
                }

            } else if (*g_sim_type == PROFILE_TYPE_OPERATIONAL) {
                if (ret == RT_SUCCESS) {
                    send_buf[0] = OPERATIONAL_HAVE_INTERNET;
                } else {
                    send_buf[0] = OPERATIONAL_NO_INTERNET;
                }

            } else if (*g_sim_type == PROFILE_TYPE_SIM) {
                if (ret == RT_SUCCESS) {
                    send_buf[0] = SIM_CARD_HAVE_INTERNET;
                } else {
                    send_buf[0] = SIM_CARD_NO_INTERNET;
                }
            } else {
                MSG_PRINTF(LOG_INFO, "unkown g_sim_type is %d\n", *g_sim_type);
            }

            msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_PING_RES, send_buf, sizeof(send_buf));
        }

        if (network_detect != NULL) {
            cJSON_Delete(network_detect);
        }
    }

exit_entry:

    rt_exit_task(NULL);
}

int32_t init_ping_task(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;
    int32_t uicc_mode = ((public_value_list_t *)arg)->config_info->sim_mode;

    if (uicc_mode == SIM_MODE_TYPE_SIM_FIRST || uicc_mode == SIM_MODE_TYPE_VUICC_ONLY) {
        MSG_PRINTF(LOG_DBG, "uicc_mode is %d\n", uicc_mode);
    } else if (uicc_mode == SIM_MODE_TYPE_SIM_ONLY) {
        MSG_PRINTF(LOG_DBG, "uicc_mode is %d\n", uicc_mode);
        return RT_SUCCESS;
    } else {
        MSG_PRINTF(LOG_DBG, "uicc_mode is %d\n", uicc_mode);
        return RT_ERROR;
    }

    g_sim_type = (profile_type_e *)&(((public_value_list_t *)arg)->card_info->type);
    MSG_PRINTF(LOG_DBG, "g_sim_type is %d\n", *g_sim_type);

    ret = rt_create_task(&task_id, (void *)network_ping_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}


