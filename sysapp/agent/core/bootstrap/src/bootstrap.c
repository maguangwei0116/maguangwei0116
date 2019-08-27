
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
#include "profile_file_parsing.h"
#include "tlv.h"
#include "file.h"
#include "rt_manage_data.h"
#include "rt_type.h"

#define RANDOM_FILE                     "/dev/urandom"

uint32_t g_default_single_interval_time = 60; // 激活失败单次间隔时间
uint32_t default_max_enable_num = 7; // 最大重试次数
uint32_t default_sleep_time = 24 * 60 * 60; // 休眠时长
uint32_t enable_num = 1; // 第几次激活

static uint16_t get_random(void)
{
    int32_t ret = 0;
    int random = 0;
    if (rt_read_data(RANDOM_FILE, 0, &random, sizeof(random)) == RT_ERROR) {
        printf("read urandom error \n");
        return RT_ERROR;
    }

    srand((int) random);
    ret = rand();
    return ret;
}

static int32_t bootstrap_select_profile(void)
{
    selected_profile(get_random());
    // todo 激活profile
    g_default_single_interval_time *= enable_num;
    enable_num++;

    return 0;
}

static uint32_t enable_profile_fail(void)
{
    if (enable_num > default_max_enable_num) {
        enable_num = 0;
        // todo 开始休眠
        rt_os_alarm(default_sleep_time);
    } else {
        bootstrap_select_profile();
    }

}

int32_t init_bootstrap(int32_t *arg) {
    rt_os_signal(RT_SIGINT, enable_profile_fail);
    return init_profile_file(NULL);
}

void bootstrap_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    MSG_PRINTF(LOG_INFO, "Help us choose the card\n");
    // todo 激活是否成功
    if (mode == 0) {
        bootstrap_select_profile();
    } else {
        rt_os_alarm(g_default_single_interval_time);

    }
}

