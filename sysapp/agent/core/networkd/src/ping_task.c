
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

#define RT_AND                  1
#define RT_OR                   0
#define RT_INSPECT_FILE         128

#define EXCELLENT_DELAY         100
#define GOOD_DELAY              200
#define COMMON_DELAY            500

#define EXCELLENT_LOST          0
#define GOOD_LOST               2
#define COMMON_LOST             5
#define PROVISONING_LOST        100

#define EXCELLENT_MDEV          20
#define GOOD_MDEV               50
#define COMMON_MDEV             100

#define EXCELLENT               3
#define GOOD                    2
#define COMMON                  1
#define NONE_DEFINE             0

#define RT_INIT_TIME            80

#define PROVISONING_PING_IP     "23.91.101.68"

static rt_bool              g_sim_switch            = RT_TRUE;
static rt_bool              g_external_cut_card     = RT_FALSE;
static profile_type_e *     g_card_type             = NULL;
static profile_sim_cpin_e * g_sim_cpin              = NULL;
static msg_mode_e           g_network_state         = MSG_NETWORK_DISCONNECTED;

void rt_external_cut_card()
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

int32_t ping_task_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    (void)buf;
    (void)len;

    if (mode == MSG_NETWORK_CONNECTED) {
        g_network_state = MSG_NETWORK_CONNECTED;
        if (*g_card_type == PROFILE_TYPE_SIM) {
            sim_switch_enable();
        }
        MSG_PRINTF(LOG_DBG, "====> msg recv network connecte\n");
    } else if (mode == MSG_NETWORK_DISCONNECTED) {
        g_network_state = MSG_NETWORK_DISCONNECTED;
        MSG_PRINTF(LOG_DBG, "====> msg recv network disconnected\n");
    }

    return RT_SUCCESS;
}

static int32_t rt_ping_provisoning_get_status()
{
    int32_t lost;
    int32_t ret = RT_ERROR;
    double delay, shake;

    rt_local_ping(PROVISONING_PING_IP, &delay, &lost, &shake);

    if (lost < PROVISONING_LOST) {      // 种子卡丢包小于100%，则认为网络通畅
        ret = RT_SUCCESS;
    }

    return ret;
}

static int32_t rt_ping_get_level(int8_t *ip, int32_t level, int32_t type)
{
    int32_t lost;
    int32_t network_level;
    double delay, shake;

    rt_local_ping(ip, &delay, &lost, &shake);

    if ((delay <= EXCELLENT_DELAY) && (lost == EXCELLENT_LOST) && (shake <= EXCELLENT_MDEV)) {      // 丢包=0;  延时<=100; 抖动<=20;
        network_level = EXCELLENT;
    } else if ( (delay <= GOOD_DELAY) && ( (lost <= GOOD_LOST) || (shake <= GOOD_MDEV))) {          // 丢包<=2%; 延时<=200; 抖动<=50;
        network_level = GOOD;
    } else if ( (delay <= COMMON_DELAY) && ( (lost <= COMMON_LOST) || (shake <= COMMON_MDEV))) {    // 丢包<=5%; 延时<=500; 抖动<=10;
        network_level = COMMON;
    } else {
        network_level = NONE_DEFINE;
    }

    MSG_PRINTF(LOG_DBG, "ping %s, get network_level : %d\n", ip, network_level);
    MSG_PRINTF(LOG_DBG, "delay : %lf, lost : %d, shake : %lf\n", delay, lost, shake);

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
    uint8_t  inspect_file[RT_INSPECT_FILE] = {0};
    snprintf(inspect_file, sizeof(RT_DATA_PATH) + sizeof(RUN_CONFIG_FILE), "%s%s", RT_DATA_PATH, RUN_CONFIG_FILE);

    return inspect_device_key(inspect_file);
}

static void rt_judge_card_status(profile_type_e *last_card_type)
{
    while (1) {
        MSG_PRINTF(LOG_INFO, "==============> last card type : %d\n", *last_card_type);
        MSG_PRINTF(LOG_INFO, "==============> now  card type : %d\n", *g_card_type);

        if (*last_card_type != *g_card_type || g_external_cut_card == RT_TRUE) {                   // 极端情况: 切卡的同时开始网络检测, 需要预留时间拨号
            MSG_PRINTF(LOG_DBG, "card type switch [%d] ====> [%d]\n", *last_card_type, *g_card_type);
            *last_card_type = *g_card_type;

            /*
                两种情况需要重新拨号：
                1. 切卡后, 网络状态为断开则重新拨号; (如业务卡切到种子卡,拨号还在进行,则预留时间拨号)
                2. 平台下发切到SIM卡, 且当前网络断开则重新拨号
            */
            if (g_network_state == MSG_NETWORK_DISCONNECTED || (g_external_cut_card == RT_TRUE && g_network_state == MSG_NETWORK_DISCONNECTED) ) {
                MSG_PRINTF(LOG_ERR, "reset dial up !\n");
                g_external_cut_card = RT_FALSE;
                network_force_down();
                sleep(RT_INIT_TIME);                                      // 经验值, 后续是否需要修改
            }
        }

        if (*g_card_type == PROFILE_TYPE_PROVISONING && g_network_state == MSG_NETWORK_CONNECTED) {       // 当为种子卡且ping通后,则后续不进行监控
            MSG_PRINTF(LOG_INFO, "====> provisoning network well !\n");
            sleep(10);       // 后续是否需要修改
            continue;
        }

        if (*g_card_type == PROFILE_TYPE_SIM && g_sim_switch == RT_FALSE) {                              // vUICC --> SIM 则暂停网络监控
            MSG_PRINTF(LOG_ERR, "====> sim network bad !\n");
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

    if (*g_sim_cpin == SIM_CPIN_ERROR) {
        MSG_PRINTF(LOG_ERR, "cpin error!\n");
        rt_send_msg_card_status(RT_ERROR);          // 开机没有检测到实体卡, 切到vUICC
    } else if (*g_sim_cpin == SIM_CPIN_READY) {
        MSG_PRINTF(LOG_ERR, "cpin ready\n");
    }

    sleep(RT_INIT_TIME);          // 模组上电后, 需要进行驻网拨号, 初始化完成后再开始网络监测

    while (1) {

        rt_judge_card_status(&last_card_type);
        devicekey_status = rt_get_devicekey_status();
        // if (devicekey_status == RT_FALSE) {
        //     sleep(60);      // 后续是否需要修改; device key校验失败60s后再检测
        //     continue;
        // }

        // 后续优化为一个接口
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

        if (/*devicekey_status == RT_TRUE && */enabled->valueint == RT_TRUE) {
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

                        if (rt_type->valueint == RT_OR) {
                            if (network_level >= level->valueint) {     // 当为or时，第一个满足则break;
                                ret = RT_SUCCESS;
                                break;
                            } else {
                                ret = RT_ERROR;
                            }
                        } else if (rt_type->valueint == RT_AND) {
                            if (network_level < level->valueint) {     // 当为and时，第一个不满足则break;
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
            } else {
                MSG_PRINTF(LOG_ERR, "unknow card type !\n");
            }

            rt_send_msg_card_status(ret);

            MSG_PRINTF(LOG_DBG, "====> wait %d min\n", interval->valueint);
            rt_os_sleep(interval->valueint * 60);
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
    g_card_type = (profile_type_e *)&(((public_value_list_t *)arg)->card_info->type);
    g_sim_cpin  = (profile_sim_cpin_e *)&(((public_value_list_t *)arg)->card_info->sim_info.state);

    if (uicc_mode == SIM_MODE_TYPE_SIM_ONLY) {
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
