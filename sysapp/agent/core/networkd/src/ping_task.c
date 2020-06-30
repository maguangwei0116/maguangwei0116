
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

#ifdef CFG_REDTEA_READY_ON

#include "rt_os.h"
#include "usrdata.h"
#include "ping_task.h"
#include "agent_queue.h"
#include "network_detection.h"

static rt_bool                      g_sim_switch            = RT_TRUE;
static profile_type_e *             g_card_type             = NULL;
static profile_sim_cpin_e *         g_sim_cpin              = NULL;
static sim_mode_type_e *            g_sim_mode              = NULL;
static rt_bool                      g_network_state         = RT_FALSE;
extern rt_bool                      g_downstream_event;

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
    int32_t ret = RT_ERROR;
    int32_t lost;
    double delay, mdev;

    ret = rt_local_ping(RT_PROVISONING_IP, &delay, &lost, &mdev);
    if (ret == RT_SUCCESS) {
        return RT_SUCCESS;
    }

    if (lost < RT_PROVISONING_LOST) {
        ret = RT_SUCCESS;
    }

    return ret;
}

static int32_t rt_ping_get_level(int8_t *ip, int32_t level, int32_t type)
{
    int32_t ret = RT_ERROR;
    int32_t network_level = 0;
    int32_t lost;
    double delay, mdev;

    ret = rt_local_ping(ip, &delay, &lost, &mdev);
    if (ret == RT_SUCCESS) {
        return RT_EXCELLENT;        // 外部切卡事件导致ping结果不准确, 等待下次网络监控
    }

    if ( (delay <= RT_EXCELLENT_DELAY) && (lost == RT_EXCELLENT_LOST) && (mdev <= RT_EXCELLENT_MDEV)) {     // 延时<=100; 丢包=0;  抖动<=20;
        network_level = RT_EXCELLENT;
    } else if ( (delay <= RT_GOOD_DELAY) && (lost <= RT_GOOD_LOST) && (mdev <= RT_GOOD_MDEV)) {             // 延时<=200; 丢包<=2; 抖动<=50;
        network_level = RT_GOOD;
    } else if ( (delay <= RT_COMMON_DELAY) && (lost <= RT_COMMON_LOST) && (mdev <= RT_COMMON_MDEV)) {       // 延时<=500; 丢包<=5; 抖动<=100;
        network_level = RT_COMMON;
    }

    MSG_PRINTF(LOG_INFO, "ping %s, delay:%lf, lost:%d, mdev:%lf\n", ip, delay, lost, mdev);
    MSG_PRINTF(LOG_INFO, "expected level : %d, result : %d\n", level, network_level);

    return network_level;
}

static int32_t rt_send_msg_card_status()
{
    char send_buf[1] = {0};

    if (*g_card_type == PROFILE_TYPE_PROVISONING) {
        if (*g_sim_mode == SIM_MODE_TYPE_VUICC_ONLY || *g_sim_cpin == SIM_ERROR) {
            return RT_SUCCESS;
        }
        sim_switch_disable();
        send_buf[0] = PROVISONING_NO_INTERNET;

    } else if (*g_card_type == PROFILE_TYPE_OPERATIONAL) {
        send_buf[0] = OPERATIONAL_NO_INTERNET;

    } else if (*g_card_type == PROFILE_TYPE_SIM) {
        send_buf[0] = SIM_CARD_NO_INTERNET;
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
    1. 程序切卡且当前网络断开;
    2. 当种子卡ping通后, 后续不在进行ping;
    3. 网络故障, 从Provisioning-->SIM, 后续不再进行网络检测;
*/
static void rt_judge_card_status(profile_type_e *last_card_type)
{
    while (1) {
        if (*last_card_type != *g_card_type) {
            MSG_PRINTF(LOG_DBG, "card type switch [%d] ====> [%d]\n", *last_card_type, *g_card_type);
            *last_card_type = *g_card_type;

            if (g_network_state == RT_FALSE) {
                MSG_PRINTF(LOG_DBG, "reset dial up !\n");
                network_force_down();
                sleep(RT_DIAL_UP_TIME);     // 80s, 经验值, 后续是否需要修改
            }
        }
        if (*g_card_type == PROFILE_TYPE_PROVISONING && g_network_state == RT_TRUE) {                   // 当种子卡ping通后, 后续不在进行ping
            sleep(10);                      // 后续是否需要修改
            continue;
        }
        if (*g_card_type == PROFILE_TYPE_SIM && g_sim_switch == RT_FALSE) {                              // Provisioning-->SIM, 不再进行网络检测
            sleep(10);                      // 后续是否需要修改
            continue;
        }

        break;
    }
}

static void network_ping_task(void *arg)
{
    int32_t ii = 0;
    int32_t wait_times = 0;
    int32_t times_tmp = 0;
    int32_t ret = RT_ERROR;
    int32_t network_level;
    int32_t strategy_num = NULL;
    uint8_t tmp_buffer[RT_STRATEGY_LIST_LEN + 1] = {0};
    rt_bool devicekey_status = RT_FALSE;
    cJSON *enabled = NULL;
    cJSON *interval = NULL;
    cJSON *network_detect = NULL;
    cJSON *strategy_list = NULL;
    cJSON *strategy_item = NULL;
    cJSON *domain = NULL;
    cJSON *level = NULL;
    cJSON *type = NULL;
    profile_type_e last_card_type = *g_card_type;

    if (*g_sim_mode == SIM_MODE_TYPE_SIM_FIRST) {
        if (*g_sim_cpin != SIM_READY) {
            rt_send_msg_card_status();          // 开机检测无实体卡, 切换至vUICC
        }
    }

    sleep(RT_INIT_TIME);    // 70s, 驻网拨号等初始化完成后再开始网络监测

    while (1) {
        devicekey_status = rt_get_devicekey_status();
        if (devicekey_status == RT_FALSE) {
            sleep(RT_DEVICE_TIME);          // 后续是否需要修改; device key校验失败60s后再检测
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
                times_tmp = interval->valueint * 60;
            } else {
                MSG_PRINTF(LOG_ERR, "Json parse error, Use default monitor strategy !\n");
                rt_write_default_strategy();
                continue;
            }
        }

        if (enabled->valueint == RT_TRUE) {
            if (*g_card_type == PROFILE_TYPE_PROVISONING) {
                ret = rt_ping_provisoning_get_status();

            } else if (*g_card_type == PROFILE_TYPE_OPERATIONAL || *g_card_type == PROFILE_TYPE_SIM) {
                type = cJSON_GetObjectItem(network_detect, "type");
                strategy_list = cJSON_GetObjectItem(network_detect, "strategies");

                if (strategy_list != NULL) {
                    strategy_num = cJSON_GetArraySize(strategy_list);
                    for (ii = 0; ii < strategy_num; ii++) {
                        strategy_item = cJSON_GetArrayItem(strategy_list, ii);
                        domain = cJSON_GetObjectItem(strategy_item, "domain");
                        level = cJSON_GetObjectItem(strategy_item, "level");
                        network_level = rt_ping_get_level(domain->valuestring, level->valueint, type->valueint);

                        if (type->valueint == RT_AND) {
                            if (network_level >= level->valueint) {
                                ret = RT_SUCCESS;
                                break;
                            } else {
                                ret = RT_ERROR;
                            }
                        } else if (type->valueint == RT_OR) {
                            if (network_level < level->valueint) {
                                ret = RT_ERROR;
                                break;
                            } else {
                                ret = RT_SUCCESS;
                            }
                        }
                    }
                } else {
                    MSG_PRINTF(LOG_ERR, "Strategies parse error !\n");
                }
            }

            if (ret == RT_ERROR) {
                rt_send_msg_card_status();
            }
        }

        if (network_detect != NULL) {
            cJSON_Delete(network_detect);
        }

        wait_times = times_tmp;
        while (--wait_times) {
            sleep(1);
            if (g_downstream_event == RT_TRUE) {           // 能收到平台下发的消息, 说明当前网络通畅, 等待下次监控
                MSG_PRINTF(LOG_INFO, "reset timing!\n");
                g_downstream_event = RT_FALSE;
                wait_times = times_tmp;
            }
        }
    }

exit_entry:

    rt_exit_task(NULL);
}

int32_t ping_task_network_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    switch (mode) {
        case MSG_NETWORK_CONNECTED:
            g_network_state = RT_TRUE;
            if (*g_card_type == PROFILE_TYPE_SIM) {
                sim_switch_enable();
                rt_forbid_bootstrap();
            }
            break;

        case MSG_NETWORK_DISCONNECTED:
            g_network_state = RT_FALSE;
            break;

        default:
            break;
    }

    return RT_SUCCESS;
}

int32_t sync_downstream_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    switch (mode) {
        case MSG_SYNC_DOWNSTREAM:
            g_downstream_event = RT_TRUE;
            break;

        default:
            break;
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

#endif
