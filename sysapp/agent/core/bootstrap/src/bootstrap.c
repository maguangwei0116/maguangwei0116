
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : bootstrap.c
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include <stdio.h>
#include "bootstrap.h"
#include "profile_parse.h"
#include "tlv.h"
#include "file.h"
#include "rt_manage_data.h"
#include "agent_queue.h"
#include "rt_type.h"

#define RANDOM_FILE                     "/dev/urandom"

static uint32_t g_single_interval_time = 10; // 激活失败单次间隔时间
static uint32_t g_max_retry_times = 7; // 最大重试次数
static uint32_t g_sleep_time = 24 * 60 * 60; // 休眠时长

static int32_t get_random(void)
{
    int32_t random = 0;
    if (rt_read_data(RANDOM_FILE, 0, (uint8_t * ) & random, sizeof(random)) == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "read urandom error\n");
    }
    return rand();
}

void bootstrap_select_profile(void)
{
    static uint8_t retry_times = 0;
    MSG_PRINTF(LOG_INFO, "bootstrap_select_profile:%d\n", g_single_interval_time);
    if (retry_times > g_max_retry_times) {
        retry_times = 0;
    } else {
        selected_profile(get_random());
        retry_times++;
        g_single_interval_time *= retry_times;
    }
}

int32_t init_bootstrap(void *arg)
{
    return init_profile_file(NULL);
}

void bootstrap_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    MSG_PRINTF(LOG_INFO, "The current mode is %d.\n", mode);
    if (mode == 0) {
        bootstrap_select_profile();
    } else if (mode == MSG_NETWORK_DISCONNECTED) {
        MSG_PRINTF(LOG_INFO, "g_single_interval_time:%d\n", g_single_interval_time);
        register_timer(g_single_interval_time, 0, &bootstrap_select_profile);
    } else if (mode == MSG_NETWORK_CONNECTED) {
    }
}
