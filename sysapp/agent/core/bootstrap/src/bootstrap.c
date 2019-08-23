
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

extern int32_t share_profile_num;  // share_profile数目
extern uint16_t share_profile_length;  // share_profile长度
extern int16_t share_profile_sequential;  // 是否连续为share_profile
extern uint16_t profile_offset;  // profile偏移量

uint16_t get_random() {
    int32_t ret = 0;
    int random = 0;
    if (rt_read_data(RANDOM_FILE, 0, &random, sizeof(random)) == RT_ERROR) {
        printf("read urandom error \n");
        return RT_ERROR;
    }

    srand((int) random);
    ret = rand();
    ret = ret % share_profile_num;

    return ret;
}

int get_profile_offset(rt_fshandle_t fp, uint8_t type, uint16_t *offset) {
    int ret = 0;
    uint8_t buf[4];
    rt_fread(buf, 1, 4, fp);
    rt_fread(buf, 1, 4, fp);
    *offset += 8;
    rt_fread(buf, 1, 4, fp);
    while (buf[0] != type) {
        *offset += 4;
        if (buf[1] == ASN1_LENGTH_2BYTES) {
            *offset += ((uint16_t) buf[2] << 8) + buf[3];
        } else if (buf[1] == ASN1_LENGTH_1BYTES) {
            *offset += buf[2] - 1;
        } else if ((buf[1] & 0x80) == 0) {
            *offset += buf[1] - 2;
        } else {
            ret = 105;
            break;
        }

        rt_fseek(fp, *offset, RT_FS_SEEK_SET);
        if (rt_fread(buf, 1, 4, fp) != 4) {
            return 106;
        }
    }
    if (type == FILE_INFO || type == ROOT_SK || type == PROFILE_KEY) {
        *offset += 2;
    } else {
        *offset += 4;
    }

    return ret;

}

int bootstrap_enable_profile() {
    int32_t select_block = 0;
    select_block = get_random();

    return 0;
}

int32_t selected_profile(rt_fshandle_t fp) {
    uint8_t buf[4];
    uint8_t profile[share_profile_length];
    int i;

    rt_fseek(fp, profile_offset, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    if (buf[1] == ASN1_LENGTH_2BYTES) {
        profile_offset += 4;
    }

    // 随机抽取profile
    profile_offset += get_random() * share_profile_length;
    printf("profile_offset:%d\n", profile_offset);
    rt_fseek(fp, profile_offset, RT_FS_SEEK_SET);
    rt_fread(profile, 1, share_profile_length, fp);
    // profile即为随机抽取的，待完善
    return 0;
}

