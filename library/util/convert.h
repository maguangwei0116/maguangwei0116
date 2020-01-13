
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : covert.h
 * Date        : 2018.08.30
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __CONVERT_H__
#define __CONVERT_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "rt_type.h"

typedef enum ERR_CODE {
    RT_ERR_CONVER_BAD_LEN = 0,
    RT_ERR_CONVER_BAD_CHAR
} err_code_e;

typedef struct byte_struct {
    uint8_t low:    4;
    uint8_t high:   4;
} byte_t;

void swap_nibble(uint8_t *buf, uint16_t swap_cnt);
void pad_F(char *raw_string, char *target, uint8_t size);
int hexstring2bytes(const char *hextring, uint8_t *bytes, uint16_t *length);
int bytes2hexstring(const uint8_t *bytes, uint16_t length, char *hextring);
rt_bool strncmp_case_insensitive(const char *src, const char *dst, uint16_t len);
void bytestring_to_charstring(const char *bytestring, char *charstring, uint16_t length);

#endif // __CONVERT_H__
