
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
#define RT_NOT_DEFINE       -1
#define RT_STRATEGY_NUM     2
#define RT_DOMAIN_LEN       64
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
        MSG_PRINTF(LOG_DBG, "network connected\r\n");
    } else if (MSG_NETWORK_DISCONNECTED == mode) {
        g_ping_task_network_state = MSG_NETWORK_DISCONNECTED;
        MSG_PRINTF(LOG_DBG, "network disconnected\r\n");
    }

    return RT_SUCCESS;
}

int32_t ping_provisoning()
{
    MSG_PRINTF(LOG_DBG, "g_ping_task_network_state is %d\r\n", g_ping_task_network_state);

    if (g_ping_task_network_state == MSG_NETWORK_DISCONNECTED) {
        return RT_ERROR;
    } else {
        return RT_SUCCESS;
    }
}

int32_t ping_operational(int8_t *domain1, int32_t level1, int8_t *domain2, int32_t level2, int32_t type)
{
    int32_t pr_argc = 4;
    int32_t lost1, lost2;
    int32_t local_level1 = 0;
    int32_t local_level2 = 0;
    int32_t ret = RT_ERROR;
    int8_t *pr_argv1[] = {"redtea", "-c", "10", domain1};
    int8_t *pr_argv2[] = {"redtea", "-c", "10", domain2};
    double avg_delay1, mdev1, avg_delay2, mdev2;

    if (type == RT_NOT_DEFINE) {
        // only one
        local_ping(pr_argc, pr_argv1, &avg_delay1, &lost1, &mdev1);

        if ((avg_delay1 <= EXCELLENT_DELAY) && (lost1 == EXCELLENT_LOST) && (mdev1 <= EXCELLENT_MDEV)) {
            local_level1 = EXCELLENT;
        } else if ((avg_delay1 <= GOOD_DELAY) && ((lost1 <= GOOD_LOST) || (mdev1 <= GOOD_MDEV))) {
            local_level1 = GOOD;
        } else if ((avg_delay1 <= COMMON_DELAY) && ((lost1 <= COMMON_LOST) || (mdev1 <= COMMON_MDEV))) {
            local_level1 = COMMON;
        } else {
            local_level1 = NONE_DEFINE;
        }

        MSG_PRINTF(LOG_DBG, "ping %s operational is %lf----%lf----%d\n", domain1, avg_delay1, mdev1, lost1);

        if (local_level1 >= level1) {
            ret = RT_SUCCESS;
        } else {
            ret = RT_ERROR;
        }
    } else {
        local_ping(pr_argc, pr_argv1, &avg_delay1, &lost1, &mdev1);

        if ((avg_delay1 <= EXCELLENT_DELAY) && (lost1 == EXCELLENT_LOST) && (mdev1 <= EXCELLENT_MDEV)) {
            local_level1 = EXCELLENT;
        } else if ( (avg_delay1 <= GOOD_DELAY) && ( (lost1 <= GOOD_LOST) || (mdev1 <= GOOD_MDEV))) {
            local_level1 = GOOD;
        } else if ( (avg_delay1 <= COMMON_DELAY) && ( (lost1 <= COMMON_LOST) || (mdev1 <= COMMON_MDEV))) {
            local_level1 = COMMON;
        } else {
            local_level1 = NONE_DEFINE;
        }

        local_ping(pr_argc, pr_argv2, &avg_delay2, &lost2, &mdev2);

        if ((avg_delay2 <= EXCELLENT_DELAY) && (lost2 == EXCELLENT_LOST) && (mdev2 <= EXCELLENT_MDEV)) {
            local_level2 = EXCELLENT;
        } else if ( (avg_delay2 <= GOOD_DELAY) && ( (lost2 <= GOOD_LOST) || (mdev2 <= GOOD_MDEV))) {
            local_level2 = GOOD;
        } else if ( (avg_delay2 <= COMMON_DELAY) && ( (lost2 <= COMMON_LOST) || (mdev2 <= COMMON_MDEV))) {
            local_level2 = COMMON;
        } else {
            local_level2 = NONE_DEFINE;
        }

        MSG_PRINTF(LOG_DBG, "ping %s operational is %lf----%lf----%d\n", domain1, avg_delay1, mdev1, lost1);
        MSG_PRINTF(LOG_DBG, "ping %s operational is %lf----%lf----%d\n", domain2, avg_delay2, mdev2, lost2);

        if (type == RT_OR) {
            if ((local_level1 >= level1) || (local_level2 >= level2)) {
                ret = RT_SUCCESS;
            } else {
                ret = RT_ERROR;
            }
        } else if (type == RT_AND) {
            if ((local_level1 >= level1) && (local_level2 >= level2)) {
                ret = RT_SUCCESS;
            } else {
                ret = RT_ERROR;
            }
        } else {
            ret = RT_ERROR;
        }
    }

    return ret;
}

int32_t ping_sim(int8_t *domain1, int32_t level1, int8_t *domain2, int32_t level2, int32_t type)
{
    int32_t pr_argc = 4;
    int32_t lost1, lost2;
    int32_t local_level1 = 0;
    int32_t local_level2 = 0;
    int32_t ret = RT_ERROR;
    // unuse redtea
    int8_t *pr_argv1[] = {"redtea", "-c", "10", domain1};
    int8_t *pr_argv2[] = {"redtea", "-c", "10", domain2};
    double avg_delay1,  mdev1, avg_delay2, mdev2;

    if (type == RT_NOT_DEFINE) {
        // only one
        local_ping(pr_argc, pr_argv1, &avg_delay1, &lost1, &mdev1);

        if ((avg_delay1 <= EXCELLENT_DELAY) && (lost1 == EXCELLENT_LOST) && (mdev1 <= EXCELLENT_MDEV)) {
            local_level1 = EXCELLENT;
        } else if ( (avg_delay1 <= GOOD_DELAY) && ( (lost1 <= GOOD_LOST) || (mdev1 <= GOOD_MDEV))) {
            local_level1 = GOOD;
        } else if ( (avg_delay1 <= COMMON_DELAY) && ( (lost1 <= COMMON_LOST) || (mdev1 <= COMMON_MDEV))) {
            local_level1 = COMMON;
        } else {
            local_level1 = NONE_DEFINE;
        }
        MSG_PRINTF(LOG_DBG, "ping %s sim is %lf----%lf----%d\n", domain1, avg_delay1, mdev1, lost1);
        if (local_level1 >= level1) {
            ret = RT_SUCCESS;
        } else {
            ret = RT_ERROR;
        }
    } else {
        local_ping(pr_argc, pr_argv1, &avg_delay1, &lost1, &mdev1);

        if ((avg_delay1 <= EXCELLENT_DELAY) && (lost1 == EXCELLENT_LOST) && (mdev1 <= EXCELLENT_MDEV)) {
            local_level1 = EXCELLENT;
        } else if ( (avg_delay1 <= GOOD_DELAY) && ( (lost1 <= GOOD_LOST) || (mdev1 <= GOOD_MDEV))) {
            local_level1 = GOOD;
        } else if ( (avg_delay1 <= COMMON_DELAY) && ( (lost1 <= COMMON_LOST) || (mdev1 <=COMMON_MDEV))) {
            local_level1 = COMMON;
        } else {
            local_level1 = NONE_DEFINE;
        }

        local_ping(pr_argc, pr_argv2, &avg_delay2, &lost2, &mdev2);

        if ((avg_delay2 <= EXCELLENT_DELAY) && (lost2 == EXCELLENT_LOST) && (mdev2 <= EXCELLENT_MDEV)) {
            local_level2 = EXCELLENT;
        } else if ( (avg_delay2 <= GOOD_DELAY) && ( (lost2 <= GOOD_LOST) || (mdev2 <= GOOD_MDEV))) {
            local_level2 = GOOD;
        } else if ( (avg_delay2 <= COMMON_DELAY) && ( (lost2 <= COMMON_LOST) || (mdev2 <= COMMON_MDEV))) {
            local_level2 = COMMON;
        } else {
            local_level2 = NONE_DEFINE;
        }
        MSG_PRINTF(LOG_DBG, "ping %s sim is %lf----%lf----%d\n", domain1, avg_delay1, mdev1, lost1);
        MSG_PRINTF(LOG_DBG, "ping %s sim is %lf----%lf----%d\n", domain2, avg_delay2, mdev2, lost2);
        if (type == 0) {
            if ((local_level1 >= level1) || (local_level2 >= level2)) {
                ret = RT_SUCCESS;
            } else {
                ret = RT_ERROR;
            }
        } else if (type == 1) {
            if ((local_level1 >= level1) && (local_level2 >= level2)) {
                ret = RT_SUCCESS;
            } else {
                ret = RT_ERROR;
            }
        } else {
            ret = RT_ERROR;
        }
    }

    return ret;
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


/*
    此函数功能为ping逻辑的实现
    send_buf FFFFFFFF
    第一位、第三位、第五位分别代表种子卡、业务卡、实体卡；1表示在用、F表示不在用
    第二位、第四位、第六位分别代表能否满足网络条件，1表示满足网络条件，0表示不满足条件
 */
static void network_ping_task(void *arg)
{
    int32_t i = 0;
    int32_t ii = 0;
    int32_t ret = 0;
    int32_t strategy_num = NULL;
    int32_t rt_level[RT_STRATEGY_NUM] = {-1, -1};
    int8_t rt_domain[RT_STRATEGY_NUM][RT_DOMAIN_LEN] = {0};
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
            // MSG_PRINTF(LOG_WARN, "network_detect : %s\n", cJSON_Print(network_detect));
            if (network_detect != NULL) {
                enabled = cJSON_GetObjectItem(network_detect, "enabled");
                interval = cJSON_GetObjectItem(network_detect, "interval");
            }
        }

        if (detect_flg == RT_TRUE && enabled->valueint == RT_TRUE) {
            g_to_start = RT_TRUE;
        } else {
            g_to_start = RT_FALSE;
        }

        MSG_PRINTF(LOG_DBG, "interval->valueint is %d\r\n", interval->valueint);

        rt_type = cJSON_GetObjectItem(network_detect, "type");
        MSG_PRINTF(LOG_DBG, "rt_type->valueint is %d\n",  rt_type->valueint);

        rt_os_sleep(interval->valueint * 60);       // first wait interval mins for dail up

        strategy_list = cJSON_GetObjectItem(network_detect, "strategies");
        if (strategy_list != NULL) {
            strategy_num = cJSON_GetArraySize(strategy_list);
            MSG_PRINTF(LOG_DBG, "strategy_num is %d\n", strategy_num);

            for (ii = 0; ii < strategy_num; ii++) {
                strategy_item = cJSON_GetArrayItem(strategy_list, ii);
                domain = cJSON_GetObjectItem(strategy_item, "domain");

                MSG_PRINTF(LOG_DBG, "domain->valuestring is %s\n",  domain->valuestring);
                strcpy(rt_domain[ii], domain->valuestring);

                if (!domain) {
                    MSG_PRINTF(LOG_WARN, "domain content failed!!\n");
                }

                MSG_PRINTF(LOG_DBG, "rt_domain is %s\n",  rt_domain[ii]);
                level = cJSON_GetObjectItem(strategy_item, "level");
                MSG_PRINTF(LOG_DBG, "level_value is %d\n",  level->valueint);
                rt_level[ii] = level->valueint;

                if (!level) {
                    MSG_PRINTF(LOG_WARN, "level content failed!!\n");
                }
                MSG_PRINTF(LOG_DBG, "level is %d\n",  rt_level[ii]);
            }
        } else {
            MSG_PRINTF(LOG_WARN, "strategies list is error\r\n");
        }

        if (network_detect != NULL) {
            cJSON_Delete(network_detect);
        }

        if (g_to_start == RT_TRUE) {
            MSG_PRINTF(LOG_DBG, "into tostart\r\n");

            if (*g_sim_type == PROFILE_TYPE_PROVISONING) {
                ret = ping_provisoning();

                MSG_PRINTF(LOG_DBG, "ping_provisoning status : %d\n", ret);

                if (ret == RT_SUCCESS) {
                    send_buf[0] = PROVISONING_HAVE_INTERNET;
                } else {
                    send_buf[0] = PROVISONING_NO_INTERNET;
                }

            } else if (*g_sim_type == PROFILE_TYPE_OPERATIONAL) {
                ret = ping_operational(rt_domain[0], rt_level[0], rt_domain[1], rt_level[1], rt_type->valueint);

                MSG_PRINTF(LOG_DBG, "ping_operational status : %d\n", ret);

                if (ret == RT_SUCCESS) {
                    send_buf[0] = OPERATIONAL_HAVE_INTERNET;
                } else {
                    send_buf[0] = OPERATIONAL_NO_INTERNET;
                }

            } else if (*g_sim_type == PROFILE_TYPE_SIM) {
                ret = ping_sim(rt_domain[0], rt_level[0], rt_domain[1], rt_level[1], rt_type->valueint);

                MSG_PRINTF(LOG_DBG, "ping_sim status : %d\n", ret);

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
    }

exit_entry:

    rt_exit_task(NULL);
}

int32_t init_ping_task(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;
    int32_t uicc_mode = ((public_value_list_t *)arg)->config_info->sim_mode;

    if ((uicc_mode == SIM_MODE_TYPE_VUICC_ONLY || uicc_mode == SIM_MODE_TYPE_SIM_ONLY)) {
        MSG_PRINTF(LOG_DBG, "uicc_mode is %d\n", uicc_mode);
        return RT_SUCCESS;
    } else if (uicc_mode == SIM_MODE_TYPE_SIM_FIRST) {
        MSG_PRINTF(LOG_DBG, "uicc_mode is %d\n", uicc_mode);
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


