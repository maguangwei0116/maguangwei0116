
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
#define SHARED_PROFILE                          0x30
#define HASH_CODE                               0x41
#define FILE_INFO                               0xA0
#define ROOT_SK                                 0x81
#define PROFILE_KEY                             0x82
#define OPT_PROFILES                            0xA3
#define PROFILE_VERSION                         0x81
#define PROFILE                                 0x81

int32_t init_profile_file(int32_t *arg);
int32_t selected_profile(int32_t random);

#endif // __PROFILE_PARSE_H__
