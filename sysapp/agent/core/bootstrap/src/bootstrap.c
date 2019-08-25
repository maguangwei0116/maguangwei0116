
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

uint32_t default_single_interval_time = 1; // 激活失败单次间隔时间
uint32_t default_max_enable_num = 7; // 最大重试次数
uint32_t default_sleep_time = 1; // 休眠时长
uint32_t enable_num = 0; // 第几次激活

uint32_t enable_profile_fail() {
    if (enable_num >= 7) {
        enable_num = 0;
        // todo 开始休眠
    } else {
        bootstrap_enable_profile(NULL);
    }

}

uint16_t get_random() {
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

int bootstrap_enable_profile(int32_t *arg) {

    selected_profile(get_random(), enable_num);
    // todo 激活profile
    enable_num++;

    return 0;
}

