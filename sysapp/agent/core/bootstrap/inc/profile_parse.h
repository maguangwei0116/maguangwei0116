
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : profile_parse.h
 * Date        : 2019.08.28
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __PROFILE_PARSE_H__
#define __PROFILE_PARSE_H__

#include <stdio.h>
#include <stdbool.h>
#include "rt_type.h"

#define ASN1_LENGTH_1BYTES                      0x81
#define ASN1_LENGTH_2BYTES                      0x82
#define ASN1_LENGTH_3BYTES                      0x83
#define ASN1_LENGTH_4BYTES                      0x84

#define SHARED_PROFILE                          0x30
#define HASH_CODE                               0x41
#define FILE_INFO                               0xA0
#define ROOT_SK                                 0x81
#define PROFILE_KEY                             0x82
#define OPT_PROFILES                            0xA3
#define PROFILE_VERSION                         0x81
#define PROFILE                                 0x81

#define ARRAY_SIZE(a)                           (sizeof((a)) / sizeof((a)[0]))
#define SHARE_PROFILE                           "/data/redtea/profile_list.der"

typedef struct PROFILE_DATA {
    uint32_t file_info_offset;
    uint32_t root_sk_offset;
    uint32_t aes_key_offset;
    uint32_t operator_info_offset;
    uint32_t file_version_offset;
    int32_t priority;
    int32_t operator_num;
    uint32_t hash_code_offset;
} profile_data_t;

int32_t init_profile_file(int32_t *arg);
int32_t selected_profile(uint32_t random);

#endif // __PROFILE_PARSE_H__
