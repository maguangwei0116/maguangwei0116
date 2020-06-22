
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

// 延迟
#define EXCELLENT_DELAY         100
#define GOOD_DELAY              200
#define COMMON_DELAY            500

// 丢包
#define EXCELLENT_LOST          0
#define GOOD_LOST               2
#define COMMON_LOST             5
#define PROVISONING_LOST        100

// 抖动
#define EXCELLENT_MDEV          20
#define GOOD_MDEV               50
#define COMMON_MDEV             100

// 等级
#define COMMON                  1
#define GOOD                    2
#define EXCELLENT               3

#define RT_OR                   0
#define RT_AND                  1

#define RT_INIT_TIME            80
#define PROVISONING_PING_IP     "23.91.101.68"

static rt_bool              g_sim_switch            = RT_TRUE;
static rt_bool              g_external_cut_card     = RT_FALSE;
static profile_type_e *     g_card_type             = NULL;
static profile_sim_cpin_e * g_sim_cpin              = NULL;
static sim_mode_type_e *    g_sim_mode              = NULL;
static msg_mode_e           g_network_state         = MSG_NETWORK_DISCONNECTED;

extern void rt_external_cut_card()
{
    g_external_cut_card = RT_TRUE;
}

static void sim_switch_enable()
{
    g_sim_switch = RT_TRUE;
}

static void sim_switch_disable()
{
    g_sim_switch = RT_FALSE;
}

static int32_t rt_ping_provisoning_get_status()
{
    int32_t lost;
    int32_t ret = RT_ERROR;
    double delay, mdev;

    rt_local_ping(PROVISONING_PING_IP, &delay, &lost, &mdev);

    if (lost < PROVISONING_LOST) {      // 种子卡丢包小于100%，则认为网络通畅
        ret = RT_SUCCESS;
    }

    return ret;
}

static int32_t rt_ping_get_level(int8_t *ip, int32_t level, int32_t type)
{
    int32_t lost;
    int32_t network_level = 0;
    double delay, mdev;

    rt_local_ping(ip, &delay, &lost, &mdev);

    if ((delay <= EXCELLENT_DELAY) && (lost == EXCELLENT_LOST) && (mdev <= EXCELLENT_MDEV)) {      // 丢包=0;  延时<=100; 抖动<=20;
        network_level = EXCELLENT;
    } else if ( (delay <= GOOD_DELAY) && ( (lost <= GOOD_LOST) || (mdev <= GOOD_MDEV))) {          // 丢包<=2%; 延时<=200; 抖动<=50;
        network_level = GOOD;
    } else if ( (delay <= COMMON_DELAY) && ( (lost <= COMMON_LOST) || (mdev <= COMMON_MDEV))) {    // 丢包<=5%; 延时<=500; 抖动<=10;
        network_level = COMMON;
    }

    MSG_PRINTF(LOG_DBG, "ping %s, delay:%lf, lost:%d, mdev:%lf\n", ip, delay, lost, mdev);
    MSG_PRINTF(LOG_DBG, "expected level : %d, result : %d\n", level, network_level);

    return network_level;
}

static int32_t rt_send_msg_card_status(int32_t ret)
{
    char send_buf[1] = {0};

    if (*g_card_type == PROFILE_TYPE_PROVISONING) {
        if (ret == RT_SUCCESS) {
            send_buf[0] = PROVISONING_HAVE_INTERNET;
            MSG_PRINTF(LOG_INFO, "====> Hold Provisioning\r\n");
            return RT_SUCCESS;
        } else {
            if (*g_sim_mode == SIM_MODE_TYPE_VUICC_ONLY) {      // vUICC only: 种子卡没网, 等待下一次bootstrap
                return RT_SUCCESS;
            }
            sim_switch_disable();                   // 当种子卡没网切换到实体卡后, 不再进行网络检测
            send_buf[0] = PROVISONING_NO_INTERNET;
        }

    } else if (*g_card_type == PROFILE_TYPE_OPERATIONAL) {
        if (ret == RT_SUCCESS) {
            send_buf[0] = OPERATIONAL_HAVE_INTERNET;
            MSG_PRINTF(LOG_INFO, "====> Hold Opeational\r\n");
            return RT_SUCCESS;
        } else {
            send_buf[0] = OPERATIONAL_NO_INTERNET;
        }

    } else if (*g_card_type == PROFILE_TYPE_SIM) {
        if (ret == RT_SUCCESS) {
            send_buf[0] = SIM_CARD_HAVE_INTERNET;
            MSG_PRINTF(LOG_INFO, "====> Hold SIM\n");
            return RT_SUCCESS;
        } else {
            send_buf[0] = SIM_CARD_NO_INTERNET;
        }
    } else {
        MSG_PRINTF(LOG_INFO, "unkown g_card_type is %d\n", *g_card_type);
    }

    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_PING_RES, send_buf, sizeof(send_buf));

    return RT_SUCCESS;
}

static rt_bool rt_get_devicekey_status()
{
    uint8_t  inspect_file[128] = {0};
    snprintf(inspect_file, sizeof(RT_DATA_PATH) + sizeof(RUN_CONFIG_FILE), "%s%s", RT_DATA_PATH, RUN_CONFIG_FILE);

    return inspect_device_key(inspect_file);
}

/*
    三种情况需要等待:
    1. 当识别到外部切卡;
    2. 当种子卡ping通后, 后续不在进行ping;
    3. 网络故障, 从Provisioning-->SIM, 后续不再进行网络检测;
*/
static void rt_judge_card_status(profile_type_e *last_card_type)
{
    while (1) {
        MSG_PRINTF(LOG_INFO, "last type : %d, now : %d\n", *last_card_type, *g_card_type);

        if (*last_card_type != *g_card_type || g_external_cut_card == RT_TRUE) {
            MSG_PRINTF(LOG_DBG, "card type switch [%d] ====> [%d]\n", *last_card_type, *g_card_type);
            *last_card_type = *g_card_type;

            /*
                两种情况需要重新拨号：
                1. 切卡后, 网络状态为断开则重新拨号; (如业务卡切到种子卡,拨号还在进行,则预留时间拨号)
                2. 平台下发指令 : switch to SIM, enable, delete, push_ac; 切换到下一张业务卡
            */
            if (g_network_state == MSG_NETWORK_DISCONNECTED || (g_external_cut_card == RT_TRUE && g_network_state == MSG_NETWORK_DISCONNECTED) ) {
                MSG_PRINTF(LOG_DBG, "reset dial up !\n");
                g_external_cut_card = RT_FALSE;
                network_force_down();
                sleep(RT_INIT_TIME);    // 经验值, 后续是否需要修改
            }
        }

        if (*g_card_type == PROFILE_TYPE_PROVISONING && g_network_state == MSG_NETWORK_CONNECTED) {       // 当种子卡ping通后, 后续不在进行ping
            // MSG_PRINTF(LOG_INFO, "====> provisoning network well !\n");
            sleep(10);       // 后续是否需要修改
            continue;
        }

        if (*g_card_type == PROFILE_TYPE_SIM && g_sim_switch == RT_FALSE) {                              // Provisioning-->SIM, 不再进行网络检测
            // MSG_PRINTF(LOG_ERR, "====> sim network bad !\n");
            sleep(10);       // 后续是否需要修改
            continue;
        }

        break;
    }
}

static void network_ping_task(void *arg)
{
    int32_t ii = 0;
    int32_t ret = RT_ERROR;
    int32_t network_level;
    int32_t strategy_num = NULL;
    uint8_t tmp_buffer[RT_STRATEGY_LIST_LEN + 1] = {0};
    rt_bool devicekey_status = RT_FALSE;
    rt_bool ping_start = RT_FALSE;
    cJSON *enabled = NULL;
    cJSON *interval = NULL;
    cJSON *network_detect = NULL;
    cJSON *strategy_list = NULL;
    cJSON *strategy_item = NULL;
    cJSON *domain = NULL;
    cJSON *level = NULL;
    cJSON *rt_type = NULL;
    profile_type_e last_card_type = *g_card_type;

    if (*g_sim_mode == SIM_MODE_TYPE_SIM_FIRST) {
        if (*g_sim_cpin == SIM_CPIN_ERROR) {
            MSG_PRINTF(LOG_ERR, "cpin error!\n");
            rt_send_msg_card_status(RT_ERROR);          // 开机没有检测到实体卡, 切换至vUICC
        } else if (*g_sim_cpin == SIM_CPIN_READY) {
            MSG_PRINTF(LOG_INFO, "cpin ready\n");
        }
    }

    sleep(RT_INIT_TIME);          // 驻网拨号等初始化完成后再开始网络监测

    while (1) {
        devicekey_status = rt_get_devicekey_status();
        if (devicekey_status == RT_FALSE) {
            sleep(60);      // 后续是否需要修改; device key校验失败60s后再检测
            continue;
        }

        rt_judge_card_status(&last_card_type);
        rt_inspect_monitor_strategy(RT_RUN_CHECK);

        rt_read_strategy(0, tmp_buffer, RT_STRATEGY_LIST_LEN);
        if (tmp_buffer != NULL) {
            network_detect = cJSON_Parse(tmp_buffer);
            if (network_detect != NULL) {
                enabled = cJSON_GetObjectItem(network_detect, "enabled");
                interval = cJSON_GetObjectItem(network_detect, "interval");
            } else {
                MSG_PRINTF(LOG_ERR, "cJSON_Parse error, Use default monitor strategy !\n");
                rt_write_default_strategy();
                continue;
            }
        }

        if (devicekey_status == RT_TRUE && enabled->valueint == RT_TRUE) {
            ping_start = RT_TRUE;
        } else {
            ping_start = RT_FALSE;
        }

        if (ping_start == RT_TRUE) {
            if (*g_card_type == PROFILE_TYPE_PROVISONING) {
                ret = rt_ping_provisoning_get_status();

            } else if (*g_card_type == PROFILE_TYPE_OPERATIONAL || *g_card_type == PROFILE_TYPE_SIM) {
                rt_type = cJSON_GetObjectItem(network_detect, "type");
                strategy_list = cJSON_GetObjectItem(network_detect, "strategies");

                if (strategy_list != NULL) {
                    strategy_num = cJSON_GetArraySize(strategy_list);
                    for (ii = 0; ii < strategy_num; ii++) {
                        strategy_item = cJSON_GetArrayItem(strategy_list, ii);
                        domain = cJSON_GetObjectItem(strategy_item, "domain");
                        level = cJSON_GetObjectItem(strategy_item, "level");
                        network_level = rt_ping_get_level(domain->valuestring, level->valueint, rt_type->valueint);

                        if (rt_type->valueint == RT_AND) {
                            if (network_level >= level->valueint) {
                                ret = RT_SUCCESS;
                                break;
                            } else {
                                ret = RT_ERROR;
                            }
                        } else if (rt_type->valueint == RT_OR) {
                            if (network_level < level->valueint) {
                                ret = RT_ERROR;
                                break;
                            } else {
                                ret = RT_SUCCESS;
                            }
                        }
                    }
                } else {
                    MSG_PRINTF(LOG_ERR, "strategies list is error !\n");
                }
            }

            rt_send_msg_card_status(ret);
        }

        if (network_detect != NULL) {
            cJSON_Delete(network_detect);
        }

        MSG_PRINTF(LOG_INFO, "====> wait %d min\n", interval->valueint);
        rt_os_sleep(interval->valueint * 60);
    }

exit_entry:

    rt_exit_task(NULL);
}

int32_t ping_task_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    (void)buf;
    (void)len;

    if (mode == MSG_NETWORK_CONNECTED) {
        g_network_state = MSG_NETWORK_CONNECTED;
        if (*g_card_type == PROFILE_TYPE_SIM) {
            sim_switch_enable();
        }
        MSG_PRINTF(LOG_DBG, "====> ping task msg recv network connecte\n");
    } else if (mode == MSG_NETWORK_DISCONNECTED) {
        g_network_state = MSG_NETWORK_DISCONNECTED;
        MSG_PRINTF(LOG_DBG, "====> ping task msg recv network disconnected\n");
    }

    return RT_SUCCESS;
}

int32_t init_ping_task(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;
    g_sim_mode  = (sim_mode_type_e *)&((public_value_list_t *)arg)->config_info->sim_mode;
    g_card_type = (profile_type_e *)&(((public_value_list_t *)arg)->card_info->type);
    g_sim_cpin  = (profile_sim_cpin_e *)&(((public_value_list_t *)arg)->card_info->sim_info.state);

    if (*g_sim_mode == SIM_MODE_TYPE_SIM_ONLY) {
        MSG_PRINTF(LOG_ERR, "SIM Only, Not open ping task ...\n");
        return RT_ERROR;
    }

    ret = rt_create_task(&task_id, (void *)network_ping_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}
